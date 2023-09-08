
#include<windows.h>
#include <comutil.h>
#include<tchar.h>
#include <setupapi.h>
#include <QtConcurrent>
#include <cfgmgr32.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include<windows.h>
#include<tchar.h>
#include <cfgmgr32.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <winreg.h>
#include <QDir>
#include "infomachine.h"
#include "qregularexpression.h"

#define SWPOLISTDIR                   "C:\\system.sav\\logs\\OSIT\\sw_ver"
#define DEVREGVERPATH            "SYSTEM\\CurrentControlSet\\Control\\Class"
#define SOFTWARE32                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
#define SOFTWARE64                    "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
#define VALUE_STRING                  "Not Find It Value"
#define  ENABLE_MULTITHREAD   1 //1 Enable / / 0 Disable

InFoMachine::InFoMachine(QObject *parent)
    : QObject{parent}
{
    CollectInFoDM();
}
const QString& InFoMachine::Reg2DisPlay(const QString& RegKeyStr,const QString& CompleteRegTree,HKEY RootKey)
{
    ReturnStr.clear();
    wchar_t* Buffer = new wchar_t[1024];
    DWORD BufferLen = 1024;
    HKEY hkRKey;
    DWORD dwType = REG_BINARY|REG_DWORD|REG_EXPAND_SZ|REG_MULTI_SZ|REG_NONE|REG_SZ;
    if (RegOpenKeyEx(RootKey, (CompleteRegTree).toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkRKey)==ERROR_SUCCESS) {
            RegQueryValueEx(hkRKey,RegKeyStr.toStdWString().c_str(), 0, &dwType, (LPBYTE)Buffer,&BufferLen);
            ReturnStr = QString::fromStdWString(Buffer);
    }
    delete[] Buffer;
    return ReturnStr;
}
bool  InFoMachine::CollectInFoDM(const QString &PNDriveName,const QString& PnDriverVer)
{
    bool check_status=false;
    static bool  hwscan = true;
    QString RegPath = DEVREGVERPATH;
    DWORD dwDevStatus, dwProblem,i;
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVINFO_DATA DevInfoData = { sizeof(SP_DEVINFO_DATA),{0,0,0,{0}},0,0};
    hDevInfo = SetupDiGetClassDevs(NULL,  0,  0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    wchar_t* sezClassBuff = new wchar_t[MAX_PATH];
    wchar_t* sezDescBuff = new wchar_t[MAX_PATH];
    wchar_t* FirendName = new wchar_t[MAX_PATH];
    wchar_t* HardWareID = new wchar_t[MAX_PATH];
    wchar_t*  Driver = new wchar_t[MAX_PATH];

    if (hDevInfo == INVALID_HANDLE_VALUE)
        qDebug("Device error INVALID");
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
        DWORD DataT;
        wchar_t* buffer = nullptr;
        DWORD buffersize = 0;
        QString ClassStr,DescStr,FirendNameStr,HardWareIDStr,DriverStr;
        while (!SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID,&DataT, (PBYTE)buffer, buffersize, &buffersize)) {
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                                if (buffer) LocalFree(buffer);
                                buffer = (LPTSTR)LocalAlloc(LPTR, buffersize);
                        } else   break;// Insert error handling here.
                }
        if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (PBYTE)sezClassBuff, MAX_PATH-1,NULL) )
            ClassStr = QString::fromStdWString(sezClassBuff);
        if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)sezDescBuff, MAX_PATH-1,NULL) )
            DescStr = QString::fromStdWString(sezDescBuff);

        if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)FirendName, MAX_PATH-1,NULL) )
            FirendNameStr = QString::fromStdWString(FirendName);

        if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)HardWareID, MAX_PATH-1,NULL) )
             HardWareIDStr = QString::fromStdWString(HardWareID);

        if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData,SPDRP_DRIVER, NULL, (PBYTE)Driver, MAX_PATH-1,NULL) ) {
            DriverStr = QString::fromStdWString(Driver);
            if (!PnDriverVer.isEmpty() && !PNDriveName.isEmpty()) {
                if (PNDriveName == FirendNameStr || PNDriveName == DescStr) {
                    if (PnDriverVer == Reg2DisPlay("DriverVersion",RegPath+"\\"+DriverStr,HKEY_LOCAL_MACHINE))
                    {
                        delete[] sezClassBuff;
                        delete[] sezDescBuff;
                        delete[] FirendName;
                        delete[] HardWareID;
                        delete[]  Driver;
                        if (buffer) LocalFree(buffer);
                        return true;
                    }

                }
            }
        }
        if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DevInfoData))
            qDebug("EnumDeviceInfo Error!");
        if (CM_Get_DevNode_Status(&dwDevStatus, &dwProblem, DevInfoData.DevInst, 0) != CR_SUCCESS)  {//CM_Get_Device_ID_List_Ex
            qDebug("<h3><font color=red>DevNode_Status Error!</font></h3>");
        }
        CHAR IDBuff[MAX_DEVNODE_ID_LEN] = {0};
        CM_Get_Device_ID_ExA(DeviceInfoData.DevInst,IDBuff,MAX_PATH-1,0,0);   // Second list_infotemp.append("ID:--------------"+QString::fromUtf8(IDBuff)+"\n"); //This is subID
        if (dwProblem!=0) {
            //Dev_err++;
            check_status=true;
        }
        if (HardWareIDStr.isEmpty()){
            ClassStr.clear();
            DescStr.clear();
            FirendNameStr.clear();
            HardWareIDStr.clear();
            DriverStr.clear();
            continue;
        }
        if (hwscan) {
            if ((FirendNameStr==DescStr) && !FirendNameStr.isEmpty())
                   HwidMap.emplace(ClassStr,FirendNameStr+"\t##\t"+HardWareIDStr);
                else if (!DescStr.isEmpty() &&  !FirendNameStr.isEmpty()) {
                        HwidMap.emplace(ClassStr,FirendNameStr+"\t##\t"+HardWareIDStr);
                }else  if (FirendNameStr.isEmpty() && !DescStr.isEmpty())
                                HwidMap.emplace(ClassStr,DescStr+"\t##\t"+HardWareIDStr);
                            else  HwidMap.emplace(ClassStr,"Unknow Device\t##\t"+HardWareIDStr);
          }
        ClassStr.clear();
        DescStr.clear();
        FirendNameStr.clear();
        HardWareIDStr.clear();
        DriverStr.clear();
       if (buffer) LocalFree(buffer);
    }
    hwscan = false;
    delete[] sezClassBuff;
   delete[] sezDescBuff;
   delete[] FirendName;
   delete[] HardWareID;
   delete[]  Driver;
   return check_status;
}
bool  InFoMachine::CollectInFoApp(const QString& SoftwareName,const QString& SoftwareVersion)
{
    QStringList S_softwareRegPath;
    QString Qregstring= SOFTWARE64;
    QString CompleteStr = SOFTWARE32;
    QString m_strSoftName;
    QString m_strSoftVersion;
    HKEY RootKey;
    HKEY hkResult;
    LONG lReturn;
    DWORD dwType = REG_BINARY|REG_DWORD|REG_EXPAND_SZ|REG_MULTI_SZ|REG_NONE|REG_SZ;
    RootKey = HKEY_LOCAL_MACHINE;
    S_softwareRegPath.append(CompleteStr);
    S_softwareRegPath.append(Qregstring);
    foreach (QString Regpath, S_softwareRegPath) {
        foreach (QString var_next, ReturnNextKeyname(Regpath,HKEY_LOCAL_MACHINE)) {
              if (SoftwareName ==  Reg2DisPlay("DisplayName",Regpath+"\\"+var_next,HKEY_LOCAL_MACHINE))
                                if (SoftwareVersion == Reg2DisPlay("DisplayVersion",Regpath+"\\"+var_next,HKEY_LOCAL_MACHINE))
                                            return true;
        }
        lReturn = RegOpenKeyEx(RootKey, Regpath.toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkResult);
        if (lReturn != ERROR_SUCCESS)
              return false;
        QString Appxreg = "Software\\Classes";
        QString OpenStr = "Application";
        HKEY hkRKey_app;
        HKEY AppRootKey = HKEY_USERS;
         wchar_t* AppnameBuffer = new wchar_t[1024];
        DWORD AppNameLen = 1024;
        foreach (QString var, ReturnNextKeyname(NULL,AppRootKey)) {
              foreach (QString Nextvar, ReturnNextKeyname(var+"\\"+Appxreg,AppRootKey)) {
                    if (Nextvar.contains("AppX")) {
                            if (RegOpenKeyEx(AppRootKey, (var+"\\"+Appxreg+"\\"+Nextvar+"\\"+OpenStr).toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkRKey_app)==ERROR_SUCCESS) {
                                    RegQueryValueEx(hkRKey_app, _T("ApplicationName"), 0, &dwType, (LPBYTE)AppnameBuffer,&AppNameLen);
                                    m_strSoftName =QString::fromStdWString(AppnameBuffer);
                                    QRegularExpressionMatch match_name,match_ver;
                                    AppNameLen = 1024;
                                    memset(AppnameBuffer, 0, sizeof(wchar_t));
                                    RegQueryValueEx(hkRKey_app, _T("ApplicationIcon"), 0, &dwType, (LPBYTE)AppnameBuffer,&AppNameLen);
                                    m_strSoftVersion = QString::fromStdWString(AppnameBuffer);
                                    AppNameLen = 1024;
                                    memset(AppnameBuffer, 0, sizeof(wchar_t));
                                    static QRegularExpression reg_ver("_\\d+.\\d+.\\d+.\\d_");
                                    static QRegularExpression reg_name(".*\\w_"),reg_name_2(".[a-zA-Z]+_");
                                    if (m_strSoftVersion.indexOf(reg_ver,0,&match_ver))
                                         m_strSoftVersion = match_ver.captured().replace("_","");
                                    if(!m_strSoftName.isEmpty()) {
                                        if(!m_strSoftName.indexOf(reg_name,0,&match_name))
                                            m_strSoftName = match_name.captured();

                                        if ( m_strSoftName.contains("@{Microsoft")) {
                                            if(m_strSoftName.indexOf(reg_name_2,0,&match_name)){
                                                m_strSoftName = match_name.captured().replace(".","");
                                                m_strSoftName = m_strSoftName.replace("_","");
                                            }
                                        }
                                        if(m_strSoftName == SoftwareName && m_strSoftVersion == SoftwareVersion  )
                                                    return true;
                                    }
                            }
                    }
              }
       }
     delete[] AppnameBuffer;
 //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    return false;
}
void  InFoMachine::CollectInFoLocalPn()
{
    QDir Pndir(SWPOLISTDIR);
    Pndir.setFilter( QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Files);
    if (Pndir.entryInfoList().isEmpty())
       return;
    std::function<std::variant<QString,QStringList> (const QString& NodeStr,const QString& Keystr,const QString& FilePath)> KeyFindValue= std::bind([=](const QString& NodeStr,const QString& Keystr,const QString& FilePath)->std::variant<QString,QStringList>{
        char* Keybuff =  new char[1024];
        char* Valuebuff =  new char[1024];
        QString Return_str;
        QStringList DeviceNameList;
        GetPrivateProfileStringA(NodeStr.toStdString().c_str(),NULL,VALUE_STRING,Keybuff,1024,FilePath.toStdString().c_str());
        for (char* keyname = Keybuff; *keyname!='\0'; keyname+=strlen(keyname)+1) {
                 if (Keystr.isEmpty()) {
                    GetPrivateProfileStringA(NodeStr.toStdString().c_str(),keyname,VALUE_STRING,Valuebuff,1024,FilePath.toStdString().c_str());
                    if ( VALUE_STRING != QString::fromStdString(Valuebuff) && QString::fromStdString(Valuebuff) !=nullptr){
                        DeviceNameList.append(QString::fromStdString(Valuebuff));
                    }
                 }
                if (Keystr==QString::fromStdString(keyname)) {
                         GetPrivateProfileStringA(NodeStr.toStdString().c_str(),keyname,VALUE_STRING,Valuebuff,1024,FilePath.toStdString().c_str());
                         Return_str = QString::fromStdString(Valuebuff);
                         delete[] Keybuff;
                         delete[]Valuebuff;
                         return Return_str;
                }
        }
        if (Keystr.isEmpty()) {
            delete[] Keybuff;
            delete[]Valuebuff;
            return DeviceNameList;
        }
        delete[] Keybuff;
        delete[]Valuebuff;
        return  Return_str;
    },std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);


    foreach (QFileInfo var, Pndir.entryInfoList()) {
        CompareLocalPn LocalPninfo = {nullptr,nullptr,nullptr,nullptr,{nullptr},false};

#if ENABLE_MULTITHREAD==1
                  QFuture<std::variant<QString,QStringList>> QFuGeneral_PN = QtConcurrent::run(KeyFindValue,"General","PN",var.canonicalFilePath());
                  LocalPninfo.PnName   =std::get<QString>(QFuGeneral_PN.result());

                  QFuture<std::variant<QString,QStringList>> QFuGeneral_Version = QtConcurrent::run(KeyFindValue,"General","Version",var.canonicalFilePath());
                  LocalPninfo.Version   =std::get<QString>(QFuGeneral_Version.result());

                  QFuture<std::variant<QString,QStringList>> QFuGeneral_Type = QtConcurrent::run(KeyFindValue,"General","Type",var.canonicalFilePath());
                  LocalPninfo.Type   =std::get<QString>(QFuGeneral_Type.result());

                  QFuture<std::variant<QString,QStringList>> QFuSoftwareTitle_US = QtConcurrent::run(KeyFindValue,"Software Title","US",var.canonicalFilePath());
                  LocalPninfo.ApplicationName   =std::get<QString>(QFuSoftwareTitle_US.result());

                  QFuture<std::variant<QString,QStringList>> QFuDevices = QtConcurrent::run(KeyFindValue,"Devices",nullptr,var.canonicalFilePath());
                  LocalPninfo.DevNameList   =std::get<QStringList>(QFuDevices.result());
#elif ENABLE_MULTITHREAD==0
                    LocalPninfo.PnName   =std::get<QString>( KeyFindValue("General","PN",var.canonicalFilePath()));
                      LocalPninfo.Version   =std::get<QString>( KeyFindValue("General","Version",var.canonicalFilePath()));
                           LocalPninfo.Type   =std::get<QString> (KeyFindValue("General","Type",var.canonicalFilePath()));
     LocalPninfo.ApplicationName   =std::get<QString> (KeyFindValue("Software Title","US",var.canonicalFilePath()));
            LocalPninfo.DevNameList   =std::get<QStringList> (KeyFindValue("Devices",nullptr,var.canonicalFilePath()));
#endif
        if (LocalPninfo.Type=="Driver"){
                if (!LocalPninfo.DevNameList.isEmpty()) {
                    foreach (QString drivername, LocalPninfo.DevNameList) {
                            if (drivername.isEmpty() || LocalPninfo.Version.isEmpty())
                                    continue;
                         LocalPninfo.LocalRight = CollectInFoDM(drivername,LocalPninfo.Version); //Call DM Scan
                        if ( LocalPninfo.LocalRight)
                               break;
                    }
                }else   LocalPninfo.LocalRight = false;
            }
        if (LocalPninfo.Type=="Application")
            {
                if (!LocalPninfo.ApplicationName.isEmpty() && !LocalPninfo.Version.isEmpty())
                       LocalPninfo.LocalRight = CollectInFoApp(LocalPninfo.ApplicationName,LocalPninfo.Version);//Call DM Scan
            }
        PnComMap.emplace(LocalPninfo.PnName,LocalPninfo);
    }
    emit CollectInFoLocalPnFinished(PnComMap);
}
const std::list<QString>& InFoMachine::ReturnNextKeyname(const QString& CurrentKeyName,HKEY RootKey)
{
    HKEY hkResult;
    DWORD KeyLen = 1024;
    wchar_t* szKeyName = new wchar_t[KeyLen];
    if (!KeyListName.empty())
            KeyListName.clear();
    if (ERROR_SUCCESS == RegOpenKeyEx(RootKey, CurrentKeyName.toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkResult)){
            for (DWORD index = 0; ERROR_NO_MORE_ITEMS !=RegEnumKeyEx(hkResult, index, szKeyName, &KeyLen, 0, NULL, NULL, NULL);  index++ ) {
                    KeyListName.push_back(QString::fromStdWString(szKeyName));
                    KeyLen = 1024;
                    memset(szKeyName, 0, sizeof(wchar_t));
            }
    }
    delete[] szKeyName;
    return KeyListName;
}

std::variant<bool,QString>  InFoMachine::Get_SetSerialNumber(const QString& SetSN)
{
    if(SetSN.isEmpty())
        return SerialNumber;
    else{
        SerialNumber = SetSN;
           return true;
    }
   return false;
};
const std::list<QString>& InFoMachine::WmicQuery(const QString& QueryString,const QStringList& KeyStr)
{
   WmicReulstString.clear();
   HRESULT hres;
   hres = CoInitializeEx(0,COINIT_APARTMENTTHREADED);
   if (FAILED(hres))
       qDebug() << "Failed to initialize COM library. " << "Error code 0x"<<Qt::hex<<hres;
   hres = CoInitializeSecurity(
       NULL,
       -1,
       NULL,
       NULL,
       RPC_C_AUTHN_LEVEL_DEFAULT,
       RPC_C_IMP_LEVEL_IMPERSONATE,
       NULL,
       EOAC_NONE,
       NULL
       );
   if ((hres != RPC_E_TOO_LATE) && FAILED(hres)){
           qDebug()<< "Failed to initialize security. "<< "Error code = 0x"<< hres;
           CoUninitialize();
   }
   IWbemLocator* pLoc = 0;
   hres = CoCreateInstance(
       CLSID_WbemLocator,
       0,
       CLSCTX_INPROC_SERVER,
       IID_IWbemLocator, (LPVOID*)&pLoc);
   if (FAILED(hres))
   {
           qDebug()<< "Failed to create IWbemLocator object. " << "Error code = 0x" <<  hres;
           CoUninitialize();
   }
   IWbemServices* pSvc = 0;
   hres = pLoc->ConnectServer(
       _bstr_t(L"ROOT\\CIMV2"),
       NULL,
       NULL,
       0,
       NULL,
       0,
       0,
       &pSvc
       );
   if (FAILED(hres))
   {
           qDebug() << "Could not connect. Error code = 0x"<< hres;
           pLoc->Release();
           CoUninitialize();
   }
   hres = CoSetProxyBlanket(
       pSvc,
       RPC_C_AUTHN_WINNT,
       RPC_C_AUTHZ_NONE,
       NULL,
       RPC_C_AUTHN_LEVEL_CALL,
       RPC_C_IMP_LEVEL_IMPERSONATE,
       NULL,
       EOAC_NONE
       );
   if (FAILED(hres)){
           qDebug()<< "Could not set proxy blanket. Error code = 0x"<< hres;
           pSvc->Release();
           pLoc->Release();
           CoUninitialize();
   }
   IEnumWbemClassObject* pEnumerator = NULL;
   QString WMI_Class_Name = QueryString;
   QString QS2bstr_wql = "WQL";
   QString QS2bstr_sql = "SELECT * FROM  "+WMI_Class_Name;
   wchar_t* bstr_wql = SysAllocString(QS2bstr_wql.toStdWString().c_str());
   wchar_t* bstr_sql  = SysAllocString(QS2bstr_sql.toStdWString().c_str() );

   hres = pSvc->ExecQuery(
       bstr_wql,
       bstr_sql,
       WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
       NULL,
       &pEnumerator);

   if (FAILED(hres)){
           qDebug()<< "Query for processes failed. "<< "Error code = 0x"<<Qt::hex<< hres ;
           pSvc->Release();
           pLoc->Release();
           CoUninitialize();
   }else{
           IWbemClassObject* pclsObj;
           ULONG uReturn = 0;
           while (pEnumerator)
           {
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                if (0 == uReturn)break;
                foreach (QString var, KeyStr) {
                    VARIANT vtProp;
                    pclsObj->Get(var.toStdWString().c_str(), 0, &vtProp, 0, 0);
                    if (1==vtProp.vt)continue;
                    if (3==vtProp.vt) {
                        if (!(QString::fromStdString)(std::to_string(vtProp.lVal)).isEmpty()) {
                               WmicReulstString.push_back(var+":\t"+QString::fromStdString(std::to_string(vtProp.lVal) ) );
                        }
                    }else{
                        if(11==vtProp.vt)
                               vtProp.boolVal==VARIANT_TRUE ? WmicReulstString.push_back(var+":\t"+"true") :WmicReulstString.push_back(var+":\t"+"false");
                        if(8==vtProp.vt)
                        if (!(QString::fromStdWString(vtProp.bstrVal)).isEmpty()) {
                                        WmicReulstString.push_back(var+":\t"+QString::fromStdWString(vtProp.bstrVal));
                        }
                    }
                    VariantClear(&vtProp);
                }
                pclsObj->Release();
                pclsObj = NULL;
           }

   }
   SysFreeString(bstr_wql);
   SysFreeString(bstr_sql);
   pSvc->Release();
   pLoc->Release();
   pEnumerator->Release();
   CoUninitialize();
   return WmicReulstString;
}
QString InFoMachine::GetMacAddress() const
{
   PIP_ADAPTER_INFO adapterInfo;
   DWORD bufferSize = sizeof(IP_ADAPTER_INFO);
   adapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);

   if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW)
   {
           free(adapterInfo);
           adapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);
   }

   if (GetAdaptersInfo(adapterInfo, &bufferSize) == NO_ERROR)
   {
           PIP_ADAPTER_INFO adapter = adapterInfo;
           while (adapter)
           {
                std::stringstream macAddress;
                macAddress << std::hex << std::setfill('0');
                for (unsigned int i = 0; i < adapter->AddressLength; i++)
                {
                    macAddress << std::setw(2) << (int)adapter->Address[i];
                    if (i != adapter->AddressLength - 1)
                    {
                        macAddress << "-";
                    }
                }
                 free(adapterInfo);
                return  QString::fromStdString(macAddress.str());
                adapter = adapter->Next;
           }
   }
   free(adapterInfo);
   return "";
}










