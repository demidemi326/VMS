#include <QApplication>
#include <QtWidgets>

#include "frengine.h"
#include "servicebase.h"
#include "activationdlg.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    int ret = SetActivation(setting.value("activation key").toString().toUtf8().data());
    if(ret != 0)
    {
        ActivationDlg dlg;
        ret = dlg.exec();
        if(ret == 0)
        {
            return 0;
        }
        else
        {
            WCHAR binPath[MAX_PATH];
            GetModuleFileName(NULL, binPath, MAX_PATH);

            QString binPathStr = QString::fromUtf16((ushort*)binPath);
            QFileInfo fileInfo(binPathStr);

            QString program = fileInfo.absoluteDir().absolutePath() + "/sudo.cmd";
            QStringList argments;
            argments << "VMSService_video.exe" << "-install";
            QProcess process;
            process.start(program, argments);
            process.waitForFinished();
            QThread::msleep(3000);

            argments.clear();
            argments << "sc" << "start" << QString::fromUtf16((ushort*)SERVICE_NAME);
            process.start(program, argments);
            process.waitForFinished();
        }
    }
    return 0;
}
