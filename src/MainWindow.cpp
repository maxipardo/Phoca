#include "MainWindow.h"
#include "DownloadItem.h"
#include "About.h"
#include "DownloadConfig.h"
#include "ServiceMaintainer.h"
#include <qaction.h>
#include <QSignalBlocker>
#include <QList>
#include <QUrlQuery>
#include <QApplication>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QCoreApplication::setOrganizationName("MaximoPardo");
    QCoreApplication::setOrganizationDomain("io.github.maxipardo");
    QApplication::setApplicationName("Phoca");

    QSettings settings;

    /* Persistent settings */
    m_downloadLocation = settings.value("downloadLocation", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    m_savePlaylistInFolder = settings.value("savePlaylistInFolder", true).toBool();
    m_saveThumbnail = settings.value("saveThumbnail", false).toBool();
    m_firstLaunch = settings.value("firstLaunch", true).toBool();
    m_nightlyService = settings.value("nightlyService", true).toBool();
    m_forceIPv4 = settings.value("IPv4", true).toBool();
    m_thumbnailVisibility = settings.value("thumbnailVisibility", true).toBool();
    m_cookies = settings.value("cookies", "").toString();

    QString dateString = settings.value("lastEngineUpdate", QDateTime::currentDateTime().toString(Qt::ISODate)).toString();
    m_lastEngineUpdate = QDateTime::fromString(dateString, Qt::ISODate);
    
    if (!m_lastEngineUpdate.isValid()) {
        m_lastEngineUpdate = QDateTime::currentDateTime();
    }

    QDateTime now = QDateTime::currentDateTime();
    bool needsUpdate = m_lastEngineUpdate.daysTo(now) >= 3;

#ifndef FLATPAK_BUILD
    if (m_firstLaunch || needsUpdate) {
        QTimer::singleShot(500, [this]() {
            getServiceSlot();
        });

        if (m_firstLaunch) {
            settings.setValue("firstLaunch", false);
            settings.setValue("nightlyService", m_nightlyService);
        }
    }
#endif

    setupUI();
    setupConnections();

    this->setWindowTitle("Phoca");
    setCentralWidget(m_centralWidget);
    this->resize(200, 200);
    this->setMinimumWidth(462);
    
    this->statusBar()->addWidget(m_locationLabel);
    updateLocationLabel();
}
    
void MainWindow::updateLocationLabel() {
    const QString shown = QDir::toNativeSeparators(m_downloadLocation);
    m_locationLabel->setText(tr("Download location: %1").arg(shown));
    m_locationLabel->setToolTip(shown);
}
    
void MainWindow::engineDownloading() { this->statusBar()->showMessage(tr("[yt-dlp] Downloading...")); }

void MainWindow::engineDownloaded(int exit) {
    switch (exit) {
        case 0:
        this->statusBar()->showMessage(tr("[yt-dlp] Downloaded successfully"), 5000);
        break;
        case 1:
        this->statusBar()->showMessage(tr("[yt-dlp] Download failed: Network error"), 5000);
        break;
        case 2:
        
        #ifdef Q_OS_WIN
        this->statusBar()->showMessage(tr("[yt-dlp] Download failed: Engine file in use, or need permissions to write"), 5000);
        #else
        this->statusBar()->showMessage(tr("[yt-dlp] Download failed: Need permissions to write"), 5000);
        #endif
        break;
    }
    setDownloadReadiness();
}
    
void MainWindow::setDownloadReadiness() {
    // search quality in qualityBox
    bool validQuality = m_qualityBox->findText(m_qualityBox->currentText()) != -1;
    bool validConversion = m_conversionBox->findText(m_conversionBox->currentText()) != -1;
    
    if (!m_linkBox->text().isEmpty() && m_maintainer->exists() && validQuality && validConversion) {
        m_downloadButton->setEnabled(true);
    } else {
        m_downloadButton->setEnabled(false);
    }
}
    
void MainWindow::getServiceSlot() {
#ifdef FLATPAK_BUILD
    return; // OTA download disabled in Flatpak builds
#else
    m_downloadButton->setEnabled(false);
    m_maintainer->getService(m_nightlyService);
    QSettings settings;
    settings.setValue("lastEngineUpdate", QDateTime::currentDateTime().toString(Qt::ISODate));
#endif
}
    
    void MainWindow::changeLocation() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Choose where to save files"), m_downloadLocation,
            QFileDialog::ShowDirsOnly);
            
            if (!dir.isEmpty()) {
                m_downloadLocation = dir;
                
                QSettings settings;
                settings.setValue("downloadLocation", m_downloadLocation);
                
                qDebug() << "Settings saved. Chosen folder:" << m_downloadLocation;
                updateLocationLabel();
            }
        }
        
void MainWindow::startDownload() {
    if (!m_maintainer->exists()){
        getServiceSlot();
        return;
    }
    
    int format {0}; // both
    if (m_videoButton->isChecked()) {
        format = 1;
    } else if (m_audioButton->isChecked()) {
        format = 2;
    }
    QString link = m_linkBox->text();
    bool playlist = false;
    bool looksLikePlaylist = link.contains("list=") || 
    link.contains("/playlist/") || 
    link.contains("/album/") || 
    link.contains("/sets/") || 
    link.contains("/showcase/") || 
    link.contains("/series/") || 
    link.contains("/collection/") || 
    link.contains("/show/") || 
    link.contains("&set=");
    
    // A playlist-only link has no individual video
    bool isPlaylistOnly = false;
    if (looksLikePlaylist) {
        QUrl parsedUrl(link);
        QUrlQuery query(parsedUrl);
        bool hasVideo = query.hasQueryItem("v") || parsedUrl.path().contains("watch");
        isPlaylistOnly = !hasVideo;
    }
    
    if (looksLikePlaylist) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Playlist detected"));
        if (m_saveThumbnail) {
            msgBox.setText(tr("This link contains a playlist.\nDownload the whole list?\nThumbnails will not be saved"));
        } else {
            msgBox.setText(tr("This link contains a playlist.\nDownload the whole list?"));
        }
        
        if (isPlaylistOnly) {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        } else {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.button(QMessageBox::Cancel)->hide();
        }
        
        int answer = msgBox.exec();
        
        if (answer == QMessageBox::Yes) {
            playlist = true;
        } else if (answer == QMessageBox::No && !isPlaylistOnly) {
            playlist = false;
        } else {
            return; 
        }
    }
    
    bool parSaveThumbnail{m_saveThumbnail && !playlist};
    
    QString quality;
    QString conversion;
    if (m_qualityBox->currentIndex() == 0) {
        quality = "0";
    } else {
        quality = m_qualityBox->currentText();
    }
    if (m_conversionBox->currentIndex() == 0) {
        conversion = "0";
    } else {
        conversion = m_conversionBox->currentText();
    }
    
    // Config struct
    DownloadConfig config;
    config.link = link;
    config.downloadLocation = m_downloadLocation;
    config.format = format;
    config.quality = quality;
    config.conversion = conversion;
    config.playlist = playlist;
    config.savePlaylistInFolder = m_savePlaylistInFolder;
    config.saveThumbnail = parSaveThumbnail;
    config.saveSubtitles = m_subtitlesBox->isChecked();
    config.forceIPv4 = m_forceIPv4;
    config.cookies = m_cookies;
    config.thumbnailVisibility = m_thumbnailVisibility;
    
    DownloadItem *newDownload = new DownloadItem(config, this);
    QListWidgetItem *item = new QListWidgetItem();
    
    connect(newDownload, &DownloadItem::removeRequested, [this, item]() {
        QTimer::singleShot(0, this, [this, item]() {
            delete item; 
            
            // Check for finished items
            bool hasFinishedItems = false;
            for (int i = 0; i < m_list->count(); ++i) {
                DownloadItem *di = qobject_cast<DownloadItem*>(m_list->itemWidget(m_list->item(i)));
                if (di && di->isFinished()) {
                    hasFinishedItems = true;
                    break;
                }
            }
            m_clearFinishedButton->setEnabled(hasFinishedItems);
        });
    });
    
    connect(newDownload, &DownloadItem::finishedSignal, this, &MainWindow::itemFinished);
    
    item->setSizeHint(newDownload->sizeHint());
    m_list->insertItem(0, item);
    m_list->setItemWidget(item, newDownload);
    
    m_linkBox->clear();
    
}
        
void MainWindow::aboutPage() {
    About aboutWindow(this);
    aboutWindow.exec();
}
        
void MainWindow::toggleQualityOptions() {
    if (!m_audioButton->isChecked()) {
        m_qualityBox->setEnabled(true);
    } else {
        m_qualityBox->setCurrentIndex(0);
        m_qualityBox->setEnabled(false);
    }
}
        
void MainWindow::changeSavePlaylistInFolder() {
    m_savePlaylistInFolder = m_savePlaylistInFolderAction->isChecked();
    QSettings settings;
    settings.setValue("savePlaylistInFolder", m_savePlaylistInFolderAction->isChecked());
}

void MainWindow::changeSaveThumbnail() {
    m_saveThumbnail = m_saveThumbnailAction->isChecked();
    QSettings settings;
    settings.setValue("saveThumbnail", m_saveThumbnailAction->isChecked());
}
        
void MainWindow::closeEvent(QCloseEvent *event) {
    // Check for active downloads
    bool hasActiveDownloads = false;
    for (int i = 0; i < m_list->count(); ++i) {
        DownloadItem *di = qobject_cast<DownloadItem*>(m_list->itemWidget(m_list->item(i)));
        
        if (di && !di->isFinished()) {
            hasActiveDownloads = true;
            break;
        }
    }
    
    if (hasActiveDownloads) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, tr("Warning"),
        tr("There is a download in progress.\nAre you sure you want to close Phoca?\nThe download will be cancelled."),
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::No);
        
        if (resBtn != QMessageBox::Yes) {
            event->ignore(); 
            return;
        }
    }
    
    event->accept(); 
}
        
void MainWindow::clearFinishedDownloads() {
    for (int i = m_list->count() - 1; i >= 0; --i) {
        
        QListWidgetItem *item = m_list->item(i);
        QWidget *widget = m_list->itemWidget(item);
        // Casting to class
        DownloadItem *downloadItem = qobject_cast<DownloadItem*>(widget);
        
        if (downloadItem && downloadItem->isFinished()) {
            delete item;
        }
    }
    m_clearFinishedButton->setEnabled(false);
}

void MainWindow::itemFinished() {
    m_clearFinishedButton->setEnabled(true);
}

void MainWindow::changeNightlyService() {
#ifdef FLATPAK_BUILD
    return; // OTA download disabled in Flatpak builds
#else
    m_nightlyService = m_chooseNightlyAction->isChecked();
    QSettings settings;
    settings.setValue("nightlyService", m_nightlyService);
    getServiceSlot();
#endif
}
        
void MainWindow::changeForceIPv4() {
    if (!m_forceIPv4Action->isChecked()) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, tr("Warning"),
        tr("Are you sure you want to change this setting?\nMost users will only need IPv4 and downloads may have issues when disabled depending on the network configuration"),
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::No);
        
        if (resBtn != QMessageBox::Yes) {
            // Signal blocked inside this scope
            QSignalBlocker blocker(m_forceIPv4Action);
            
            m_forceIPv4Action->setChecked(true);
            return;
        }
    }
    
    m_forceIPv4 = m_forceIPv4Action->isChecked();
    QSettings settings;
    settings.setValue("IPv4", m_forceIPv4);
}

void MainWindow::changeThumbnailVisibility() {
    m_thumbnailVisibility = m_thumbnailVisibilityAction->isChecked();
    
    for (int i = m_list->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = m_list->item(i);
        QWidget *widget = m_list->itemWidget(item);
        // Casting to class
        DownloadItem *downloadItem = qobject_cast<DownloadItem*>(widget);
        
        if (downloadItem) {
            downloadItem->changeThumbnailVisibility(m_thumbnailVisibility);
        }
    }
    
    QSettings settings;
    settings.setValue("thumbnailVisibility", m_thumbnailVisibility);
}

void MainWindow::changeCookies(const QString &browser) {
    m_cookies = browser;
    
    QSettings settings;
    settings.setValue("cookies", m_cookies);
}

void MainWindow::setupConnections() {
    connect(m_deleteAction, &QAction::triggered, this, [this]() {
        QListWidgetItem *currentItem = m_list->currentItem();
        if (!currentItem) return;
        
        DownloadItem *di = qobject_cast<DownloadItem*>(m_list->itemWidget(currentItem));
        if (di) {
            di->stopDownload(); 
        }
    });

    connect(m_linkBox, &QLineEdit::textChanged, this, &MainWindow::setDownloadReadiness);
    connect(m_qualityBox, &QComboBox::editTextChanged, this, &MainWindow::setDownloadReadiness);
    connect(m_conversionBox, &QComboBox::editTextChanged, this, &MainWindow::setDownloadReadiness);
#ifndef FLATPAK_BUILD
    connect(m_getEngineButton, &QPushButton::clicked, this, &MainWindow::getServiceSlot);
    connect(m_maintainer, &ServiceMaintainer::started, this, &MainWindow::engineDownloading);
    connect(m_maintainer, &ServiceMaintainer::finished, this, &MainWindow::engineDownloaded);
    connect(m_chooseStableAction, &QAction::triggered, this, &MainWindow::changeNightlyService);
    connect(m_chooseNightlyAction, &QAction::triggered, this, &MainWindow::changeNightlyService);
#endif
    connect(m_chooseLocationAction, &QAction::triggered, this, &MainWindow::changeLocation);
    connect(m_forceIPv4Action, &QAction::triggered, this, &MainWindow::changeForceIPv4);
    connect(m_thumbnailVisibilityAction, &QAction::triggered, this, &MainWindow::changeThumbnailVisibility);
    connect(m_aboutAction, &QAction::triggered, this,  &MainWindow::aboutPage);
    connect(m_clearFinishedButton, &QPushButton::clicked, this, &MainWindow::clearFinishedDownloads);

    connect(m_linkBox, &QLineEdit::textChanged, this, [this](const QString &text) {
        int currentLength = text.length();

        if (qAbs(currentLength - m_lastLength) > 1 && currentLength > 0) {
        m_linkBox->setCursorPosition(0);
        }

        m_lastLength = currentLength;
    });

    connect(m_bothButton, &QPushButton::clicked, this,  &MainWindow::toggleQualityOptions);
    connect(m_videoButton, &QPushButton::clicked, this,  &MainWindow::toggleQualityOptions);
    connect(m_audioButton, &QPushButton::clicked, this,  &MainWindow::toggleQualityOptions);
    connect(m_bothButton, &QRadioButton::clicked, this, [this]() {
        m_conversionBox->clear();
        m_conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"}); 
    });
    connect(m_videoButton, &QRadioButton::clicked, this, [this]() {
        m_conversionBox->clear();
        m_conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"}); 
    });
    connect(m_audioButton, &QRadioButton::clicked, this, [this]() {
        m_conversionBox->clear();
        m_conversionBox->addItems({tr("Original"), ".mp3", ".wav", ".flac", ".m4a"}); 
    });

    connect(m_savePlaylistInFolderAction, &QAction::triggered, this, &MainWindow::changeSavePlaylistInFolder);
    connect(m_saveThumbnailAction, &QAction::triggered, this, &MainWindow::changeSaveThumbnail);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::startDownload);
    connect(m_linkBox, &QLineEdit::returnPressed, m_downloadButton, &QPushButton::click);
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    m_fullLayout = new QVBoxLayout(m_centralWidget);
    m_layout = new QVBoxLayout();
    m_linkLayout = new QHBoxLayout();
    m_optionsLayout = new QHBoxLayout();
    m_linkBox = new QLineEdit(m_centralWidget);
    m_downloadButton = new QPushButton(m_centralWidget);
    m_clearFinishedButton = new QPushButton(m_centralWidget);
    m_getEngineButton = new QPushButton(m_centralWidget);
    m_maintainer = new ServiceMaintainer(this);
    m_list = new QListWidget(m_centralWidget);
    m_deleteAction = new QAction(m_list);
    m_bothButton = new QRadioButton(tr("Both"), m_centralWidget);
    m_videoButton = new QRadioButton(tr("Video"), m_centralWidget);
    m_audioButton = new QRadioButton(tr("Audio"), m_centralWidget);
    m_optionsMenu = new QMenu(tr("Options"), this);
    m_aboutAction = new QAction(tr("About"), this);
    m_buildMenu = new QMenu(tr("Choose yt-dlp version"), m_optionsMenu);
    m_advancedMenu = new QMenu(tr("Advanced"), m_optionsMenu);
    m_cookiesMenu = new QMenu(tr("Browser cookies"), m_advancedMenu);
    m_chooseLocationAction = new QAction(tr("Change download location..."), this);
    m_chooseNightlyAction = new QAction(tr("Use yt-dlp nightly (recommended)"), this);
    m_chooseStableAction = new QAction(tr("Use yt-dlp stable"), this);
    m_versionGroup = new QActionGroup(this);
    m_savePlaylistInFolderAction = new QAction(tr("Save playlists in folder"), this);
    m_saveThumbnailAction = new QAction(tr("Save thumbnail"), this);
    m_forceIPv4Action = new QAction(tr("Force IPv4 connections (Recommended)"), this);
    m_cookiesAction = new QAction("Browser cookies", this);
    m_cookiesGroup = new QActionGroup(this);
    m_thumbnailVisibilityAction = new QAction(tr("Show thumbnails on list"), this);
    m_bottomLayout = new QHBoxLayout();
    m_qualityBox = new QComboBox(this);
    m_conversionBox = new QComboBox(this);
    m_subtitlesBox = new QCheckBox(this);
    m_spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_locationLabel = new QLabel(this);

    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setShortcutContext(Qt::WidgetShortcut);
    m_list->addAction(m_deleteAction);
    
    /* Menu */
    menuBar()->addMenu(m_optionsMenu);
    m_optionsMenu->addMenu(m_buildMenu);
    menuBar()->addAction(m_aboutAction);
    
    auto addCookieOption = [this](const QString &label, const QString &value) {
        QAction *action = m_cookiesMenu->addAction(label, this, [this, value]() { changeCookies(value); });
        action->setCheckable(true);
        action->setChecked(value == m_cookies);
        m_cookiesGroup->addAction(action);
        return action;
    };
    
    addCookieOption("No cookies", "");
    addCookieOption("Brave", "brave");
    addCookieOption("Chrome", "chrome");
    addCookieOption("Chromium", "chromium");
    addCookieOption("Edge", "edge");
    addCookieOption("Firefox", "firefox");
    addCookieOption("Opera", "opera");
    addCookieOption("Safari", "safari");
    addCookieOption("Vivaldi", "vivaldi");
    addCookieOption("Whale", "whale");
    
    m_savePlaylistInFolderAction->setCheckable(true);
    m_savePlaylistInFolderAction->setChecked(m_savePlaylistInFolder);
    m_saveThumbnailAction->setCheckable(true);
    m_saveThumbnailAction->setChecked(m_saveThumbnail);
    m_chooseNightlyAction->setCheckable(true);
    m_chooseStableAction->setCheckable(true);
    m_chooseNightlyAction->setChecked(m_nightlyService);
    m_chooseStableAction->setChecked(!m_nightlyService);
    m_versionGroup->addAction(m_chooseNightlyAction);
    m_versionGroup->addAction(m_chooseStableAction);
    
    m_forceIPv4Action->setCheckable(true);
    m_forceIPv4Action->setChecked(m_forceIPv4);
    
    m_thumbnailVisibilityAction->setCheckable(true);
    m_thumbnailVisibilityAction->setChecked(m_thumbnailVisibility);
    
    m_optionsMenu->addAction(m_chooseLocationAction);
    m_optionsMenu->addAction(m_savePlaylistInFolderAction);
    m_buildMenu->addAction(m_chooseNightlyAction);
    m_buildMenu->addAction(m_chooseStableAction);
    
    m_optionsMenu->addAction(m_thumbnailVisibilityAction);
    m_optionsMenu->addAction(m_saveThumbnailAction);
    
    m_optionsMenu->addMenu(m_advancedMenu);
    m_advancedMenu->addAction(m_forceIPv4Action);
    m_advancedMenu->addMenu(m_cookiesMenu);
    
    m_linkBox->setPlaceholderText(tr("Enter link..."));
    m_downloadButton->setText(tr("Download"));
    m_downloadButton->setEnabled(false);
    m_clearFinishedButton->setText(tr("Clear finished"));
    m_getEngineButton->setText(tr("Update yt-dlp"));
    
    m_fullLayout->addLayout(m_layout);
    m_fullLayout->addWidget(m_list);
    
    m_layout->addLayout(m_linkLayout);
    m_linkLayout->addWidget(m_linkBox);
    m_layout->addLayout(m_optionsLayout);
    m_linkLayout->addWidget(m_downloadButton);

    m_bottomLayout->addWidget(m_clearFinishedButton);
    m_clearFinishedButton->setEnabled(false);
    m_bottomLayout->addWidget(m_getEngineButton);

    #ifdef FLATPAK_BUILD
        m_getEngineButton->setEnabled(false);
        m_getEngineButton->setToolTip(tr("yt-dlp OTA updates disabled on Flatpak version"));
        m_buildMenu->menuAction()->setVisible(false);
    #endif
    m_layout->addLayout(m_bottomLayout);
    
    m_optionsLayout->addWidget(m_bothButton);
    m_optionsLayout->addWidget(m_videoButton);
    m_optionsLayout->addWidget(m_audioButton);
    
    m_qualityBox->setEditable(true);
    m_qualityBox->setInsertPolicy(QComboBox::NoInsert);
    m_qualityBox->addItems({tr("Best"), "2160p", "1440p", "1080p", "720p", "480p"});
    m_optionsLayout->addWidget(m_qualityBox);
    
    m_conversionBox->setEditable(true);
    m_conversionBox->setInsertPolicy(QComboBox::NoInsert);
    m_conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"});
    m_optionsLayout->addWidget(m_conversionBox);
    
    m_subtitlesBox->setText(tr("Subtitles"));
    m_subtitlesBox->setChecked(false);
    m_optionsLayout->addWidget(m_subtitlesBox);
    
    m_optionsLayout->addItem(m_spacer);
    
    m_bothButton->setChecked(true);
}