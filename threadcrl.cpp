
#include "threadcrl.h"
#include "applistver.h"
#include "mpm.h"

Threadcrl::Threadcrl(QObject *parent)
    : QObject{parent},Soic_ApplistTh(new QThread),M_MpmMethread(new QThread)
{
    AppListVer* pappFindDriver =  new AppListVer;
    pappFindDriver->moveToThread(Soic_ApplistTh);

    void (Threadcrl::* p_PnFindDriverStart)(const QString& PnIniFile) = &Threadcrl::PnNumFindDriverStart;
    void (Threadcrl::* p_PnFindDriverStartList)(const QStringList& PnIniFileList) = &Threadcrl::PnNumFindDriverStart;

    void (Threadcrl::* p_PnFileInfoAnalyse)(const std::unordered_multimap<QString,QString>* PnFileInfoList) = &Threadcrl::PnFileInfoAnalyse;
    void (Threadcrl::* p_PnFileInfoAnalyseList)(const std::list<std::unordered_multimap<QString,QString>>* PnFileInfoList) = &Threadcrl::PnFileInfoAnalyse;

    const std::unordered_multimap<QString,QString>* (AppListVer::*  p_PnFindDriverFun)(const QString& PnIniFile) = &AppListVer::Pn_NumFindDriver;
    const std::list<std::unordered_multimap<QString,QString>>* (AppListVer::*  p_PnFindDriverFunList)(const QStringList& PnIniFileList) = &AppListVer::Pn_NumFindDriver;

    void (AppListVer::* p_FindDriverCompleteNote)(const std::unordered_multimap<QString,QString>* PnFileInfo) = &AppListVer::FindDriverComplete;
    void (AppListVer::* p_FindDriverComplete)(const std::list<std::unordered_multimap<QString,QString>>* PnFileInfoList) = &AppListVer::FindDriverComplete;

    connect(pappFindDriver,p_FindDriverCompleteNote,this,p_PnFileInfoAnalyse);  //Analyse Somethings

    connect(pappFindDriver,p_FindDriverComplete,this,p_PnFileInfoAnalyseList);  //Analyse Somethings

    connect(Soic_ApplistTh,&QThread::finished,pappFindDriver,&QObject::deleteLater);   ///

    connect(this,p_PnFindDriverStart,pappFindDriver,p_PnFindDriverFun,Qt::QueuedConnection);
    connect(this,p_PnFindDriverStartList,pappFindDriver,p_PnFindDriverFunList,Qt::QueuedConnection);
    Soic_ApplistTh->start();

    MPM *p_mpm = new MPM;
    p_mpm->moveToThread(M_MpmMethread);
    void (Threadcrl::*p_Mpm_start)(QString filename,QString bcu_path)=&Threadcrl::setting_startMpm;
    void (MPM::*p_mpmwork)(QString filename,QString bcu_path)=&MPM::mpmwork;
    void (Threadcrl::*p_Mestaus)(bool me,QString err_info)=&Threadcrl::setting_Me_status;

    connect(this,p_Mpm_start,p_mpm,p_mpmwork,Qt::QueuedConnection);
    connect(p_mpm,&MPM::MpmMe_status,this,p_Mestaus);                              //信号向上传递
    M_MpmMethread->start();

}
Threadcrl::~Threadcrl()
{
    Soic_ApplistTh->quit();
    Soic_ApplistTh->wait();
    delete Soic_ApplistTh;
}
void Threadcrl::PnFileInfoAnalyse(const std::list<std::unordered_multimap<QString,QString>>* PnFileInfoList)
{
    foreach (auto var, *PnFileInfoList) {
        std::unordered_multimap<QString,QString>::const_iterator Out;
        for (Out = var.equal_range("General").first; Out != var.equal_range("General").second; ++Out) {
               PnAnalyseList.append(Out->second);
        }
    }
    emit PnNumFindDriverStopTh(PnAnalyseList);
}
void Threadcrl::PnFileInfoAnalyse(const std::unordered_multimap<QString,QString>* PnFileInfo)
{
    std::unordered_multimap<QString,QString>::const_iterator Out;
    for (Out = PnFileInfo->equal_range("General").first; Out != PnFileInfo->equal_range("General").second; ++Out) {
        if (Out->second.contains("PN=")){
               if (PnAnalyseList.contains(Out->second))
                   continue;
               qDebug()<<Out->second;
               PnAnalyseList.append(Out->second);
        }
    }
     emit PnNumFindDriverStopTh(PnAnalyseList);
}













