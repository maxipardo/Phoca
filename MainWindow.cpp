#include "MainWindow.h"
#include "DownloadItem.h"
#include "About.h"
#include "DownloadConfig.h"
#include "ServiceMaintainer.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QWidget *centralWidget = new QWidget(this);
  fullLayout = new QVBoxLayout(centralWidget);
  layout = new QVBoxLayout();
  linkLayout = new QHBoxLayout();
  optionsLayout = new QHBoxLayout();
  linkBox = new QLineEdit(centralWidget);
  downloadButton = new QPushButton(centralWidget);
  clearFinishedButton = new QPushButton(centralWidget);
  getEngineButton = new QPushButton(centralWidget);
  statusLabel = new QLabel(centralWidget);
  titleLabel = new QLabel(centralWidget);
  progressBar = new QProgressBar(centralWidget);
  maintainer = new ServiceMaintainer(this);
  chosenDirectory = QDir::homePath();

  list = new QListWidget(centralWidget);

  QAction *deleteAction = new QAction(list);
  deleteAction->setShortcut(QKeySequence::Delete);
  deleteAction->setShortcutContext(Qt::WidgetShortcut);
  list->addAction(deleteAction);

  // Supr key
  connect(deleteAction, &QAction::triggered, this, [this]() {
      QListWidgetItem *currentItem = list->currentItem();
      if (!currentItem) return;

      DownloadItem *di = qobject_cast<DownloadItem*>(list->itemWidget(currentItem));
      if (di) {
          di->stopDownload(); 
      }
  });

  bothButton = new QRadioButton(tr("Both"), centralWidget);
  videoButton = new QRadioButton(tr("Video"), centralWidget);
  audioButton = new QRadioButton(tr("Audio"), centralWidget);

  /* MainWindow size */
  this->resize(200, 200);
  this->setMinimumWidth(462);

  /* Persistent settings */
  QSettings settings("MaximoPardo", "Phoca");
  downloadLocation = settings.value("downloadLocation", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
  savePlaylistInFolder = settings.value("savePlaylistInFolder", true).toBool();
  saveThumbnail = settings.value("saveThumbnail", false).toBool();
  firstLaunch = settings.value("firstLaunch", true).toBool();
  lastEngineUpdate = settings.value("lastEngineUpdate", QDateTime::currentDateTime()).toDateTime();
  QDateTime now = QDateTime::currentDateTime();

  if (lastEngineUpdate.daysTo(now) >= 3) {
    getServiceSlot();
}

  /* Menu */
  optionsMenu = new QMenu(tr("Options"), this);
  aboutAction = new QAction(tr("About"), this);
  buildMenu = new QMenu(tr("Choose yt-dlp version"), optionsMenu);
  menuBar()->addMenu(optionsMenu);
  optionsMenu->addMenu(buildMenu);
  menuBar()->addAction(aboutAction);

  chooseLocationAction = new QAction(tr("Change download location..."), this);
  chooseNightlyAction = new QAction(tr("Use yt-dlp nightly (recommended)"), this);
  chooseStableAction = new QAction(tr("Use yt-dlp stable"), this);
  versionGroup = new QActionGroup(this);
  savePlaylistInFolderAction = new QAction(tr("Save playlists in folder"), this);
  saveThumbnailAction = new QAction(tr("Save thumbnail"), this);

  savePlaylistInFolderAction->setCheckable(true);
  savePlaylistInFolderAction->setChecked(savePlaylistInFolder);
  saveThumbnailAction->setCheckable(true);
  saveThumbnailAction->setChecked(saveThumbnail);
  chooseNightlyAction->setCheckable(true);
  chooseStableAction->setCheckable(true);
  chooseNightlyAction->setChecked(true);
  versionGroup->addAction(chooseNightlyAction);
  versionGroup->addAction(chooseStableAction);

  optionsMenu->addAction(chooseLocationAction);
  optionsMenu->addAction(savePlaylistInFolderAction);
  optionsMenu->addAction(saveThumbnailAction);
  buildMenu->addAction(chooseNightlyAction);
  buildMenu->addAction(chooseStableAction);

  linkBox->setPlaceholderText(tr("Enter link..."));
  downloadButton->setText(tr("Download"));
  downloadButton->setEnabled(false);
  clearFinishedButton->setText(tr("Clear finished"));
  getEngineButton->setText(tr("Update yt-dlp"));

  fullLayout->addLayout(layout);
  fullLayout->addWidget(list);
  
  layout->addLayout(linkLayout);
  linkLayout->addWidget(linkBox);
  layout->addLayout(optionsLayout);
  linkLayout->addWidget(downloadButton);
  layout->addWidget(titleLabel);
  layout->addWidget(statusLabel);
  layout->addWidget(progressBar);
  QHBoxLayout *bottomLayout = new QHBoxLayout();
  bottomLayout->addWidget(clearFinishedButton);
  clearFinishedButton->setEnabled(false);
  bottomLayout->addWidget(getEngineButton);
  layout->addLayout(bottomLayout);
  titleLabel->hide();
  progressBar->hide();

  optionsLayout->addWidget(bothButton);
  optionsLayout->addWidget(videoButton);
  optionsLayout->addWidget(audioButton);

  qualityBox = new QComboBox(this);
  qualityBox->setEditable(true);
  qualityBox->setInsertPolicy(QComboBox::NoInsert);
  qualityBox->addItems({tr("Best"), "2160p", "1440p", "1080p", "720p", "480p"});
  optionsLayout->addWidget(qualityBox);

  conversionBox = new QComboBox(this);
  conversionBox->setEditable(true);
  conversionBox->setInsertPolicy(QComboBox::NoInsert);
  conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"});
  optionsLayout->addWidget(conversionBox);

  QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  optionsLayout->addItem(spacer);

  bothButton->setChecked(true);

  this->setWindowTitle("Phoca");
  setCentralWidget(centralWidget);

  connect(linkBox, &QLineEdit::textChanged, this,
          &MainWindow::setDownloadReadiness);
  connect(qualityBox, &QComboBox::editTextChanged, this,
          &MainWindow::setDownloadReadiness);
  connect(conversionBox, &QComboBox::editTextChanged, this,
          &MainWindow::setDownloadReadiness);
  connect(getEngineButton, &QPushButton::clicked, this,
          &MainWindow::getServiceSlot);
  connect(maintainer, &ServiceMaintainer::started, this,
          &MainWindow::engineDownloading);
  connect(maintainer, &ServiceMaintainer::finished, this,
          &MainWindow::engineDownloaded);
  connect(chooseStableAction, &QAction::triggered, this,
          &MainWindow::getServiceSlot);
  connect(chooseNightlyAction, &QAction::triggered, this,
          &MainWindow::getServiceSlot);
  connect(chooseLocationAction, &QAction::triggered, this,
          &MainWindow::changeLocation);
  connect(aboutAction, &QAction::triggered, this, 
          &MainWindow::aboutPage);
  connect(clearFinishedButton, &QPushButton::clicked, this,
          &MainWindow::clearFinishedDownloads);

  connect(linkBox, &QLineEdit::textChanged, this, [this](const QString &text) {
        static int lastLength = 0;
        int currentLength = text.length();
        
        if (qAbs(currentLength - lastLength) > 1 && currentLength > 0) {
            linkBox->setCursorPosition(0);
        }
        
        lastLength = currentLength;
    });

  connect(bothButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);
  connect(videoButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);
  connect(audioButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);

  connect(bothButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"}); 
  });
  connect(videoButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({tr("Original"), ".mp4", ".mkv", ".webm"}); 
  });
  connect(audioButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({tr("Original"), ".mp3", ".wav", ".flac", ".m4a"}); 
  });

  connect(savePlaylistInFolderAction, &QAction::triggered, this,
         &MainWindow::changeSavePlaylistInFolder);

  connect(saveThumbnailAction, &QAction::triggered, this,
         &MainWindow::changeSaveThumbnail);

  connect(downloadButton, &QPushButton::clicked, this,
          &MainWindow::startDownload);

  connect(linkBox, &QLineEdit::returnPressed,
         downloadButton, &QPushButton::click);

  //connect(list, &QListWidget::itemDoubleClicked, this, &MainWindow::openDirectory);
         
         
  locationLabel = new QLabel(this);
  locationLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  this->statusBar()->addWidget(locationLabel);
  updateLocationLabel();

  if (firstLaunch) {
    QTimer::singleShot(500, [this]() {
      getServiceSlot();
    });
    settings.setValue("firstLaunch", false);
  }
}

void MainWindow::updateLocationLabel() {
  const QString shown = QDir::toNativeSeparators(downloadLocation);
  locationLabel->setText(tr("Download location: %1").arg(shown));
  locationLabel->setToolTip(shown);
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
    this->statusBar()->showMessage(tr("[yt-dlp] Download failed: Need permissions to write"), 5000);
    break;
  }
  setDownloadReadiness();
}

void MainWindow::setDownloadReadiness() {
  // search quality in qualityBox
  bool validQuality = qualityBox->findText(qualityBox->currentText()) != -1;
  bool validConversion = conversionBox->findText(conversionBox->currentText()) != -1;

  if (!linkBox->text().isEmpty() && maintainer->exists() && validQuality && validConversion) {
    downloadButton->setEnabled(true);
  } else {
    downloadButton->setEnabled(false);
  }
}

void MainWindow::getServiceSlot() {
  downloadButton->setEnabled(false);
  bool nightly{chooseNightlyAction->isChecked()};
  maintainer->getService(nightly);
  QSettings settings("MaximoPardo", "Phoca");
  QDateTime now = QDateTime::currentDateTime();
  settings.setValue("lastEngineUpdate", now);
}

void MainWindow::changeLocation() {
  const QString dir = QFileDialog::getExistingDirectory(
      this, tr("Choose where to save files"), downloadLocation,
      QFileDialog::ShowDirsOnly);

  if (!dir.isEmpty()) {
    downloadLocation = dir;

    QSettings settings("MaximoPardo", "Phoca");
    settings.setValue("downloadLocation", downloadLocation);

    qDebug() << "Settings saved. Chosen folder:" << downloadLocation;
    updateLocationLabel();
  }
}

void MainWindow::startDownload() {
  int format {0}; // both
  if (videoButton->isChecked()) {
    format = 1;
  } else if (audioButton->isChecked()) {
    format = 2;
  }
  QString link = linkBox->text();
  bool playlist = false;
  if (link.contains("list=") || link.contains("/playlist/") || link.contains("/album/")) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("Playlist detected"));
    if (saveThumbnail) {
      msgBox.setText(tr("This link contains a playlist.\nDownload the whole list?\nThumbnails will not be saved"));
    } else {
      msgBox.setText(tr("This link contains a playlist.\nDownload the whole list?"));
    }
    
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.button(QMessageBox::Cancel)->hide();
    
    int answer = msgBox.exec();
    
    if (answer == QMessageBox::Yes) {
      playlist = true;
    } else if (answer == QMessageBox::No) {
      playlist = false;
    } else {
      return; 
    }
  }
  
  bool parSaveThumbnail{saveThumbnail && !playlist};

  QString quality;
  QString conversion;
  if (qualityBox->currentIndex() == 0) {
    quality = "0";
  } else {
    quality = qualityBox->currentText();
  }
  if (conversionBox->currentIndex() == 0) {
    conversion = "0";
  } else {
    conversion = conversionBox->currentText();
  }
  
  // Config struct
  DownloadConfig config;
  config.link = link;
  config.downloadLocation = downloadLocation;
  config.format = format;
  config.quality = quality;
  config.conversion = conversion;
  config.playlist = playlist;
  config.savePlaylistInFolder = savePlaylistInFolder;
  config.saveThumbnail = parSaveThumbnail;

  DownloadItem *newDownload = new DownloadItem(config, this);
  QListWidgetItem *item = new QListWidgetItem();

  connect(newDownload, &DownloadItem::removeRequested, [this, item]() {
          delete item; 
          
          // Check for finished items
          bool hasFinishedItems = false;
          for (int i = 0; i < list->count(); ++i) {
              DownloadItem *di = qobject_cast<DownloadItem*>(list->itemWidget(list->item(i)));
              if (di && di->isFinished()) {
                  hasFinishedItems = true;
                  break;
              }
          }
          clearFinishedButton->setEnabled(hasFinishedItems);
      });

  connect(newDownload, &DownloadItem::finishedSignal, this, &MainWindow::itemFinished);

  item->setSizeHint(newDownload->sizeHint());
  list->addItem(item);
  list->setItemWidget(item, newDownload);
                         
  titleLabel->hide();
  linkBox->clear();

}

void MainWindow::aboutPage() {
  About aboutWindow(this);
  aboutWindow.exec();
}

void MainWindow::toggleQualityOptions() {
  if (!audioButton->isChecked()) {
    qualityBox->setEnabled(true);
  } else {
    qualityBox->setCurrentIndex(0);
    qualityBox->setEnabled(false);
  }
}

void MainWindow::changeSavePlaylistInFolder() {
    savePlaylistInFolder = savePlaylistInFolderAction->isChecked();
    QSettings settings("MaximoPardo", "Phoca");
    settings.setValue("savePlaylistInFolder", savePlaylistInFolderAction->isChecked());
}

void MainWindow::changeSaveThumbnail() {
    saveThumbnail = saveThumbnailAction->isChecked();
    QSettings settings("MaximoPardo", "Phoca");
    settings.setValue("saveThumbnail", saveThumbnailAction->isChecked());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Check for active downloads
    bool hasActiveDownloads = false;
    for (int i = 0; i < list->count(); ++i) {
        DownloadItem *di = qobject_cast<DownloadItem*>(list->itemWidget(list->item(i)));

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
  for (int i = list->count() - 1; i >= 0; --i) {
      
      QListWidgetItem *item = list->item(i);
      QWidget *widget = list->itemWidget(item);
      // Casting to class
      DownloadItem *downloadItem = qobject_cast<DownloadItem*>(widget);
      
      if (downloadItem && downloadItem->isFinished()) {
          delete item; 
      }
  }
  clearFinishedButton->setEnabled(false);
}

void MainWindow::itemFinished() {
  clearFinishedButton->setEnabled(true);
}