
#ifndef SOIC_EXCEL_H
#define SOIC_EXCEL_H
#include <QTabWidget>
#include <QTableView>
#include "threadcrl.h"
#include "xlsxdocument.h"
#include "infomachine.h"
//#include "xlsxchartsheet.h"
//#include "xlsxcellrange.h"
//#include "xlsxchart.h"
//#include "xlsxrichstring.h"
//#include "xlsxworkbook.h"
//#include "xlsxconditionalformatting.h"
using namespace QXlsx;

#include <QObject>

class SOIC_Excel : public QObject
{
    Q_OBJECT
public:
    explicit SOIC_Excel(QObject *parent = nullptr);
    virtual ~SOIC_Excel();
    bool SOIC_ExcelOpen(QString filename);
    Document* GetExcelDocument(){return SOIC_excel;}
    bool SOIC_SheetOpen(QTableView*  DisPlayMainWin, InFoMachine &infomac, const QString& Sheetname=nullptr);
    bool SOIC_SheetOpen(QTableView*  DisPlayMainWin, InFoMachine &infomac, QTabWidget* ResultExcelBooks);
    void SOIC_SheetShow(QTableView*  DisPlayMainWin, const QString& Sheetname=nullptr);
signals:
private:
        Document*       SOIC_excel;
        Worksheet*      WorkSheet;
        QTableView*    Result_View;
        Worksheet*      WorkSheetResult;
        QTableView*    ExcelResult_tableview;
        QModelIndex    index,indexResult;
        std::multimap<QString,QString>ExcelPNAndVersion;
        Threadcrl*  SoicMainTh;
};
#endif // SOIC_EXCEL_H
