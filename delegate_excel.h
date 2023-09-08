#ifndef DELEGATE_EXCEL_H
#define DELEGATE_EXCEL_H

#include <QObject>
#include<QItemDelegate>
#include <QSpinBox>
#include <QComboBox>
#include <QEvent>

class delegate_excel : public QItemDelegate
{
    Q_OBJECT
public:
    explicit delegate_excel(QObject *parent = nullptr);
    ~delegate_excel();
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
signals:

};

#endif // DELEGATE_EXCEL_H
