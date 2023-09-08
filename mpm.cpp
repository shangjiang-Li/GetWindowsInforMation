#include "mpm.h"
#include "QFile"
#include "QDir"
#include "QFileInfo"
#include "QByteArray"
#include "QProcess"

MPM::MPM(QObject *parent)
    : QObject{parent}
{
}
MPM::~MPM()
{
}
void MPM::mpmwork(QString filename,QString bcu_path){

    QString Metoolpath("METools/MEInfo/WINDOWS64");
    QString Metoolinfoname("MEINFO.LOG");
    QString Metoolcmd("/MEInfoWin64.exe   -feat  \"NVAR Configuration State\" >"+Metoolinfoname);
    QString Mecheckstr("NVAR Configuration State: Locked");
    QString Mpmcheckstr("Manufacturing Programming Mode");
    QString TXTcheckstr("Trusted Execution Technology (TXT)");
    QFileInfo bcu(bcu_path);
    QDir bcudir(bcu.absolutePath()),tmpdir;
    bcudir.cdUp();
    tmpdir=bcudir;
    bcudir.cd(Metoolpath);
    QFile mpmfile(filename),Mefile(Metoolinfoname);;
    QByteArray meinfo,ReadLineInfo;
    bool ME_status=false;
    if(!system(bcudir.absolutePath().toUtf8()+Metoolcmd.toUtf8())){
        if(Mefile.open(QFile::ReadOnly)){
             meinfo=Mefile.readAll();
            if(meinfo.contains(Mecheckstr.toUtf8()) && meinfo!=nullptr ){
                ME_status=true;
            }
       }
        Mefile.close();
     }
    if (!mpmfile.exists())
       emit MpmMe_status(ME_status,"Can not open or have no permission: "+filename);
    if(mpmfile.open(QFile::ReadOnly)){
       QString T_txtstr,M_mpmstr,temp_txt,temp_mpm;
        while ( !(ReadLineInfo = mpmfile.readLine()).isEmpty() ) {
            if (ReadLineInfo.contains(TXTcheckstr.toUtf8())) {
                     temp_txt = mpmfile.readLine();
                     if(temp_txt.contains("*Disable"))
                          T_txtstr = "*Disable_txt";
                     else if (temp_txt.contains("*Enable"))
                          T_txtstr = "*Enable_txt";
                      temp_txt = mpmfile.readLine();
                     if(temp_txt.contains("*Disable"))
                          T_txtstr = "*Disable_txt";
                     else if (temp_txt.contains("*Enable"))
                          T_txtstr = "*Enable_txt";
            }
            if (ReadLineInfo.contains(Mpmcheckstr.toUtf8())) {
                      temp_mpm = mpmfile.readLine();
                      if(temp_mpm.contains("*Lock"))
                               M_mpmstr = "*Lock_mpm";
                      else if (temp_mpm.contains("*Unlock"))
                               M_mpmstr = "*Unlock_mpm";
                      temp_mpm = mpmfile.readLine();
                      if(temp_mpm.contains("*Lock"))
                               M_mpmstr = "*Lock_mpm";
                      else if (temp_mpm.contains("*Unlock"))
                               M_mpmstr = "*Unlock_mpm";
            }
        }
       emit MpmMe_status(ME_status,M_mpmstr+"\t"+T_txtstr);
    }else {
              emit MpmMe_status(ME_status,"Can not open or have no permission: "+filename);
        }
            mpmfile.close();
        if (mpmfile.exists())
              mpmfile.moveToTrash();
        if (Mefile.exists())
              Mefile.moveToTrash();
}

