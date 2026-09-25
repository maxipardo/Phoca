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
    bool activeDownloads();
private:
    void setupConnections();
    void setupUI();

    QWidget *m_centralWidget;
    QVBoxLayout *m_fullLayout;
    QVBoxLayout *m_layout;
    QHBoxLayout *m_linkLayout;
    QHBoxLayout *m_optionsLayout;
    QHBoxLayout *m_bottomLayout;
    QLineEdit *m_linkBox;
    QPushButton *m_downloadButton;
    QPushButton *m_clearFinishedButton;
    QPushButton *m_getEngineButton;
    ServiceMaintainer *m_maintainer;
    QMenu *m_optionsMenu;
    QMenu *m_buildMenu;
    QMenu *m_advancedMenu;
    QMenu *m_cookiesMenu;
    QAction *m_aboutAction;
    QAction *m_deleteAction;
    QSpacerItem *m_spacer;

    QListWidget *m_list;
    
    QRadioButton *m_bothButton;
    QRadioButton *m_videoButton;
    QRadioButton *m_audioButton;

    QComboBox *m_qualityBox;
    QComboBox *m_conversionBox;
    QCheckBox *m_subtitlesBox;

    QAction *m_chooseLocationAction;
    QAction *m_chooseNightlyAction;
    QAction *m_chooseStableAction;
    QActionGroup *m_versionGroup;
    QAction *m_savePlaylistInFolderAction;
    QAction *m_saveThumbnailAction;
    QAction *m_forceIPv4Action;
    QAction *m_cookiesFileAction;
    QAction *m_cookiesAction;
    QActionGroup *m_cookiesGroup;

    QAction *m_thumbnailVisibilityAction;

    QString m_downloadLocation;
    QString m_cookiesFile;
    bool m_savePlaylistInFolder;
    bool m_saveThumbnail;
    bool m_firstLaunch;
    bool m_nightlyService;
    bool m_forceIPv4;
    bool m_thumbnailVisibility;
    QString m_cookies;
    QDateTime m_lastEngineUpdate;

    QLabel *m_locationLabel;
    
    int m_lastLength = 0;
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
    void chooseCookiesFile();

    void startDownload();
    void clearFinishedDownloads();
    void itemFinished();

    void aboutPage();
    void changeThumbnailVisibility();
    void changeCookies(const QString &browser);
    void setGetEngineButton();
};