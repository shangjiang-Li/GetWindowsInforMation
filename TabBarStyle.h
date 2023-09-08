
#ifndef TABBARSTYLE_H
#define TABBARSTYLE_H


#include "qtabbar.h"
#include <QObject>
#include <QProxyStyle>

class TabBarStyle : public QProxyStyle
{
    Q_OBJECT

public:
    TabBarStyle(Qt::Orientation orientation = Qt::Vertical);
    ~TabBarStyle();

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = nullptr) const override;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;
/*
    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;
    QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos, const QWidget *widget = nullptr) const override;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;
    QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const override;
    int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    void polish(QWidget *widget) override;
    void polish(QPalette &pal) override;
    void polish(QApplication *app) override;
    QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    QPalette standardPalette() const override;
    QPixmap standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = nullptr) const override;
    int styleHint(QStyle::StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;
    QRect subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *option, QStyle::SubControl sc, const QWidget *widget) const override;
    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    void unpolish(QWidget *widget) override;
    void unpolish(QApplication *app) override;
*/
private:
    Qt::Orientation m_orientation;
};

class TabBarPlus:public QTabBar
{
public:
    TabBarPlus();
    QSize tabSizeHint(int index) const override;
protected:
    void paintEvent(QPaintEvent *pevent) override;
};

#endif // TABBARSTYLE_H
