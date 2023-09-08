
#include "TabBarStyle.h"
#include <QPainter>
#include <QStyleOptionTab>
#include <QStyleOption>
#include <QDebug>
#include <QCommonStyle>

TabBarStyle::TabBarStyle(Qt::Orientation orientation/* = Qt::Vertical*/)
    : QProxyStyle()
{
    m_orientation = orientation;
}

TabBarStyle::~TabBarStyle()
{
}

void TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget /*= nullptr*/) const
{
    QProxyStyle::drawControl(element, option, painter, widget);


    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect controlRect = tab->rect;
            QString tabText;
            QTextOption option;
            option.setAlignment(Qt::AlignCenter);
            tabText = tab->text;
            painter->drawText(controlRect, tabText, option);
            return;
        }
    }
    if (element == CE_TabBarTab) {
        QProxyStyle::drawControl(element,option,painter,widget);
    }
}

void TabBarStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole /*= QPalette::NoRole*/) const
{
    //QProxyStyle::drawItemText(painter, rect, flags, pal,enabled,text,textRole);
}

QSize TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget /*= nullptr*/) const
{
    QSize size = QProxyStyle::sizeFromContents(type,option,contentsSize,widget);
    if (type == CT_TabBarTab)
    {
        size.transpose();
        size.rwidth()=200;
        size.rheight()=100;
    }
    return size;
}
void TabBarStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    alignment = Qt::AlignVCenter;
    qDebug()<<pixmap.height();
    QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}
QRect TabBarStyle::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    qDebug()<<r.toRectF();
    return QProxyStyle::itemPixmapRect(r,flags, pixmap);
}
QSize TabBarPlus::tabSizeHint(int index) const
{
    QSize s = QTabBar::tabSizeHint(index);
    s.transpose();
    return s;
}

void TabBarPlus::paintEvent(QPaintEvent * pevent)
{
        qDebug()<<pevent;
//    QStylePrivate Painter(this);
//    QStyleOptionTab opt;
//    for(int i = 0;i < count();i++)
//    {
//        initStyleOption(&opt,i);
//        Painter.drawControl(QStyle::CE_TabBarTabShape, opt);
//        Painter.save();

//        QSize s = opt.rect.size();
//        s.transpose();
//        QRect r(QPoint(), s);
//        r.moveCenter(opt.rect.center());
//        opt.rect = r;

//        QPoint c = tabRect(i).center();
//        Painter.translate(c);
//        Painter.rotate(90);
//        Painter.translate(-c);
//        Painter.drawControl(QStyle::CE_TabBarTabLabel,opt);
//        Painter.restore();

//    }

}
