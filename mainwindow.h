#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "applistver.h"
#include "qfuturewatcher.h"
#include "qstandarditemmodel.h"
#include "qtextbrowser.h"
#include "soic_excel.h"
//#include "threadcrl.h"
#include "infomachine.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  void keyPressEvent(QKeyEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void resizeEvent(QResizeEvent *event);
  bool RunAsAdmin();
  void UI_InitInFoDisplay();
  void Main_RightWindow(const std::unordered_multimap<QString,QString>& H_hwmmp,const std::unordered_multimap<QString,QString>& S_swmmp);
  void HwIdsDisPlay(AppListVer* DeviceScan,std::unordered_multimap<QString,QString>& S_swmmp,std::unordered_multimap<QString,QString>& H_hwmmp);
  void HW_CheckResult(const std::unordered_multimap<QString, QString> &H_hwmmp);
  void Mpm_Me_function();
  ~MainWindow();

private:
    Ui::MainWindow *ui;
    QStandardItemModel* H_Model;
    QStandardItemModel* S_Model;
    QStandardItemModel* IDs_Model;
    QStandardItemModel* HW_resultModel;
    QStandardItemModel* HW_HhpdiagModel;
    QStandardItemModel* HW_DmYB_Model;
    Threadcrl* MainThreadCtrl;
    SOIC_Excel* Soic_Excel;
    QFutureWatcher<void>*  GetPnInfoWatcher;
    AppListVer* applistver;
    InFoMachine* info_Machine;
    std::function<void(const QStringList& TableColumnHeader,const QStringList& TableRowHeader,QStandardItemModel* Model,const QStringList& ItemData,int Row)>SetTableData;
    std::function<QString(const QString& qss)>Q_qsssetfunc;
    QTextBrowser* FloatWin;
};
#endif // MAINWINDOW_H
