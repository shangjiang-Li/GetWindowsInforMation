#ifndef APPLISTVER_H
#define APPLISTVER_H
#include <QObject>
#include<windows.h>
#include<tchar.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <QDebug>
struct SoftInfo
{
    QString m_strSoftName;
    QString m_strSoftVersion;
    QString m_strInstallLocation;
    QString m_strPublisher;
    QString m_strMainProPath;
    QString m_strUninstallPth;
};
struct DeviceInfo
{
    QString sezClassBuff;
    QString sezDescBuf;
    QString sezIocinfoBuff;
    QString FirendName;
    QString HardWareID;
    QString Driver;
};
class AppListVer: public QObject
{
    Q_OBJECT
public:
    AppListVer(QObject *parent = nullptr);
    ~AppListVer();
    std::variant<bool,QStringList> DeviceCheck();
    void List_Appx(const QString& Qregstring);
    const std::unordered_multimap<QString, QString> *Pn_NumFindDriver(const QString& PnIniFile);
    const std::list<std::unordered_multimap<QString,QString>>* Pn_NumFindDriver(const QStringList& PnIniFileList);
    const std::list<QString>& ReturnNextKeyname(const QString& CurrentKeyName,HKEY RootKey);
    const std::list<QString>& WmicListStr(const QString& QueryString,const QStringList& KeyStr);
    const std::unordered_multimap<QString,QString>& Reg2DisPlay(const QStringList& RegKeyStr,const QString& CompleteRegTree,HKEY RootKey,QString split=nullptr);
    const std::vector<DeviceInfo>& GetDeviceInfo2DM(){ return *DeviceInfovector;};
    const std::unordered_multimap<QString,QString>& Get_PnorIniInfo(){return *PN_NoteMmp;}
    void CleanData();
    void Function_Sample();
    std::list<QString> DeviceDMinfo;
    std::list<QString>AppList;
signals:
    void device_complete(const QStringList& informations);
    void FindDriverComplete( const std::unordered_multimap<QString,QString>*  PN_NoteMmp);
    void FindDriverComplete( QMultiMap<QString,QString>&  qt_PN_NoteMmp);

    void FindDriverComplete( const std::list<std::unordered_multimap<QString,QString>>* PnFileInfoList);
private:
    char*  Notebuff;
    char*  Keybuff;
    char*  Valuebuff;
    std::unordered_multimap<QString,QString>* PN_NoteMmp;
    std::unordered_multimap<QString,QString>* applistmmp;
    std::list<std::unordered_multimap<QString,QString>>* PN_FileListInFo;
    std::list<QString>* KeyListName;
    std::list<QString>* WmicReulstString;
    std::vector<DeviceInfo>*  DeviceInfovector;
};

#endif // APPLISTVER_H
