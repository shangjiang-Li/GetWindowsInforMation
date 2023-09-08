
#ifndef THREADCRL_H
#define THREADCRL_H


#include <QObject>
#include<QThread>

class Threadcrl : public QObject
{
    Q_OBJECT
public:
    explicit Threadcrl(QObject *parent = nullptr);
    ~Threadcrl();

signals:
    void setting_startMpm(QString filename,QString bcu_path);
    void setting_Me_status(bool Me,QString err_info);

    void PnNumFindDriverStart(const QString& PnIniFile);
    void PnNumFindDriverStart(const QStringList& PnIniFileList);
    void PnNumFindDriverStopTh(const QString& PnInFo);
    void PnNumFindDriverStopTh(const QStringList& PnInFoList);
private slots:
    void PnFileInfoAnalyse(const std::unordered_multimap<QString,QString>* PnFileInfo);
    void PnFileInfoAnalyse(const std::list<std::unordered_multimap<QString, QString> > *PnFileInfoList);
private:
    QThread*  Soic_ApplistTh;
    QThread*  M_MpmMethread;
    QStringList PnAnalyseList;
};

#endif // THREADCRL_H
