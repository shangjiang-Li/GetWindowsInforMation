
#include "soic_excel.h"
#include "qforeach.h"
#include "qheaderview.h"
#include "xlsxsheetmodel.h"


SOIC_Excel::SOIC_Excel(QObject *parent)
    : QObject{parent},Result_View(new QTableView),SoicMainTh(new Threadcrl)
{

}
SOIC_Excel::~SOIC_Excel()
{
    delete SOIC_excel;
    delete SoicMainTh;
    delete Result_View;
}
bool SOIC_Excel::SOIC_ExcelOpen(QString filename)
{
    SOIC_excel = new Document(filename);
    return SOIC_excel->load();
}

bool SOIC_Excel::SOIC_SheetOpen(QTableView * DisPlayMainWin,InFoMachine& infomac, const QString& Sheetname)
{
    int Col_PartNo = 0;
    int Col_Ver = 0;
    int Col_Result = 0;
    if (Sheetname.isEmpty())
        WorkSheet = dynamic_cast<Worksheet *>(SOIC_excel->currentSheet());
    else{
        SOIC_excel->selectSheet(Sheetname);
        WorkSheet = dynamic_cast<Worksheet *>(SOIC_excel->sheet(Sheetname));
    }
    qDebug()<<SOIC_excel->currentSheet()->sheetName();
    DisPlayMainWin->setModel(new SheetModel(WorkSheet, DisPlayMainWin));
    foreach (CellRange range, WorkSheet->mergedCells())
           DisPlayMainWin->setSpan(range.firstRow()-1, range.firstColumn()-1, range.rowCount(), range.columnCount());
    for (int var = 0; var < DisPlayMainWin->model()->columnCount(); ++var) {
        index = DisPlayMainWin->model()->index(0,var);
        if (index.data().toString()=="PartNo")
               Col_PartNo = var;

        if (index.data().toString()=="Version")
               Col_Ver = var;

        if (var==DisPlayMainWin->model()->columnCount()-1)
               Col_Result = var;
    }

    foreach (auto info_var,  infomac.GetPnComMap()) {
        for (int var = 1; var < DisPlayMainWin->model()->rowCount(); ++var) {
               index = DisPlayMainWin->model()->index(var,Col_PartNo);
               QString PnStr = index.data().toString();
               index = DisPlayMainWin->model()->index(var,Col_Ver);
               ExcelPNAndVersion.emplace(PnStr,index.data().toString());
               QString VerStr = index.data().toString();

               indexResult = DisPlayMainWin->model()->index(var,Col_Result);
               if ( (info_var.second.PnName==PnStr) && (info_var.second.Version == VerStr) ) {                                          /*info_var.second.LocalRight*///    this is DM check is Machine
                       QXlsx::Format pass_fmat;
                       pass_fmat.setFontColor(Qt::green);
                       DisPlayMainWin->model()->setData(indexResult,"PASS");
                       SOIC_excel->write(*new CellReference(DisPlayMainWin->verticalHeader()->logicalIndex(indexResult.row())+1,DisPlayMainWin->horizontalHeader()->logicalIndex(indexResult.column())+1),"PASS",pass_fmat);
               }else  if (DisPlayMainWin->model()->data(indexResult) !="PASS"){
                            QXlsx::Format fail_fmat;
                            fail_fmat.setFontColor(Qt::red);
                            DisPlayMainWin->model()->setData(indexResult,"FAIL");
                            SOIC_excel->write(*new CellReference(DisPlayMainWin->verticalHeader()->logicalIndex(indexResult.row())+1,DisPlayMainWin->horizontalHeader()->logicalIndex(indexResult.column())+1),"FAIL",fail_fmat);
                }
        }
    }
        DisPlayMainWin->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(221,255,255);color:black;}");
        DisPlayMainWin->verticalHeader()->setStyleSheet("QHeaderView::section{background:rgb(255,221,255);color:black;}");
        DisPlayMainWin->horizontalHeader()->setSortIndicatorShown(true);                                     //sort by header
        DisPlayMainWin->horizontalHeader()->setSortIndicator(0,Qt::SortOrder::AscendingOrder);   //sort by header
        SOIC_excel->save();
    return true;
}
bool SOIC_Excel::SOIC_SheetOpen(QTableView*  DisPlayMainWin, InFoMachine &infomac, QTabWidget* ResultExcelBooks)
{
        SOIC_SheetShow(DisPlayMainWin);
        QChar RandChar('a'+rand()%26);
        QString NewTabname = "Result"+QString::number(rand())+QString(RandChar);
        if(!std::get<QString>(infomac.Get_SetSerialNumber()).isEmpty())
                NewTabname = std::get<QString>(infomac.Get_SetSerialNumber()).simplified()+"_"+QString::number(rand())+QString(RandChar);
        SOIC_excel->copySheet(SOIC_excel->currentSheet()->sheetName(),NewTabname);
        SOIC_SheetOpen(Result_View,infomac,NewTabname);
        ResultExcelBooks->addTab(Result_View,NewTabname);
        return true;
 }
void SOIC_Excel::SOIC_SheetShow(QTableView*  DisPlayMainWin, const QString& Sheetname)
 {
        if (Sheetname.isEmpty())
                WorkSheet = dynamic_cast<Worksheet *>(SOIC_excel->currentSheet());
        else {
                SOIC_excel->selectSheet(Sheetname);
                WorkSheet = dynamic_cast<Worksheet *>(SOIC_excel->sheet(Sheetname));
        }
        DisPlayMainWin->setModel(new SheetModel(WorkSheet, DisPlayMainWin));
        foreach (CellRange range, WorkSheet->mergedCells())
        DisPlayMainWin->setSpan(range.firstRow()-1, range.firstColumn()-1, range.rowCount(), range.columnCount());
        DisPlayMainWin->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(216,255,247);color:black;}");
        DisPlayMainWin->verticalHeader()->setStyleSheet("QHeaderView::section{background:rgb(239,255,218);color:black;}");
        DisPlayMainWin->horizontalHeader()->setSortIndicatorShown(true);                                     //sort by header
        DisPlayMainWin->horizontalHeader()->setSortIndicator(0,Qt::SortOrder::AscendingOrder);   //sort by header
}
