#ifndef TABBARPLUS_H
#define TABBARPLUS_H

#include "qtabbar.h"
#include <QObject>

class TabBarPlus : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBarPlus(QObject *parent = nullptr);
     void paintEvent(QPaintEvent *) override;
signals:

};

#endif // TABBARPLUS_H
