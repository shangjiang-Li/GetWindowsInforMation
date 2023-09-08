#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <QMessageBox>
#include <QFutureWatcher>
#include <QFuture>
#include  <QDockWidget>
#include <QStyleFactory>
#include "delegate_excel.h"
#include "mainwindow.h"
#include "applistver.h"
#include "qevent.h"
#include "smbios.h"
#include "ui_SOIC.h"
#include "DumpSMBIOS.h"
#include <DefStrings.h>
//#include "TabBarStyle"
//#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),ui(new Ui::MainWindow),H_Model(new QStandardItemModel(this)),
    S_Model(new QStandardItemModel(this)),IDs_Model(new QStandardItemModel(this)),HW_resultModel(new QStandardItemModel(this)),
    HW_HhpdiagModel(new QStandardItemModel(this)),
    HW_DmYB_Model(new QStandardItemModel(this)), MainThreadCtrl(new Threadcrl(this)),Soic_Excel(new SOIC_Excel(this)),
    GetPnInfoWatcher(new  QFutureWatcher<void>),applistver(new AppListVer(this)),info_Machine(new InFoMachine(this))
{
        ui->setupUi(this);
        QDate usedate = QDate::currentDate();
        int date_Y,date_M,date_D;
        usedate.getDate(&date_Y,&date_M,&date_D);
        if ( date_Y >= 2025 || date_Y<=2020)  exit(0);
        if (!RunAsAdmin()){
            QMessageBox::critical(this,"Administrator","Please use Administrator Run");
            exit(0);
        }
#ifdef DEBUG
        DumpSMBIOS dumpsmbios;
        dumpsmbios.DumpsmbiosMain();
        QDir swpnfile(SWPOLISTDIR);
        if (!swpnfile.exists()) QMessageBox::information(this,"SW PN FLODER","Please Check  SW PN floder exists  " SWPOLISTDIR);
#endif
        QFuture<void> GetPnInfo = QtConcurrent::run([=](){  info_Machine->CollectInFoLocalPn(); });
        GetPnInfoWatcher->setFuture(GetPnInfo);

        void ( MainWindow::* p_Mpm_Me_function)() = &MainWindow::Mpm_Me_function;
        connect(ui->Bcu_pushButton,&QPushButton::clicked,this,p_Mpm_Me_function);
        connect(MainThreadCtrl,&Threadcrl::setting_Me_status,this,[=](bool Mestatus,QString err_info){
            if(Mestatus)
                ui->ME_label->setText("Lock");
            else
               ui->ME_label->setText("Unlock");

            if (err_info.contains("*Lock_mpm"))
               ui->MPM_label->setText("Lock");

            if (err_info.contains("*Unlock_mpm"))
               ui->MPM_label->setText("UnLock");

            if (err_info.contains("*Disable_txt")) {
               ui->TXT_label->setText("Disable");
            }else if (err_info.contains("*Enable_txt")) {
               ui->TXT_label->setText("Enable");
            }else {
               ui->TXT_label->setText("NA");
            }
        });
  /*
   *
   * Excle Functions
   *
*/
        connect(GetPnInfoWatcher,&QFutureWatcher<void>::finished,this,[=](){
            connect(ui->ML_pushButton,&QPushButton::clicked,this,[=](){
                QString filename;
                filename = QFileDialog::getOpenFileName(this,tr("Excel File"),".",tr("Excel (*.xlsx *.xlsm);;(*.*)"));
                QFileInfo E_mlfile(filename);
                if (!filename.isEmpty()) {
                    if(Soic_Excel->SOIC_ExcelOpen(filename))
                    {
                        ui->Ml_Line->setPlaceholderText(filename);
                        ui->tabWidget_excel->setTabText(0,E_mlfile.fileName());
                        Soic_Excel->SOIC_SheetOpen(ui->ML_TableView,*info_Machine,ui->tabWidget_excel);
                    }
                }
            });
        });
        connect(ui->Main_Tab,&QTabWidget::currentChanged,this,[=](int index){
            if(2==index)
                ui->Bcu_Ml_List_Widget->show();
            else
                ui->Bcu_Ml_List_Widget->hide();
        });

        std::unordered_multimap<QString,QString> HW_hardwareitemData,SW_softwareitemData;
       const SMBIOS &SmBios = SMBIOS::getInstance();
        HW_hardwareitemData.emplace("SN",QString::fromStdWString(SmBios.SysSerialNumber()));
        info_Machine->Get_SetSerialNumber(QString::fromStdWString(SmBios.SysSerialNumber()));
        HW_hardwareitemData.emplace("MAC",info_Machine->GetMacAddress().simplified().toUpper());
        QStringList DDR_Info;
        QString D_displaystr;
        QVector<QString> Dis_vecstr;
        DDR_Info<<DMIQUERYDDRINFO;
        std::list<QString> I_infomemory = info_Machine->WmicQuery("Win32_PhysicalMemory",DDR_Info);
        int count = 0;
        static QRegularExpression reg("[0-9]+");
        foreach (QString wmistr,I_infomemory ) {
            if (wmistr.contains("Capacity"))
                wmistr.replace(reg,QString::number(wmistr.split(":").last().toLongLong()/1024/1024/1024)+"GB");
            if (wmistr.contains("Speed"))
                wmistr+="MHz";
            D_displaystr.append(wmistr.split(":").last());
            if (4==count++) {
                count = 0;
                Dis_vecstr.push_back(D_displaystr);
                D_displaystr.clear();
            }
        }
        count = 1;
        foreach (QString var , Dis_vecstr) {
            HW_hardwareitemData.emplace("RAM"+QString::number(count++),var.simplified());
        }

        SW_softwareitemData.emplace("ML",info_Machine->Reg2DisPlay("BuildID","SOFTWARE\\Hewlett-Packard\\CommonInfo",HKEY_LOCAL_MACHINE));
        SW_softwareitemData.emplace("BIOS",info_Machine->Reg2DisPlay("BIOSVersion","HARDWARE\\DESCRIPTION\\System\\BIOS",HKEY_LOCAL_MACHINE));

        std::unordered_multimap<QString,QString>::const_iterator swfindit;
        QString buildid,biosver;
            swfindit = SW_softwareitemData.find("ML");
            if ( swfindit != SW_softwareitemData.end())
                    if (swfindit->second.isEmpty()){
                                  if((buildid = dumpsmbios.GetBiosBuild()).isEmpty())
                                             SW_softwareitemData.emplace("ML","CleanOS");
                                   else   SW_softwareitemData.emplace("ML",buildid);
                               }
            swfindit = SW_softwareitemData.find("BIOS");
            if ( swfindit != SW_softwareitemData.end())
                               if (swfindit->second.isEmpty()){
                                   if(!(biosver = dumpsmbios.GetBiosVer()).isEmpty())
                                            SW_softwareitemData.emplace("BIOS",biosver);
                               }
        UI_InitInFoDisplay();
        HwIdsDisPlay(applistver,SW_softwareitemData,HW_hardwareitemData);
        Main_RightWindow(HW_hardwareitemData,SW_softwareitemData);
        HW_CheckResult(HW_hardwareitemData);
//HP PC Hardware Diagnostics Windows
        connect(ui->H_hwpushButton,&QPushButton::clicked,this,[=](){
            QString Start_UpAppStr;
            foreach (QString var, info_Machine->ReturnNextKeyname(APPXGREPATH,HKEY_LOCAL_MACHINE)) {
                if (var.contains(HPDIAAPPNAME)) {
                    Start_UpAppStr = info_Machine->ReturnNextKeyname(QString(APPXGREPATH)+"\\"+var,HKEY_LOCAL_MACHINE).front();
                }
            }
            if (!Start_UpAppStr.isEmpty())
                 system(APPXSTARUPCMD+Start_UpAppStr.toUtf8());
        });
//HP PC Hardware Diagnostics Windows
}
void MainWindow::UI_InitInFoDisplay()
{
        setMouseTracking(true);
        centralWidget()->setMouseTracking(true);
        ui->Main_Tab->setMouseTracking(true);
        ui->MainWin->setMouseTracking(true);
        ui->MainWin->setAttribute(Qt::WA_Hover,true);
        ui->Main_Tab->currentWidget()->setMouseTracking(true);
        setCentralWidget(nullptr);//Layout setting
        ui->Main_Tab->setParent(this);//Layout setting
        ui->Bcu_FilelineEdit->setPlaceholderText("upload BiosConfigUtility64.exe");
        ui->Main_Tab->setFocus();
        ui->MAC_lineEdit->setInputMask("HH:HH:HH:HH:HH:HH;_");
        ui->Search_lineEdit->hide();
        ui->IDs_DownpushButton->hide();
        ui->IDs_UppushButton->hide();
        if (ui->tabWidget_excel->tabText(0).isEmpty())
            ui->tabWidget_excel->setTabText(0,"Samplete.xlsx");
        ui->tabWidget_excel->setTabBarAutoHide(true);
        ui->Main_Tab->setAttribute(Qt::WA_StyledBackground);
        //ui->Main_Tab->setBackgroundRole(QPalette::Shadow);
        ui->Main_Tab->tabBar()->hide();
        ui->Main_Tab->setTabToolTip(0,"Main");
        FloatWin = new QTextBrowser(ui->Main_Tab);
        FloatWin->append(INFOAPPTIAL);
        FloatWin->setFont(QFont("Arial",20));
        FloatWin->setMinimumWidth(20);
        FloatWin->setMinimumHeight(10);
        FloatWin->setMaximumWidth(200);
        FloatWin->setFixedHeight(500);
        FloatWin->move(0,520);//
        FloatWin->setDisabled(true);
        FloatWin->hide();
        ui->Bcu_widget->setFixedWidth(420);
        ui->Bcu_widget->setFixedHeight(280);
        std::function<QByteArray (const QString&)> M_styleSheet = std::bind([=](QString FilePath){
            QFile q_qssfile(FilePath);
            QByteArray Read_Alltemp;
            if (!q_qssfile.exists())
                return (QByteArray)NULL;
            if(q_qssfile.open(QFile::ReadOnly))
            {
                Read_Alltemp = q_qssfile.readAll();
                q_qssfile.close();
                return Read_Alltemp;
            }else {
                q_qssfile.close();
                return (QByteArray)NULL;
            }
        },std::placeholders::_1);
        if(!Q_qsssetfunc)  Q_qsssetfunc = M_styleSheet;

        FloatWin->setStyleSheet(M_styleSheet(QSS_MAIN));
        ui->tabWidget_excel->setStyleSheet(M_styleSheet(QSS_TABWID));
        ui->Main_Tab->setStyleSheet(M_styleSheet(QSS_MAIN));        //setStyleSheet(M_styleSheet(QSS_MAIN));
        //ui->Main_Tab->tabBar()->setStyle(new TabBarStyle());          //font postions
        ui->H_label->setStyleSheet(M_styleSheet(QSS_LABLE));
        ui->S_label->setStyleSheet(M_styleSheet(QSS_LABLE));

        ui->H_hardwaretableView->setStyleSheet(M_styleSheet(QSS_TABLEVIEW)+M_styleSheet(QSS_HEADER));
        ui->S_softwaretableView->setStyleSheet(M_styleSheet(QSS_TABLEVIEW)+M_styleSheet(QSS_HEADER));
        ui->I_IDtableView->setStyleSheet(M_styleSheet(QSS_TABLEVIEW)+M_styleSheet(QSS_HEADER));

        ui->Main_Tab->setCurrentIndex(0);
        ui->statusbar->addWidget(new QLabel(APPVER));

        ui->H_hardwaretableView->horizontalHeader()->setStyleSheet("background-color: transparent;");
        ui->H_hardwaretableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->H_hardwaretableView->setShowGrid(false);
        ui->H_hardwaretableView->setDisabled(true);
        ui->H_hardwaretableView->horizontalHeader()->setStretchLastSection(true);
        ui->H_hardwaretableView->verticalHeader()->hide();
        ui->H_hardwaretableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

        ui->S_softwaretableView->horizontalHeader()->setStyleSheet("background-color: transparent;");
        ui->S_softwaretableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->S_softwaretableView->resizeColumnsToContents();
        ui->S_softwaretableView->setShowGrid(false);
        ui->S_softwaretableView->setDisabled(true);
        ui->S_softwaretableView->horizontalHeader()->setStretchLastSection(true);
        ui->S_softwaretableView->verticalHeader()->hide();
        ui->S_softwaretableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

        ui->HW_resulttableView->setShowGrid(false);
        ui->HW_resulttableView->horizontalHeader()->setStretchLastSection(true);
        ui->HW_resulttableView->verticalHeader()->hide();
        ui->HW_resulttableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        ui->HW_resulttableView->setStyleSheet(Q_qsssetfunc(QSS_HEADER_RSULT)+Q_qsssetfunc(QSS_TABLEVIEW));
        ui->HW_resulttableView->setAlternatingRowColors(true);

        ui->HW_CheckYBtableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->HW_CheckYBtableView->setShowGrid(false);
        ui->HW_CheckYBtableView->horizontalHeader()->setStretchLastSection(true);
        ui->HW_CheckYBtableView->verticalHeader()->hide();
        ui->HW_CheckYBtableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        ui->HW_CheckYBtableView->setStyleSheet(Q_qsssetfunc(QSS_HEADER_HWYB)+Q_qsssetfunc(QSS_TABLEVIEW));
        ui->HW_CheckYBtableView->setAlternatingRowColors(true);

        ui->Hp_DiagtableView->setItemDelegateForColumn(1,new delegate_excel(ui->Hp_DiagtableView));
        ui->Hp_DiagtableView->setShowGrid(false);
        ui->Hp_DiagtableView->horizontalHeader()->setStretchLastSection(true);
        ui->Hp_DiagtableView->verticalHeader()->hide();
        ui->Hp_DiagtableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        ui->Hp_DiagtableView->setStyleSheet(Q_qsssetfunc(QSS_HEADER_DIGA)+Q_qsssetfunc(QSS_TABLEVIEW));
        ui->Hp_DiagtableView->setAlternatingRowColors(true);

        ui->I_IDtableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->I_IDtableView->horizontalHeader()->setSortIndicatorShown(true);                                     //sort by header
        ui->I_IDtableView->horizontalHeader()->setSortIndicator(0,Qt::SortOrder::AscendingOrder);   //sort by header
        ui->I_IDtableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        ui->I_IDtableView->setStyleSheet(Q_qsssetfunc(QSS_HEADER_IDS)+Q_qsssetfunc(QSS_TABLEVIEW));
        ui->I_IDtableView->setAlternatingRowColors(true);

}
void MainWindow::Main_RightWindow(const std::unordered_multimap<QString,QString>& H_hwmmp,const std::unordered_multimap<QString,QString>& S_swmmp)
{
    QStringList HH_HeaderItem,SH_HeaderItem,HV_HeaderItem,SV_HeaderItem,H_ItemData,S_ItemData;
    std::function<void(const QStringList& HSetstr,const QStringList& VSetstr,QStandardItemModel* Model,const QStringList& ItemData,int Column)>
        SetViewData = std::bind([=](const QStringList& HSetstr,const QStringList& VSetstr,QStandardItemModel* Model,const QStringList& ItemData,int Column){
            if (!HSetstr.isEmpty()) {
                for (int var = 0; var < HSetstr.count(); ++var) {//Table Col Header
                    Model->setHorizontalHeaderItem(var,new QStandardItem(HSetstr[var]));
                }
            }
            if (!VSetstr.isEmpty()) {
                for (int var = 0; var < VSetstr.count(); ++var) {//Table Row Header
                    Model->setItem(var,0,new QStandardItem(VSetstr[var]));
                }
            }
            if (!ItemData.isEmpty()) {
                    for (int var = 0; var < ItemData.count(); ++var) {//Row Insert data
                        Model->setItem(var,Column,new QStandardItem(ItemData[var]));
                    }
            }
        },std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5);
    if (!SetTableData)  SetTableData = SetViewData;

        HH_HeaderItem<<H_TABLECOLUMNHEADER;
        SH_HeaderItem<<S_TABLECOLUMNHEADER;
        HV_HeaderItem<<H_TABLEROWHEADER;
        SV_HeaderItem<<S_TABLEROWHEADER;

        std::unordered_multimap<QString,QString>::const_iterator hwit,swit;
        foreach (QString KeyValue, HV_HeaderItem) {
           hwit = H_hwmmp.find(KeyValue.simplified());
           if ( hwit != H_hwmmp.end())
                 H_ItemData.append(hwit->second);
           else H_ItemData.append("NA");
        }
        foreach (QString KeyValue, SV_HeaderItem) {
           swit = S_swmmp.find(KeyValue.simplified());
           if ( swit != S_swmmp.end())
                 S_ItemData.append(swit->second);
           else S_ItemData.append("NA");
        }
        SetViewData(HH_HeaderItem,HV_HeaderItem,H_Model,H_ItemData,1);
        SetViewData(SH_HeaderItem,SV_HeaderItem,S_Model,S_ItemData,1);

        ui->H_hardwaretableView->setModel(H_Model);
        ui->H_hardwaretableView->resizeColumnsToContents();

        ui->S_softwaretableView->setModel(S_Model);
        ui->S_softwaretableView->resizeColumnsToContents();

        connect(ui->S_pushButton,&QPushButton::clicked,ui->H_pushButton,[=](){emit ui->H_pushButton->clicked();});
        connect(ui->H_pushButton,&QPushButton::clicked,this,[=](){
            QString   Exportname = QFileDialog::getSaveFileName(this,tr("Export File"),"SystemInFormation",tr("txt (*.txt *.log)"));
            if(Exportname.isEmpty())
                                           return;
                            QFile ExportFile(Exportname);
                            QTextStream out(&ExportFile);
                            if (ExportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                                for (int var_row = 0; var_row < H_Model->rowCount(); ++var_row) {
                                    for (int var_col = 0; var_col < H_Model->columnCount(); ++var_col) {
                                        out <<H_Model->index(var_row,var_col).data().toString()+"\n";
                                    }
                                }
                                for (int var_row = 0; var_row < S_Model->rowCount(); ++var_row) {
                                    for (int var_col = 0; var_col < S_Model->columnCount(); ++var_col) {
                                        out <<S_Model->index(var_row,var_col).data().toString()+"\n";
                                    }
                                }
                            }
                            ExportFile.close();
        });
}
void MainWindow::HwIdsDisPlay(AppListVer* DeviceScan,std::unordered_multimap<QString,QString>& S_swmmp,std::unordered_multimap<QString,QString>& H_hwmmp)
{
        std::variant<bool,QStringList> D_dmstatus = DeviceScan->DeviceCheck();
        if(const auto check = std::get_if<bool>(&D_dmstatus))
        {
           ui->M_messagetextBrowser->append(DM_SCANFPASSSTRING);
        }else {
           foreach (QString var, std::get<QStringList>(D_dmstatus)) {
               ui->M_messagetextBrowser->append(var);
           }
        }
       QStringList  HeaderItem;
        HeaderItem<<HWIDSTABLEHEADER;
        QString S_softversion;

        for (int var = 0; var < HeaderItem.count(); ++var) {
           IDs_Model->setHorizontalHeaderItem(var,new QStandardItem(HeaderItem[var]));
        }
        int row = 0;
        static QRegularExpression reg(".+_");
        foreach (struct DeviceInfo var, DeviceScan->GetDeviceInfo2DM()) {
           if (var.sezDescBuf.isEmpty() && var.FirendName.isEmpty())
                 continue;
                         std::unordered_multimap<QString,QString> Devlistmmp;
                         if (var.sezClassBuff.isEmpty())
                                 continue;
                         QString PciDevice = "SYSTEM\\CurrentControlSet\\Control\\Class";
                         QStringList DevListInfo;
                         DevListInfo<<"MatchingDeviceId"<<"DriverVersion";
                        Devlistmmp =  applistver->Reg2DisPlay(DevListInfo,PciDevice+"\\"+var.Driver,HKEY_LOCAL_MACHINE,"#");
                        for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
                                 if (varmmp->second.isEmpty())
                                        continue;
                            static QRegularExpression re("(\\d+\\.\\d+\\.\\d+\\.\\d+)");
                            if (var.sezClassBuff == "Display") {
                                    QRegularExpressionMatch match = re.match(varmmp->second);
                                    if (match.hasMatch()) {
                                            S_softversion.clear();
                                            S_softversion = match.captured(0);
                                    }
                                    if (var.FirendName.contains("Intel(R)") || var.sezDescBuf.contains("Intel(R)"))
                                            S_swmmp.emplace("VGA", S_softversion);
                                    else  S_swmmp.emplace("DGPU", S_softversion);
                            }
                            if (var.sezClassBuff == "DiskDrive") {
                                    static int disk_count = 1;
                                    if (!var.FirendName.isEmpty())
                                            H_hwmmp.emplace("SSD"+QString::number(disk_count++),var.FirendName);
                                    else H_hwmmp.emplace("SSD"+QString::number(disk_count++),var.sezDescBuf);
                            }
                            if (var.sezClassBuff == "Processor") {
                                    static bool SetBool = true;
                                    if (SetBool) {
                                         SetBool = false;
                                        if (!var.FirendName.isEmpty())
                                                H_hwmmp.emplace("CPU",var.FirendName);
                                        else H_hwmmp.emplace("CPU",var.sezDescBuf);
                                    }
                            }
                            if (var.sezClassBuff == "Net" && (var.FirendName.contains("802.11") || var.FirendName.contains("Realtek") ||  var.FirendName.contains("Intel(R)") || var.FirendName.contains("WiFi") ||var.FirendName.contains("Wi-Fi")
                                                              || var.sezDescBuf.contains("802.11") || var.sezDescBuf.contains("Realtek") ||  var.sezDescBuf.contains("Intel(R)") || var.sezDescBuf.contains("WiFi") ||var.sezDescBuf.contains("Wi-Fi"))) {                             //Wlan && Lan
                                    if (var.FirendName.contains("Virtual") || var.sezDescBuf.contains("Virtual")){
                                        //skip                    Microsoft Wi-Fi Direct Virtual Adapter      Microsoft Wi-Fi Direct Virtual Adapter #2
                                    }
                                    else {
                                                QRegularExpressionMatch match = re.match(varmmp->second);
                                                if (match.hasMatch()) {
                                                    S_softversion.clear();
                                                    S_softversion = match.captured(0);
                                                }
                                                if ( (var.FirendName.contains("Realtek") || var.FirendName.contains("Intel(R)") || var.sezDescBuf.contains("Realtek") || var.sezDescBuf.contains("Intel(R)")) ) { //Wlan && Lan
                                                    if (var.FirendName.contains("802.11")|| var.FirendName.contains("WiFi") ||var.FirendName.contains("Wi-Fi")||var.sezDescBuf.contains("WiFi") ||var.sezDescBuf.contains("Wi-Fi")){       //WLan
                                                        if (var.FirendName.isEmpty())
                                                            H_hwmmp.emplace("Wlan Card",var.FirendName);
                                                        else
                                                            H_hwmmp.emplace("Wlan Card",var.sezDescBuf);
                                                        S_swmmp.emplace("Wlan", S_softversion);
                                                    }else{
                                                        S_swmmp.emplace("LAN", S_softversion);                                                                                        // Lan
                                                    }
                                                }
                                    }

                            }
                            if (!var.HardWareID.isEmpty()){                                                                                                                          // USB\Class_0e#10.0.22000.1042#
                                     QRegularExpressionMatch match_ver = re.match(varmmp->second);
                                     if (match_ver.hasMatch())
                                          var.HardWareID.append("#"+match_ver.captured());                                                                     //HID_DEVICE_SYSTEM_MOUSE#10.0.22000.653
                            }else{                                                                                                                                                                  //pci\ven_8086&dev_3e93&subsys_84ee103c#25.20.100.6446#
                                     QRegularExpressionMatch match_ver = re.match(varmmp->second);
                                     if (match_ver.hasMatch()) {
                                          static QRegularExpression remove("#.+");
                                          var.HardWareID = varmmp->second.replace(remove,"#"+match_ver.captured());                     //BTHENUM\{c7f94713-891e-496a-a0e7-983a0946126e}#10.0.22000.1936
                                     }
                            }
                        }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (var.sezClassBuff == "Biometric") {
            if (!var.FirendName.isEmpty()){
                if (var.FirendName.contains("Fingerprint") || var.FirendName.contains("fingerprint"))
                        H_hwmmp.emplace("Fingerprint",var.FirendName+"##"+var.HardWareID);
            }else {
                if (var.sezDescBuf.contains("Fingerprint") || var.sezDescBuf.contains("fingerprint"))
                        H_hwmmp.emplace("Fingerprint",var.sezDescBuf+"##"+var.HardWareID);
            }
        }
        if (var.sezClassBuff == "Camera") {
            static int Camera_count = 1;
            if (!var.FirendName.isEmpty())
                H_hwmmp.emplace("Camera"+QString::number(Camera_count++),var.FirendName+"##"+var.HardWareID);
            else H_hwmmp.emplace("Camera"+QString::number(Camera_count++),var.sezDescBuf+"##"+var.HardWareID);
        }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  table       "Item"<<"Subitem"<<"Value"<<"DriverVersion"<<"VID"<<"PID"<<"REV"
///                     value        value        value        value        value        value        value        value
///                    value        value        value        value        value        value        value        value
/// table       "Item"<<"Subitem"<<"DeviceName"<<"DriverVersion"<<"VID"<<"PID"<<"REV"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
           std::function<void(int&,const QString&)> H_hwidModelSetting = std::bind([=](int& Row,const QString& Subitem){
                       IDs_Model->setItem(Row,HeaderItem.indexOf("Item"),new QStandardItem(var.sezClassBuff));
                       IDs_Model->setItem(Row,HeaderItem.indexOf("Subitem"),new QStandardItem(Subitem));
                       IDs_Model->setItem(Row,HeaderItem.indexOf("Value"),new QStandardItem(var.HardWareID.split("#").first()));
                       IDs_Model->setItem(Row,HeaderItem.indexOf("DriverVersion"),new QStandardItem(var.HardWareID.split("#").last()));
                       if (var.HardWareID.contains("VID_") || var.HardWareID.contains("Vid_") || var.HardWareID.contains("vid_") || var.HardWareID.contains("Pid_")) {
                           QString V_vid,P_pid,R_rev,hwid_str;
                           QStringList VPR_string;
                           hwid_str = var.HardWareID.split("#").first();                 //Nomal HardWareIDs
                           VPR_string = hwid_str.split("&");
                           V_vid = VPR_string.first().replace(reg,"");
                           P_pid = VPR_string.last().replace(reg,"");
                           if(VPR_string.count()>=3){
                               P_pid = VPR_string[1].replace(reg,"");
                               static QRegularExpression reg_ver("_(\\d+)");
                               QRegularExpressionMatch match = reg_ver.match(VPR_string[2]);
                                   if (match.hasMatch())
                                            R_rev =match.captured().remove("_");
                                   else
                                            R_rev = "NA";
                           }
                           else
                               R_rev = "NA";
                           IDs_Model->setItem(Row,HeaderItem.indexOf("VID"),new QStandardItem(V_vid));
                           IDs_Model->setItem(Row,HeaderItem.indexOf("PID"),new QStandardItem(P_pid));
                           IDs_Model->setItem(Row,HeaderItem.indexOf("REV"),new QStandardItem(R_rev));
                       }else{
                           IDs_Model->setItem(Row,HeaderItem.indexOf("VID"),new QStandardItem("NA"));
                           IDs_Model->setItem(Row,HeaderItem.indexOf("PID"),new QStandardItem("NA"));
                           IDs_Model->setItem(Row,HeaderItem.indexOf("REV"),new QStandardItem("NA"));
                       }
                       Row++;
                   },std::placeholders::_1,std::placeholders::_2);
            if (!var.sezDescBuf.isEmpty()){
                if (!var.FirendName.isEmpty())
                    H_hwidModelSetting(row,var.FirendName);
                else
                    H_hwidModelSetting(row,var.sezDescBuf);
            } else if (!var.FirendName.isEmpty())
                        H_hwidModelSetting(row,var.FirendName);
                    else
                        H_hwidModelSetting(row,var.sezDescBuf);
        }

        ui->I_IDtableView->setModel(IDs_Model);
        ui->I_IDtableView->resizeColumnsToContents();

        static int find_count = 0;
        static int id_col = 0;
        static QModelIndexList indexes;
        static QModelIndexList indexes_trace;
        static int id_col_trace = 0;
        connect(ui->IDs_DownpushButton,&QPushButton::clicked,this,[=](){
            QModelIndex ix;
            if (find_count<indexes.count() && !indexes.isEmpty() ){
                ix = indexes.at(find_count++);
                ui->I_IDtableView->setCurrentIndex(ix);
                ui->I_IDtableView->scrollTo(ix);
            }else {
                        find_count = 0;
                        ix = indexes.at(find_count++);
                        ui->I_IDtableView->setCurrentIndex(ix);
                        ui->I_IDtableView->scrollTo(ix);
            }
        });
        connect(ui->IDs_UppushButton,&QPushButton::clicked,this,[=](){
            QModelIndex ix;
            if (find_count >0 && !indexes.isEmpty() ){
                ix = indexes.at(find_count--);
                ui->I_IDtableView->setCurrentIndex(ix);
                ui->I_IDtableView->scrollTo(ix);
            }else {
                        find_count = indexes.count();
                        ix = indexes.at(find_count--);
                        ui->I_IDtableView->setCurrentIndex(ix);
                        ui->I_IDtableView->scrollTo(ix);
            }
        });
        connect(ui->Search_lineEdit,&QLineEdit::returnPressed,this,[=](){
            ui->IDs_DownpushButton->setEnabled(true);
            ui->IDs_UppushButton->setEnabled(true);
            if(id_col>IDs_Model->columnCount())
                    id_col = 0;
            if (!ui->Search_lineEdit->text().isEmpty())
                    do {
                            indexes = ui->I_IDtableView->model()->match(ui->I_IDtableView->model()->index(0,id_col++),Qt::EditRole,ui->Search_lineEdit->text(),-1, Qt::MatchFlags(Qt::MatchContains|Qt::MatchWrap));
                    } while (indexes.isEmpty() && id_col<IDs_Model->columnCount());
        });
        connect(ui->I_IDtableView->horizontalHeader(),&QHeaderView::sortIndicatorChanged,this,[=](int Logicalindex,Qt::SortOrder order){IDs_Model->sort(Logicalindex,order);});


//Bug with there
        connect(ui->Search_lineEdit,&QLineEdit::textEdited,this,[=](QString Search){
            do {
                     indexes_trace = ui->I_IDtableView->model()->match(ui->I_IDtableView->model()->index(0,id_col_trace++),Qt::EditRole,Search,-1, Qt::MatchFlags(Qt::MatchContains|Qt::MatchWrap));
            } while (indexes_trace.isEmpty() && id_col_trace<IDs_Model->columnCount());
            if ( IDs_Model->columnCount() == id_col_trace)
                       id_col_trace = 0;
            if(!indexes_trace.isEmpty()){
                ui->I_IDtableView->setCurrentIndex(indexes_trace.first());
                ui->I_IDtableView->scrollTo(indexes_trace.first());
            }
    });
        connect(ui->H_downloadpushButton,&QPushButton::clicked,this,[=](){
            QString D_downloadstr = "HwIDs";
            if (!H_Model->index(0,1).data().toString().isEmpty())
                    D_downloadstr = H_Model->index(0,1).data().toString().simplified();
            QString   Downloadname = QFileDialog::getSaveFileName(this,tr("Download Excel File"),D_downloadstr,tr("Excel (*.xlsx)"));
            if(Downloadname.isEmpty())
                                                return;
                QFile D_downloadFile(Downloadname);
                if (D_downloadFile.exists())
                        D_downloadFile.remove();
                QXlsx::Document Ids_doc(Downloadname);
                Ids_doc.addSheet(D_downloadstr);
                for (int var_row = 0; var_row < ui->I_IDtableView->model()->rowCount(); ++var_row) {
                        for (int var_col = 0; var_col < ui->I_IDtableView->model()->columnCount(); ++var_col) {
                            Ids_doc.currentWorksheet()->write(1+var_row,1+var_col,ui->I_IDtableView->model()->index(var_row,var_col).data());
                        }
                }
                if (Ids_doc.save())
                        QMessageBox::information(this,Downloadname,"Download Sucessfully!  "+Downloadname);
                else QMessageBox::critical(this,Downloadname,"Download Error: "+Downloadname);
        });
}
MainWindow::~MainWindow()
{
    delete ui;
    if (H_Model)
        delete H_Model;
    if (S_Model)
        delete S_Model;
    if (IDs_Model)
        delete IDs_Model;
    if (HW_resultModel)
        delete HW_resultModel;
    if (HW_HhpdiagModel)
        delete HW_HhpdiagModel;
    if (HW_DmYB_Model)
        delete HW_DmYB_Model;
    if (MainThreadCtrl)
        delete MainThreadCtrl;
    if (Soic_Excel)
        delete Soic_Excel;
    if (GetPnInfoWatcher)
        delete GetPnInfoWatcher;
    if (applistver)
        delete applistver;
    if (info_Machine)
        delete info_Machine;
    if (FloatWin)
        delete FloatWin;
}
void MainWindow::Mpm_Me_function()
{
    QString filename;
           filename = QFileDialog::getOpenFileName(this, tr("Open File"),"C:\\", tr("EXE (*.exe)"));
    if(filename.isEmpty()==true) {
           QMessageBox::critical(this,"Error","Please input BiosConfigUtility/BCU EXE File");
           return ;
    }
    if(!filename.contains("BiosConfigUtility")){
           QMessageBox::critical(this,"Error","Please check file name is contains BiosConfigUtility");
           return ;
    }
    connect(ui->Mac_pushButton,&QPushButton::clicked,this,[=](){
        QFileInfo bcuinfo(filename);
        QDir InfPath(bcuinfo.absolutePath());
        QString InStallCMD,I_infFile;
        InfPath.setFilter( QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Files);
        if (!InfPath.entryInfoList().isEmpty()) {
           foreach (QFileInfo var, InfPath.entryInfoList()) {
                if (var.fileName().simplified().contains(".inf")){
                     I_infFile = var.fileName();
                    break;
               }
            }
        }
        InStallCMD = "RUNDLL32.EXE SETUPAPI.DLL, InstallHinfSection DefaultInstall 132 "+InfPath.absolutePath()+"\\"+I_infFile;
        if(!system(InStallCMD.toUtf8())) {
            QRegularExpression rx("[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}");
            QValidator *validator = new QRegularExpressionValidator(rx);
            ui->MAC_lineEdit->setValidator(validator);
            QFile MacFile(InfPath.absolutePath()+"\\"+MAC_FILENAME);
            QTextStream out(&MacFile);
            if (MacFile.open(QIODevice::WriteOnly | QIODevice::Text))
                           out <<ui->MAC_lineEdit->text().replace(":","");
            MacFile.close();
            ui->M_messagetextBrowser->append(InfPath.absolutePath()+MAC_WRITECMD+InfPath.absolutePath()+"\\"+MAC_FILENAME);
            ui->M_messagetextBrowser->append(QString::number(system( (InfPath.absolutePath()+MAC_WRITECMD+InfPath.absolutePath()+"\\"+MAC_FILENAME).toUtf8() )));
        }
    });

    if (!system(filename.toUtf8()+"   -getconfig:"+BIOS_CONFIG_FILE)){
           emit MainThreadCtrl->setting_startMpm(BIOS_CONFIG_FILE,filename);
    }else return ;
}
void MainWindow::HW_CheckResult(const std::unordered_multimap<QString, QString> &H_hwmmp)
{
    QStringList Table_ColHeader,Table_RowHeader,ItemData;
    std::function<void(unsigned int Ms_times)> CheckNB_DT = std::bind([=](unsigned int Ms_times ){
        QTimer* scantime = new QTimer;
        scantime->start(Ms_times);
        connect(scantime,&QTimer::timeout,this,[=](){
            QString PowerInFo;
            SYSTEM_POWER_STATUS SystemPowerStatus;
            if(GetSystemPowerStatus(&SystemPowerStatus)){
                if (128==SystemPowerStatus.BatteryFlag )
                    ui->H_hwstatuspushButton->setText("DT");
                else ui->H_hwstatuspushButton->setText("NoteBook");
                if (128!=SystemPowerStatus.BatteryFlag) {
                    if (!SystemPowerStatus.ACLineStatus)
                        PowerInFo.append("AC: OFF\n");
                    else if(SystemPowerStatus.ACLineStatus)
                        PowerInFo.append("AC: On\n");
                    if (255!=SystemPowerStatus.BatteryLifePercent )
                        PowerInFo.append("Battery level:\t"+QString::number(SystemPowerStatus.BatteryLifePercent)+ "%");
                    PowerInFo.append("\nRem:\t"+QString::number(SystemPowerStatus.BatteryLifeTime));
                    PowerInFo.append("\nFull:\t"+QString::number(SystemPowerStatus.BatteryFullLifeTime));
                }
            }
            ui->H_hwstatuspushButton->setToolTip(PowerInFo);
        });
    },std::placeholders::_1);

    CheckNB_DT(100);

    std::function<void (QStandardItemModel*,int,int)> DisableEditSel = std::bind([=](QStandardItemModel* Model,int row,int col){
        Model->item(row,col)->setEditable(false);
        Model->item(row,col)->setSelectable(false);
    },std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
    QString SN,MAC;
    SN = H_Model->index(0,1).data().toString();
    MAC = H_Model->index(1,1).data().toString();

    Table_ColHeader <<HWCHK_TABLECOLUMNHEADER;
    Table_RowHeader <<HWCHK_TABLEROWHEADER;
    ItemData<<INITITEMDATA;

    SetTableData(Table_ColHeader,Table_RowHeader,HW_resultModel,ItemData,1);
    for (int row = 0; row < HW_resultModel->rowCount(); ++row) {
           DisableEditSel(HW_resultModel,row,0);
    }
    for (int row = 1; row < HW_resultModel->rowCount(); ++row) {
           DisableEditSel(HW_resultModel,row,1);
    }
    ui->HW_resulttableView->setModel(HW_resultModel);
    ui->HW_resulttableView->resizeColumnsToContents();

    Table_ColHeader.clear();
    Table_RowHeader.clear();
    ItemData.clear();

    Table_ColHeader <<HWYB_TABLECOLUMNHEADER;
    for (int var = 0; var < Table_RowHeader.count(); ++var) {
           ItemData.append("_");
    }
    ItemData<<"-"<<"-"<<"-"<<"-"<<"-"<<"-"<<"-";
    if (H_Model) {
           for (int var_row = 0; var_row < H_Model->rowCount(); ++var_row) {
                     for (int var_col = 0; var_col < H_Model->columnCount(); ++var_col) {
                                HW_DmYB_Model->setItem(var_row,var_col,new QStandardItem(H_Model->index(var_row+2,var_col).data().toString()));//var_row+2    SKIP SN MAC
                     }
           }
    }

    HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2,0,new QStandardItem("Fingerprint"));
    std::unordered_multimap<QString,QString>::const_iterator hwit;
           hwit = H_hwmmp.find("Fingerprint");
           if ( hwit != H_hwmmp.end()){
                        HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2,1,new QStandardItem(hwit->second.split("##").first().simplified()));
                        HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2,2,new QStandardItem(hwit->second.split("##").last().simplified().split("#").first()));
           }else {
                        HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2,1,new QStandardItem("NA"));
                        HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2,2,new QStandardItem("--"));
           }

    HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-1,0,new QStandardItem("Camera"));
   for (int Camera_count = 1; Camera_count < 3; ++Camera_count) {
            hwit = H_hwmmp.find("Camera"+QString::number(Camera_count));
            if ( hwit != H_hwmmp.end()){
                     if (Camera_count>1) {
                                HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-2+Camera_count,0,new QStandardItem("Camera"+QString::number(Camera_count)));
                                HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-3+Camera_count,1,new QStandardItem(hwit->second.split("##").first().simplified()));
                                HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-3+Camera_count,2,new QStandardItem(hwit->second.split("##").last().simplified().split("#").first()));
                     }else{
                                HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-1,1,new QStandardItem(hwit->second.split("##").first().simplified()));
                                HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-1,2,new QStandardItem(hwit->second.split("##").last().simplified().split("#").first()));
                     }
            }else {
                    HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-1,1,new QStandardItem("NA"));
                    HW_DmYB_Model->setItem(HW_DmYB_Model->rowCount()-1,2,new QStandardItem("-"));
            }
   }

    SetTableData(Table_ColHeader,Table_RowHeader,HW_DmYB_Model,ItemData,2);
    ui->HW_CheckYBtableView->setModel(HW_DmYB_Model);
    ui->HW_CheckYBtableView->resizeColumnsToContents();

    Table_ColHeader.clear();
    Table_RowHeader.clear();
    ItemData.clear();
    Table_ColHeader <<HPDIGA_TABLECOLUMNHEADER;
    Table_RowHeader <<HPDIGA_TABLEROWHEADER;
    for (int var = 0; var < Table_RowHeader.count(); ++var) {
            ItemData.append(" ");
    }
    SetTableData(Table_ColHeader,Table_RowHeader,HW_HhpdiagModel,ItemData,1);
    for (int row = 0; row < HW_HhpdiagModel->rowCount(); ++row) {
            DisableEditSel(HW_HhpdiagModel,row,0);
    }
    for (int row = 0; row < HW_HhpdiagModel->rowCount(); ++row) {
            HW_HhpdiagModel->item(row,1)->setData("Check Me InPut PASS/FAIL",Qt::ToolTipRole);
    }
    HW_HhpdiagModel->horizontalHeaderItem(1)->setData("Check Me InPut PASS/FAIL",Qt::ToolTipRole);

    ui->Hp_DiagtableView->setModel(HW_HhpdiagModel);
    ui->Hp_DiagtableView->resizeColumnsToContents();

}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
  case Qt::Key_A:{
      static bool check = true;
           if (check){
                 ui->Main_Tab->tabBar()->show();
                 check = false;
           }else {
                 ui->Main_Tab->tabBar()->hide();
                 check = true;
           }
      }break;
  default:
           break;
  }
  if ((event->key() == Qt::Key_F )&& (event->modifiers() == Qt::ControlModifier))
  {
           static bool hide_status = true;
           if (hide_status) {
              ui->Search_lineEdit->show();
               ui->IDs_DownpushButton->show();
               ui->IDs_UppushButton->show();
               ui->IDs_DownpushButton->setEnabled(false);
               ui->IDs_UppushButton->setEnabled(false);
               hide_status = false;
               ui->Search_lineEdit->setFocus();
           }else {
                     ui->Search_lineEdit->hide();
                     ui->IDs_DownpushButton->hide();
                     ui->IDs_UppushButton->hide();
                     hide_status = true;
           }
  }

  return QWidget::keyPressEvent(event);
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QTabWidget* qtab = findChild<QTabWidget*>();
    if (qtab)
    {
       if(event->pos().rx()<400){
                 FloatWin->show();
                 ui->Main_Tab->tabBar()->show();
           }else if (event->pos().rx()>400) {
                 FloatWin->hide();
                 ui->Main_Tab->tabBar()->hide();
       }

    }
  return QWidget::mouseMoveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->Main_Tab->resize(event->size());
    //FloatWin->move(0,520+geometry().y());
     qDebug()<<FloatWin->geometry();
     qDebug()<<geometry().height()-geometry().y();
     FloatWin->move(0,geometry().height()-geometry().y()-100);
    return QWidget::resizeEvent(event);
}

bool MainWindow::RunAsAdmin()
{
    BOOL Admistatus;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    Admistatus = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(Admistatus)
    {
       if (!CheckTokenMembership( NULL, AdministratorsGroup, &Admistatus))
       {
             Admistatus = false;
       }
       FreeSid(AdministratorsGroup);
    }
    return(Admistatus);
}

#ifdef OUT_DEBUG
ui->textBrowser->append("---------------------------------------------Device Start ------------------------------------------------");
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class
//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Class
//QString regpath = "SYSTEM\\ControlSet001\\Control\\Class";
QString regpath = "SYSTEM\\CurrentControlSet\\Control\\Class";
QStringList regkeystr;
regkeystr<<"ProviderName"<<"DriverVersion"<<"DriverDate";
std::unordered_multimap<QString,QString> Devlistmmp;
applistver.DeviceCheck();
ui->textBrowser->append("\n");
ui->textBrowser->append("---------------------------------------------Software Component Device ------------------------------------------------");
foreach (struct DeviceInfo var, applistver.GetDeviceInfo2DM()) {
    if (var.sezClassBuff == "SoftwareComponent") {
       Devlistmmp =  applistver.Reg2DisPlay(regkeystr,regpath+"\\"+var.Driver,HKEY_LOCAL_MACHINE);
       for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
             if (varmmp->second.isEmpty())
                                continue;
             if(!var.FirendName.isEmpty())
                                ui->textBrowser->append(var.FirendName+"\t"+var.sezDescBuf+"\t"+varmmp->second);
             else  ui->textBrowser->append(var.sezDescBuf+"\t"+varmmp->second);
       }
    }
}
ui->textBrowser->append("---------------------------------------------Software Component Device------------------------------------------------");
ui->textBrowser->append("\n");
foreach (struct DeviceInfo var, applistver.GetDeviceInfo2DM()) {
    if (var.sezDescBuf.isEmpty() && var.FirendName.isEmpty())
       continue;
    if (!var.sezDescBuf.isEmpty()){
       if(!var.HardWareID.isEmpty()){
             Devlistmmp =  applistver.Reg2DisPlay(regkeystr,regpath+"\\"+var.Driver,HKEY_LOCAL_MACHINE);
             for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
                                if (varmmp->second.isEmpty())
                                    continue;
                                if (!var.FirendName.isEmpty())
                                    ui->textBrowser->append(var.FirendName+"\t"+var.sezDescBuf+"\t"+varmmp->second);
                                else ui->textBrowser->append(var.sezDescBuf+"\t"+varmmp->second);
             }
       }else {
             //if(var.Driver.isEmpty()) qDebug()<<"Give Me:\t"<<var.sezDescBuf;
             Devlistmmp =  applistver.Reg2DisPlay(regkeystr,regpath+"\\"+var.Driver,HKEY_LOCAL_MACHINE);
             for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
                                if (varmmp->second.isEmpty())
                                    continue;
                                if (!var.FirendName.isEmpty())
                                    ui->textBrowser->append(var.FirendName+"\t"+var.sezDescBuf+"\t"+varmmp->second);
                                else ui->textBrowser->append(var.sezDescBuf+"\t"+varmmp->second);
             }
       }
    } else if (!var.FirendName.isEmpty()){
       if(!var.HardWareID.isEmpty()){
             Devlistmmp =  applistver.Reg2DisPlay(regkeystr,regpath+"\\"+var.Driver,HKEY_LOCAL_MACHINE);
             for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
                                if (varmmp->second.isEmpty())
                                    continue;
                                if (!var.sezDescBuf.isEmpty())
                                    ui->textBrowser->append(var.FirendName+"\t"+var.sezDescBuf+"\t"+varmmp->second);
                                else ui->textBrowser->append(var.FirendName+"\t"+varmmp->second);
             }
       } else{
             //if(var.Driver.isEmpty())qDebug()<<"Give Me:\t"<<var.FirendName;
             Devlistmmp =  applistver.Reg2DisPlay(regkeystr,regpath+"\\"+var.Driver,HKEY_LOCAL_MACHINE);
             for (auto varmmp = Devlistmmp.begin(); varmmp !=Devlistmmp.end(); ++varmmp) {
                                if (varmmp->second.isEmpty())
                                    continue;
                                if (!var.sezDescBuf.isEmpty())
                                    ui->textBrowser->append(var.FirendName+"\t"+var.sezDescBuf+"\t"+varmmp->second);
                                else ui->textBrowser->append(var.FirendName+"\t"+varmmp->second);
             }
       }
    }else  continue;
}
ui->textBrowser->append("---------------------------------------------Device End--------------------------------------------------");
ui->textBrowser->append("\n");
ui->textBrowser->append("---------------------------------------------Appx Start ------------------------------------------------");
//_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"/*"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"*/);
QString appx_str = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
applistver.List_Appx(appx_str);
applistver.AppList.sort();
applistver.AppList.unique();
if (!applistver.AppList.empty()) {
    foreach (QString var, applistver.AppList) {
       ui->textBrowser->append(var);
    }
}
ui->textBrowser->append("---------------------------------------------Appx X64 ------------------------------------------------");
appx_str = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
applistver.List_Appx(appx_str);
applistver.AppList.sort();
applistver.AppList.unique();
if (!applistver.AppList.empty()) {
    foreach (QString var, applistver.AppList) {
       ui->textBrowser->append(var);
    }
}
ui->textBrowser->append("---------------------------------------------Appx End--------------------------------------------------");
ui->textBrowser->append("\n");
ui->textBrowser->append("---------------------------------------------Disk Driver Start ------------------------------------------------");
QStringList HDDdisplay;
QString HDD_Str = "SYSTEM\\ControlSet001\\Enum\\SCSI";
HDDdisplay<< "FriendlyName"<<"HardwareID";
std::unordered_multimap<QString,QString> hddlistmmp;
foreach (QString var_next, applistver.ReturnNextKeyname(HDD_Str,HKEY_LOCAL_MACHINE)) {
    foreach (QString var_model, applistver.ReturnNextKeyname(HDD_Str+"\\"+var_next,HKEY_LOCAL_MACHINE)) {
       hddlistmmp =  applistver.Reg2DisPlay(HDDdisplay,(HDD_Str+"\\"+var_next+"\\"+var_model),HKEY_LOCAL_MACHINE);
       for (auto var = hddlistmmp.begin(); var !=hddlistmmp.end(); ++var) {
             ui->textBrowser->append(var->second);
       }
    }
}
ui->textBrowser->append("---------------------------------------------Disk Driver End--------------------------------------------------");
ui->textBrowser->append("\n");
ui->textBrowser->append("---------------------------------------------Programs App list Version Start ------------------------------------------------");
QStringList AppListDispy;
QString CompleteStr = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
AppListDispy<< "DisplayName"<<"DisplayVersion";
std::unordered_multimap<QString,QString> applistmmp;
foreach (QString var_next, applistver.ReturnNextKeyname(CompleteStr,HKEY_LOCAL_MACHINE)) {
    applistmmp =  applistver.Reg2DisPlay(AppListDispy,CompleteStr+"\\"+var_next,HKEY_LOCAL_MACHINE);
    for (auto var = applistmmp.begin(); var !=applistmmp.end(); ++var) {
       if (var->second.isEmpty())
             continue;
       ui->textBrowser->append(var->second);
    }
}
ui->textBrowser->append("---------------------------------------------X64 ------------------------------------------------");
CompleteStr =  "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
foreach (QString var_next, applistver.ReturnNextKeyname(CompleteStr,HKEY_LOCAL_MACHINE)) {
    applistmmp =  applistver.Reg2DisPlay(AppListDispy,CompleteStr+"\\"+var_next,HKEY_LOCAL_MACHINE);
    for (auto var = applistmmp.begin(); var !=applistmmp.end(); ++var) {
       if (var->second.isEmpty())
             continue;
       ui->textBrowser->append(var->second);
    }
}
ui->textBrowser->append("---------------------------------------------Programs App list Version End ------------------------------------------------");
ui->textBrowser->append("\n");
ui->textBrowser->append("---------------------------------------------WMIC Server Call Functions Start------------------------------------------------");
/*
    *https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/computer-system-hardware-classes
    *
    * WmicListStr("Win32_Process","Name");
    * WmicListStr("Win32_PhysicalMemory","PartNumber");
    */
applistver.CleanData(); //Clean the Store data
QStringList Address;
QVector<QString>PhyMac;
Address<<"Description"<<"AdapterType"<<"MACAddress"<< "PhysicalAdapter";
foreach (QString wmistr, applistver.WmicListStr("Win32_NetworkAdapter",Address)) {
    PhyMac.push_back(wmistr);
    if (wmistr.contains("true"))
    {
       foreach (QString var, PhyMac) {
             ui->textBrowser->append(var);
       }
       PhyMac.clear();
    }else if (wmistr.contains("false")) {
       PhyMac.clear();
    }
}
applistver.CleanData();
QStringList DDR_Info;
DDR_Info<<"Manufacturer"<<"PartNumber"<<"DeviceLocator"<<"Capacity"<<"Speed";
foreach (QString wmistr, applistver.WmicListStr("Win32_PhysicalMemory",DDR_Info)) {
    ui->textBrowser->append(wmistr);
}
ui->textBrowser->append("---------------------------------------------WMIC Server Call Functions End------------------------------------------------");
#endif


