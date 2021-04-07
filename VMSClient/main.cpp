#include "mainwindow.h"
#include "logindlg.h"
#include "clientbase.h"

#include <QApplication>

QTranslator g_translator;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("VMS Client - Video");
    a.setOrganizationName("Easen");

    QFont font = qApp->font();
    font.setFamily(QString::fromUtf8("SimSun"));
    qApp->setFont(font);

    g_translator.load("vmsclient_ch");
    qApp->installTranslator(&g_translator);

    QString strPass = readPass();
    if(!strPass.isEmpty())
    {
        LoginDlg dlg;
        int nRet = dlg.exec();

        if(nRet == 0)
            return 0;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
