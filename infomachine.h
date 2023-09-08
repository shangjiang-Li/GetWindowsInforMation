
#ifndef INFOMACHINE_H
#define INFOMACHINE_H
#include <QMultiMap>
#include <QObject>
#include <minwindef.h>

struct CompareLocalPn
{
    QString PnName;
    QString Version;
    QString Type;
    QString ApplicationName;
    QStringList DevNameList;
    bool LocalRight;
};
class InFoMachine : public QObject
{
    Q_OBJECT
public:
    explicit InFoMachine(QObject *parent = nullptr);
    void CollectInFoLocalPn();
    bool CollectInFoDM(const QString& Drivename=nullptr,const QString& PnDriverVer=nullptr);//Firendname and Devicedes name + HWID  Drvier version
    bool CollectInFoApp(const QString& SoftwareName,const QString& SoftwareVersion);
    std::variant<bool,QString> Get_SetSerialNumber(const QString& SetSN=NULL);

    const std::list<QString>& WmicQuery(const QString& QueryString,const QStringList& KeyStr);
    const QString& Reg2DisPlay(const QString &RegKeyStr, const QString& CompleteRegTree, HKEY RootKey);
    std::unordered_multimap<QString,QString>& GetHwMap(){return HwidMap;}
    std::multimap<QString,CompareLocalPn>& GetPnComMap(){return PnComMap;}
    const std::list<QString>& ReturnNextKeyname(const QString& CurrentKeyName,HKEY RootKey);
    QString GetMacAddress()const ;
signals:
    void CollectInFoLocalPnFinished(const std::multimap<QString,CompareLocalPn>& PnComMap);
private:
    QString ReturnStr,SerialNumber;
    std::multimap<QString,CompareLocalPn> PnComMap;
    std::unordered_multimap <QString,QString> HwidMap;
    std::list<QString> WmicReulstString;
    std::list<QString> KeyListName;
};
#endif // INFOMACHINE_H
