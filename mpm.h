#ifndef MPM_H
#define MPM_H

#include <QObject>
#include "QDebug"
class MPM : public QObject
{
    Q_OBJECT
public:
    explicit MPM(QObject *parent = nullptr);
    ~MPM();
    void mpmwork(QString filename,QString bcu_path);
signals:
    void MpmMe_status(bool status,QString err_info);
};

#endif // MPM_H
