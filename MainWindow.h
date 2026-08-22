#pragma once
#include "ServiceMaintainer.h"
#include "Service.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QProgressBar>
#include <QStatusBar>
#include <QSettings>
#include <QComboBox>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>

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
    QHBoxLayout *fullLayout;
    QVBoxLayout *layout;
    QHBoxLayout *linkLayout;
    QHBoxLayout *optionsLayout;
    QLineEdit *linkBox;
    QPushButton *downloadButton;
    QPushButton *stopDownloadButton;
    QPushButton *getEngineButton;
    QLabel *statusLabel; // General: download and service
    QLabel *titleLabel;
    ServiceMaintainer *maintainer;
    QString chosenDirectory;
    QMenu *optionsMenu;
    QMenu *buildMenu;
    QAction *aboutAction;
    QProgressBar *progressBar;

    QListWidget *list;
    
    QRadioButton *bothButton;
    QRadioButton *videoButton;
    QRadioButton *audioButton;

    QComboBox *qualityBox;
    QComboBox *conversionBox;

    QAction *chooseLocationAction;
    QAction *chooseNightlyAction;
    QAction *chooseStableAction;
    QActionGroup *versionGroup;
    QAction *savePlaylistInFolderAction;
    QAction *saveThumbnailAction;

    QString downloadLocation;
    bool savePlaylistInFolder;
    bool saveThumbnail;

    Service *service;
    QString downloadPhase;
protected:
    void closeEvent(QCloseEvent *event) override;
    
private slots:
    void getServiceSlot();
    void setDownloadReadiness(); // Updates downloadButton status
    void toggleQualityOptions();
    void engineDownloading();
    void engineDownloaded(int exit);
    void changeLocation();
    void changeSavePlaylistInFolder();
    void changeSaveThumbnail();

    void startDownload();
    void stopDownload();

    void aboutPage();
};