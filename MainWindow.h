#pragma once
#include "ServiceMaintainer.h"
#include "Service.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QProgressBar>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <QList>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    QVBoxLayout *layout;
    QHBoxLayout *linkLayout;
    QLineEdit *linkBox;
    QPushButton *downloadButton;
    QPushButton *getEngineButton;
    QLabel *statusLabel; // General: download and service
    ServiceMaintainer *maintainer;
    QString chosenDirectory;
    QMenu *optionsMenu;
    QMenu *buildMenu;
    QProgressBar *progressBar;

    QAction *chooseLocationAction;
    QAction *chooseNightlyAction;
    QAction *chooseStableAction;
    QActionGroup *versionGroup;

    QString downloadLocation;

    Service *service;
    
private slots:
    void getServiceSlot();
    void setDownloadReadiness(); // Updates downloadButton status
    void engineDownloading();
    void engineDownloaded(int exit);
    void changeLocation();

    void startDownload();
    void downloadStarted();
    void downloadFinished(int exit);
    void downloadProcessFailed(QString error);
    void downloadProgress(int percentage);
};