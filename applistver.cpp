#include "applistver.h"
#include "qregularexpression.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <winreg.h>
#include <iostream>
#include <QtConcurrent>
#include <QFuture>


AppListVer::AppListVer(QObject *parent)
    : QObject{parent},Notebuff(new char[1024] ),Keybuff(new char[1024]), Valuebuff(new char[1024]),
    PN_NoteMmp( new  std::unordered_multimap<QString,QString>),applistmmp( new std::unordered_multimap<QString,QString>),
    PN_FileListInFo (new std::list<std::unordered_multimap<QString,QString>>),KeyListName(new std::list<QString>),
    WmicReulstString(new std::list<QString>),DeviceInfovector(new std::vector<DeviceInfo>)
{
   //DeviceCheck();
  //List_App();
  //Function_Sample();
}
AppListVer::~AppListVer()
{
  delete[]  Notebuff;
  delete[]  Keybuff;
  delete[]  Valuebuff;
  delete PN_NoteMmp;
  delete applistmmp;
  delete PN_FileListInFo;
  delete KeyListName;
  delete WmicReulstString;
  delete DeviceInfovector;
}
std::variant<bool,QStringList>AppListVer::DeviceCheck(){
        bool check_status=true;
        int ErrDevCount=0;
        QStringList info_result_list;
        DWORD dwDevStatus, dwProblem,i;
        char* IDBuff = new char[MAX_DEVNODE_ID_LEN];
        wchar_t* sezClassBuff = new wchar_t[MAX_PATH];
        wchar_t* sezDescBuff = new wchar_t[MAX_PATH];
        wchar_t* sezIocinfoBuff = new wchar_t[MAX_PATH];
        wchar_t* FirendName = new wchar_t[MAX_PATH];
        wchar_t* HardWareID = new wchar_t[MAX_PATH];
        wchar_t*  Driver = new wchar_t[MAX_PATH];
        HDEVINFO hDevInfo;
        SP_DEVINFO_DATA DeviceInfoData;
        SP_DEVINFO_DATA DevInfoData = { sizeof(SP_DEVINFO_DATA),{0,0,0,{0}},0,0};
        hDevInfo = SetupDiGetClassDevs(NULL,  0,  0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
        if (hDevInfo == INVALID_HANDLE_VALUE)
          info_result_list.append("Device error INVALID");
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
            DWORD DataT;
            LPTSTR buffer = NULL;
            DWORD buffersize = 0;
            struct DeviceInfo Devinfo;
            while (!SetupDiGetDeviceRegistryProperty( hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID,&DataT, (PBYTE)buffer, buffersize, &buffersize)) {
                        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {      // Change the buffer size.
                                if (buffer) LocalFree(buffer);
                                buffer = (LPTSTR)LocalAlloc(LPTR, buffersize);
                        } else {
                                     break;// Insert error handling here.
                           }
                 }

            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (PBYTE)sezClassBuff, MAX_PATH-1,NULL) ) {
                        Devinfo.sezClassBuff = QString::fromStdWString(sezClassBuff);
              }
            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)sezDescBuff, MAX_PATH-1,NULL) ) {
                       Devinfo.sezDescBuf = QString::fromStdWString(sezDescBuff);
              }
            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)sezIocinfoBuff, MAX_PATH-1,NULL) ) {
                       //Devinfo.
              }
            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)FirendName, MAX_PATH-1,NULL) ) {
                       Devinfo.FirendName = QString::fromStdWString(FirendName);
              }
            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)HardWareID, MAX_PATH-1,NULL) ) {
                       Devinfo.HardWareID = QString::fromStdWString(HardWareID);
              }
            if( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData,SPDRP_DRIVER, NULL, (PBYTE)Driver, MAX_PATH-1,NULL) ) {
                      Devinfo.Driver = QString::fromStdWString(Driver);
              }
            if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DevInfoData))
                 info_result_list.append("EnumDeviceInfo Error!");
            if (CM_Get_DevNode_Status(&dwDevStatus, &dwProblem, DevInfoData.DevInst, 0) != CR_SUCCESS)  {//CM_Get_Device_ID_List_Ex
                info_result_list.append("<h3><font color=red>DevNode_Status Error!</font></h3>");
            }
            CM_Get_Device_ID_ExA(DeviceInfoData.DevInst,IDBuff,MAX_PATH-1,0,0);   // Second list_infotemp.append("ID:--------------"+QString::fromUtf8(IDBuff)+"\n"); //This is subID
            if (dwProblem!=0) {
                ErrDevCount++;
                info_result_list.append("Error ID:\t"+QString::fromUtf8(IDBuff)+"\n");
                check_status=false;
             }
            DeviceInfovector->push_back(Devinfo);
            if (buffer) LocalFree(buffer);
       }
        if (!check_status) {
            info_result_list.append("<h3><font color=red>The System Device Management have YB and unknown device!</font></h3>");
            info_result_list.append( "<h3><font color=red>Error Device Count:\t"+QString::number(ErrDevCount)+"</font></h3>");
            if (system("START devmgmt.msc")) {
               info_result_list.append("<h3><font color=red>Open Decice Manager Error!</font></h3>");
            }else {
                      info_result_list.append("<h3><font color=red>Open Decice Manager sucessfully!</font></h3>");
                   }
        } else {
                    info_result_list.append("<h1><font color=green>Device Management Check PASS!</font></h1>");
                 }
       delete[] sezClassBuff;
       delete[] sezDescBuff;
       delete[] sezIocinfoBuff;
       delete[] FirendName;
       delete[] HardWareID;
       delete[] Driver;
       delete[] IDBuff;
       if (check_status)
              return true;
       else
           return info_result_list;
}
/*
RootKey = HKEY_CURRENT_USER;//"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall
SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall
*/
void AppListVer::List_Appx(const QString& Qregstring)
{
        struct SoftInfo softinfo;
        HKEY RootKey;                                                                           // 主键
        HKEY hkResult;                                                                         // 将要打开键的句柄
        LONG lReturn;                                                                        // 记录读取注册表是否成功
        DWORD dwType = REG_BINARY|REG_DWORD|REG_EXPAND_SZ|REG_MULTI_SZ|REG_NONE|REG_SZ;
        RootKey = HKEY_LOCAL_MACHINE;
        lReturn = RegOpenKeyEx(RootKey, Qregstring.toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkResult);
        if (lReturn != ERROR_SUCCESS)
                   ::MessageBox(NULL, _T("Please Run With Administrator"), NULL, MB_ICONWARNING);
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
                                softinfo.m_strSoftName =QString::fromStdWString(AppnameBuffer);
                                 QRegularExpressionMatch match_name,match_ver;
                                AppNameLen = 1024;
                                memset(AppnameBuffer, 0, sizeof(wchar_t));
                                RegQueryValueEx(hkRKey_app, _T("ApplicationIcon"), 0, &dwType, (LPBYTE)AppnameBuffer,&AppNameLen);
                                softinfo.m_strSoftVersion = QString::fromStdWString(AppnameBuffer);
                                AppNameLen = 1024;
                                memset(AppnameBuffer, 0, sizeof(wchar_t));
                                static QRegularExpression reg_ver("_\\d+.\\d+.\\d+.\\d_");
                                static QRegularExpression reg_name(".*\\w_"),reg_name_2(".[a-zA-Z]+_");
                                if (softinfo.m_strSoftVersion.indexOf(reg_ver,0,&match_ver))
                                            softinfo.m_strSoftVersion = match_ver.captured().replace("_","");
                                if(!softinfo.m_strSoftName.isEmpty()) {
                                            if(!softinfo.m_strSoftName.indexOf(reg_name,0,&match_name))
                                                 softinfo.m_strSoftName = match_name.captured();

                                                if ( softinfo.m_strSoftName.contains("@{Microsoft")) {
                                                     if(softinfo.m_strSoftName.indexOf(reg_name_2,0,&match_name)){
                                                         softinfo.m_strSoftName = match_name.captured().replace(".","");
                                                         softinfo.m_strSoftName = softinfo.m_strSoftName.replace("_","");
                                                     }
                                                }
                                         AppList.push_back(softinfo.m_strSoftName+"\t\t\t\t"+softinfo.m_strSoftVersion);
                                }
                         }
                    }
               }
        }
     delete[] AppnameBuffer;
}
const std::unordered_multimap<QString,QString>& AppListVer::Reg2DisPlay(const QStringList& RegKeyStr, const QString& CompleteRegTree, HKEY RootKey, QString split)
{
       wchar_t* Buffer = new wchar_t[1024];
       DWORD BufferLen = 1024;
       HKEY hkRKey;
       QString tmp;
       applistmmp->clear();
       std::map<QString,QString> app2something;
       DWORD dwType = REG_BINARY|REG_DWORD|REG_EXPAND_SZ|REG_MULTI_SZ|REG_NONE|REG_SZ;
        if (RegOpenKeyEx(RootKey, (CompleteRegTree).toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkRKey)==ERROR_SUCCESS) {
            for (int var = 0; var < RegKeyStr.count(); ++var) {
                  RegQueryValueEx(hkRKey,RegKeyStr[var].toStdWString().c_str(), 0, &dwType, (LPBYTE)Buffer,&BufferLen);
                  app2something[RegKeyStr[var]] = QString::fromStdWString(Buffer).simplified();
                  if (split.isEmpty())
                        tmp+=QString::fromStdWString(Buffer)+"\t\t";
                  else  tmp.append(QString::fromStdWString(Buffer)+split);
                  BufferLen = 1024;
                  memset(Buffer, 0, sizeof(wchar_t));
            }
        if (!app2something[RegKeyStr.first()].isEmpty())
                  applistmmp->emplace(app2something[RegKeyStr.first()],tmp);
        }
       RegCloseKey(hkRKey);
        delete[] Buffer;
       return *applistmmp;
}
const std::list<QString>& AppListVer::ReturnNextKeyname(const QString& CurrentKeyName,HKEY RootKey)
{
    HKEY hkResult;
    DWORD KeyLen = 1024;
    wchar_t* szKeyName = new wchar_t[1024];
    if (!KeyListName->empty())
        KeyListName->clear();
    if (ERROR_SUCCESS == RegOpenKeyEx(RootKey, CurrentKeyName.toStdWString().c_str(), 0, KEY_ALL_ACCESS, &hkResult)){
        for (DWORD index = 0; ERROR_NO_MORE_ITEMS !=RegEnumKeyEx(hkResult, index, szKeyName, &KeyLen, 0, NULL, NULL, NULL);  index++ ) {
                  KeyListName->push_back(QString::fromStdWString(szKeyName));
                        KeyLen = 1024;
                        memset(szKeyName, 0, sizeof(wchar_t));
                 }
    }
    delete[] szKeyName;
    return *KeyListName;
}
const std::list<QString>& AppListVer::WmicListStr(const QString& QueryString,const QStringList& KeyStr)
{
    HRESULT hres;
    // Initialize COM.
    // COINIT_MULTITHREADED   //Only One thread    COINIT_APARTMENTTHREADED //More thread
    hres = CoInitializeEx(0,COINIT_APARTMENTTHREADED);
    if (FAILED(hres))
                 qDebug() << "Failed to initialize COM library. " << "Error code 0x"<<Qt::hex<<hres;
    // Initialize
    hres = CoInitializeSecurity(
        NULL,
        -1,                               // COM negotiates service
        NULL,                             // Authentication services
        NULL,                             // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,        // authentication
        RPC_C_IMP_LEVEL_IMPERSONATE,      // Impersonation
        NULL,                             // Authentication info
        EOAC_NONE,                       // Additional capabilities
        NULL                             // Reserved
        );
    if ((hres != RPC_E_TOO_LATE) && FAILED(hres)){                                                                 //RPC_E_TOO_LATE this value is OK
                 qDebug()<< "Failed to initialize security. "<< "Error code = 0x"<< hres;
                 CoUninitialize();
    }
    // Obtain the initial locator to Windows Management
    // on a particular host computer.
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
    // Connect to the root\cimv2 namespace with the
    // current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // WMI namespace
        NULL,                    // User name
        NULL,                    // User password
        0,                       // Locale
        NULL,                    // Security flags
        0,                       // Authority
        0,                       // Context object
        &pSvc                    // IWbemServices proxy
        );
    if (FAILED(hres))
    {
                 qDebug() << "Could not connect. Error code = 0x"<< hres;
                 pLoc->Release();
                 CoUninitialize();
    }

//    qDebug() << "Connected to ROOT\\CIMV2 WMI namespace" ;

    // Set the IWbemServices proxy so that impersonation
    // of the user (client) occurs.
    hres = CoSetProxyBlanket(
        pSvc,                         // the proxy to set
        RPC_C_AUTHN_WINNT,            // authentication service
        RPC_C_AUTHZ_NONE,             // authorization service
        NULL,                         // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,       // authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE,  // impersonation level
        NULL,                         // client identity
        EOAC_NONE                     // proxy capabilities
        );
    if (FAILED(hres)){
                 qDebug()<< "Could not set proxy blanket. Error code = 0x"<< hres;
                 pSvc->Release();
                 pLoc->Release();
                 CoUninitialize();
    }
    // Use the IWbemServices pointer to make requests of WMI.
    // Make requests here:
    // For example, query for all the running processes
    IEnumWbemClassObject* pEnumerator = NULL;

  //`_com_util::ConvertStringToBSTR(char const*)'           Error code
    QString WMI_Class_Name = QueryString;
    QString QS2bstr_wql = "WQL";
    QString QS2bstr_sql = "SELECT * FROM  "+WMI_Class_Name;
  //`_com_util::ConvertStringToBSTR(char const*)'           Error code

    BSTR bstr_wql = SysAllocString(QS2bstr_wql.toStdWString().c_str());
    BSTR bstr_sql  = SysAllocString(QS2bstr_sql.toStdWString().c_str() );

    hres = pSvc->ExecQuery(
        bstr_wql,
        bstr_sql,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);
    //qDebug()<<QS2bstr_sql;
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
                        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                        if (0 == uReturn)
                                  break;
                        foreach (QString var, KeyStr) {
                              VARIANT vtProp;
                              hres = pclsObj->Get(var.toStdWString().c_str(), 0, &vtProp, 0, 0);                                      // Get the value of the Name property
                              if (1==vtProp.vt)
                                continue;
                              if (3==vtProp.vt) {
                                if (!(QString::fromStdString)(std::to_string(vtProp.lVal)).isEmpty()) {
                                         WmicReulstString->push_back(var+":\t"+QString::fromStdString(std::to_string(vtProp.lVal) ) );
                                }
                              }else{
 // VT_EMPTY = 0, VT_NULL = 1, VT_I2 = 2, VT_I4 = 3,VT_R4 = 4,VT_R8 = 5,VT_CY = 6,VT_DATE = 7,VT_BSTR = 8,VT_DISPATCH = 9,VT_ERROR = 10,VT_BOOL = 11,VT_VARIANT = 12,VT_UNKNOWN = 13,VT_DECIMAL = 14,VT_I1 = 16,VT_UI1 = 17,VT_UI2 = 18,VT_UI4 = 19,
 //VT_I8 = 20,VT_UI8 = 21,VT_INT = 22,VT_UINT = 23,VT_VOID = 24,VT_HRESULT = 25,VT_PTR = 26,VT_SAFEARRAY = 27,VT_CARRAY = 28,VT_USERDEFINED = 29,VT_LPSTR = 30,VT_LPWSTR = 31,VT_RECORD = 36,VT_INT_PTR = 37,VT_UINT_PTR = 38,VT_FILETIME = 64,
//VT_BLOB = 65,VT_STREAM = 66,VT_STORAGE = 67,VT_STREAMED_OBJECT = 68,VT_STORED_OBJECT = 69,VT_BLOB_OBJECT = 70,VT_CF = 71,VT_CLSID = 72,VT_VERSIONED_STREAM = 73,VT_BSTR_BLOB = 0xfff,VT_VECTOR = 0x1000,VT_ARRAY = 0x2000,VT_BYREF = 0x4000,
//VT_RESERVED = 0x8000,VT_ILLEGAL = 0xffff,VT_ILLEGALMASKED = 0xfff,VT_TYPEMASK = 0xfff
                                         if(11==vtProp.vt)
                                         vtProp.boolVal==VARIANT_TRUE ? WmicReulstString->push_back(var+":\t"+"true") :WmicReulstString->push_back(var+":\t"+"false");
                                        if(8==vtProp.vt)
                                        if (!(QString::fromStdWString(vtProp.bstrVal)).isEmpty()) {
                                                     WmicReulstString->push_back(var+":\t"+QString::fromStdWString(vtProp.bstrVal));
                                        }
                              }

                              VariantClear(&vtProp);
                        }
                        pclsObj->Release();
                        pclsObj = NULL;
                 }

    }
    // Cleanup
    // ========
    SysFreeString(bstr_wql);
    SysFreeString(bstr_sql);
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return *WmicReulstString;
}
void AppListVer::Function_Sample()
{
    SP_DRVINFO_DATA_V2_A Data_A ;
    Data_A.cbSize =   sizeof(SP_DEVINFO_DATA);
    if(SetupDiEnumDriverInfoA(SetupDiGetClassDevs(NULL,  0,  0, DIGCF_PRESENT | DIGCF_ALLCLASSES),NULL,SPDIT_CLASSDRIVER|SPDIT_COMPATDRIVER,0,&Data_A))
                 qDebug()<<"DriverVersion:@@@@@@@@@@@"<<Data_A.DriverVersion;
}
/*******************************************************************************
 * void AppListVer::Pn_NumFindDriver(QString PnFile)
   * Get Note Enum
   * Note return
   * GetPrivateProfileSectionNamesA(buff_2,1024,PnFile.toStdString().c_str())
   * GetPrivateProfileStringA(NULL,NULL,"Not Find It Key",buff_2,1024,PnFile.toStdString().c_str())
   * ****************************************************************************
   * Key Enum
   * Key word return
   * GetPrivateProfileSectionNamesA(buff_2,1024,PnFile.toStdString().c_str());
   *  GetPrivateProfileStringA(NULL,NULL,"Not Find It Key",buff_2,1024,PnFile.toStdString().c_str());
   * *****************************************************************************
   *
   * Return Key=Value
   *
*********************************************************************************/
const std::unordered_multimap<QString, QString>* AppListVer::Pn_NumFindDriver(const QString& PnIniFile)
{
    GetPrivateProfileStringA(NULL,NULL,"Not Find It Note",Notebuff,1024,PnIniFile.toStdString().c_str());
    for (char* Notename = Notebuff; *Notename!='\0'; Notename+=strlen(Notename)+1) {
                        GetPrivateProfileStringA(Notename,NULL,"Not Find It Key",Keybuff,1024,PnIniFile.toStdString().c_str());
                        for (char* keyname = Keybuff; *keyname!='\0'; keyname+=strlen(keyname)+1) {
                                  //QFuture<DWORD> funGet = QtConcurrent::run(GetPrivateProfileStringA,Notename,keyname,"Not Find It Value",Valuebuff,1024,PnIniFile.toStdString().c_str());
                                 // funGet.waitForFinished();
                              GetPrivateProfileStringA(Notename,keyname,"Not Find It Value",Valuebuff,1024,PnIniFile.toStdString().c_str());
                              PN_NoteMmp->emplace(QString::fromStdString(Notename),QString::fromStdString(keyname)+"="+Valuebuff);
                              memset(Valuebuff, 0, sizeof(1024));
                        }
                 memset(Keybuff, 0, sizeof(1024));
    }
    memset(Notebuff, 0, sizeof(256));
    emit FindDriverComplete(PN_NoteMmp);
    return PN_NoteMmp;
}
 const std::list<std::unordered_multimap<QString,QString>>* AppListVer::Pn_NumFindDriver(const QStringList& PnIniFileList)
{
    foreach (QString var, PnIniFileList) {
           PN_FileListInFo->push_back(*Pn_NumFindDriver(var));
    }
    emit FindDriverComplete(PN_FileListInFo);
    return PN_FileListInFo;
}
void AppListVer::CleanData()
{
    if(KeyListName !=nullptr)
        KeyListName->clear();
    if(WmicReulstString !=nullptr)
        WmicReulstString->clear();
    if(applistmmp != nullptr)
        applistmmp->clear();
    if(PN_NoteMmp != nullptr)
        PN_NoteMmp->clear();
 }










