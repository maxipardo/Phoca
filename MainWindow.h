#pragma once
#include "ServiceMaintainer.h"
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
#include <QCheckBox>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <QList>
#include <QTimer>
#include <QDateTime>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    QVBoxLayout *fullLayout;
    QVBoxLayout *layout;
    QHBoxLayout *linkLayout;
    QHBoxLayout *optionsLayout;
    QLineEdit *linkBox;
    QPushButton *downloadButton;
    QPushButton *clearFinishedButton;
    QPushButton *getEngineButton;
    ServiceMaintainer *maintainer;
    QMenu *optionsMenu;
    QMenu *buildMenu;
    QMenu *advancedMenu;
    QAction *aboutAction;

    QListWidget *list;
    
    QRadioButton *bothButton;
    QRadioButton *videoButton;
    QRadioButton *audioButton;

    QComboBox *qualityBox;
    QComboBox *conversionBox;
    QCheckBox *subtitlesBox;

    QAction *chooseLocationAction;
    QAction *chooseNightlyAction;
    QAction *chooseStableAction;
    QActionGroup *versionGroup;
    QAction *savePlaylistInFolderAction;
    QAction *saveThumbnailAction;
    QActionGroup *advancedGroup;
    QAction *forceIPv4Action;
    QAction *thumbnailVisibilityAction;

    QString downloadLocation;
    bool savePlaylistInFolder;
    bool saveThumbnail;
    bool firstLaunch;
    bool nightlyService;
    bool forceIPv4;
    bool thumbnailVisibility;
    QDateTime lastEngineUpdate;

    QLabel *locationLabel;
    
    int lastLength = 0;
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
    void updateLocationLabel();
    void changeNightlyService();
    void changeForceIPv4();

    void startDownload();
    void clearFinishedDownloads();
    void itemFinished();

    void aboutPage();
    void changeThumbnailVisibility();
};