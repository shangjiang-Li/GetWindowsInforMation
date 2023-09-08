#include "delegate_excel.h"

delegate_excel::delegate_excel(QObject *parent) : QItemDelegate(parent)
{

}
QWidget * delegate_excel::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QStringList combox_list;
   QComboBox *box=new QComboBox(parent);
   combox_list<<"PASS"<<"FAIL";
   box->addItems(combox_list);
   return box;
   return QItemDelegate::createEditor(parent, option, index);
}
delegate_excel::~delegate_excel()
{

}
