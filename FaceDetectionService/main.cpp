#include "mainwindow.h"
#include "activationdlg.h"
#include "frengine.h"
#include "base.h"
#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QObject>
#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setOrganizationName(ORG_NAME);

    QSharedMemory sharedMemory;
    sharedMemory.setKey("FaceDetectionService_video-UniqueId");
    if(sharedMemory.attach()) {
        QMessageBox::warning(0, QApplication::translate("MainWindow", "Warning", 0),
                             QApplication::translate("MainWindow", "Now running already this program.", 0));
        return 0;
    }

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(0, QApplication::translate("MainWindow", "Warning", 0),
                             QApplication::translate("MainWindow", "Now running already this program.", 0));
        return 0; // Exit already a process running
    }

#ifdef _ACTIVATION_
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
    }
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
