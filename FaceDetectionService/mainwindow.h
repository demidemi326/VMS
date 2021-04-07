#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "servicebase.h"
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class CameraProcessEngine;
class ClientListeningSocket;
class FaceSendingSocket;
class AuthenticatingSocket;
class FaceImageSendingSocket;
class ServerInfoSocket;
class SurveillanceService;
class QSystemTrayIcon;
class QMenu;
class ProcessClustering;
class Cluster;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void    slotStartService();
    void    slotStop();
    void    slotLogOut(QString logStr);

    void    slotClustering();
    void    slotEval();
    void    slotProbe();
    void    slotGallery();

private:
    void    setupActions();

protected:

private:
    Ui::MainWindow *ui;

    SurveillanceService*            m_surveillanceService;
    ProcessClustering*              m_clustering;

    Cluster* m_gallery;
    Cluster* m_probe;
};

#endif // MAINWINDOW_H
