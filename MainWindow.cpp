#include "MainWindow.h"
#include "Service.h"
#include "About.h"
#include <qaction.h>
#include <qmessagebox.h>
#include <qstandardpaths.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QWidget *centralWidget = new QWidget(this);

  layout = new QVBoxLayout(centralWidget);
  linkLayout = new QHBoxLayout();
  optionsLayout = new QHBoxLayout();
  linkBox = new QLineEdit(centralWidget);
  downloadButton = new QPushButton(centralWidget);
  getEngineButton = new QPushButton(centralWidget);
  statusLabel = new QLabel(centralWidget);
  titleLabel = new QLabel(centralWidget);
  progressBar = new QProgressBar(centralWidget);
  maintainer = new ServiceMaintainer(this);
  chosenDirectory = QDir::homePath();

  bothButton = new QRadioButton("Both", centralWidget);
  videoButton = new QRadioButton("Video", centralWidget);
  audioButton = new QRadioButton("Audio", centralWidget);

  /* MainWindow size */
  this->resize(462, 100);

  /* Download location */
  QSettings settings("MaximoPardo", "Phoca");
  downloadLocation = settings.value("downloadLocation", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();

  /* Persistent playlist in folder */
  savePlaylistInFolder = settings.value("savePlaylistInFolder", true).toBool();

  /* Menu */
  optionsMenu = new QMenu("Options", this);
  aboutAction = new QAction("About", this);
  buildMenu = new QMenu("Choose yt-dlp version", optionsMenu);
  menuBar()->addMenu(optionsMenu);
  optionsMenu->addMenu(buildMenu);
  menuBar()->addAction(aboutAction);

  chooseLocationAction = new QAction("Change download location...", this);
  chooseNightlyAction = new QAction("Use yt-dlp nightly (recommended)", this);
  chooseStableAction = new QAction("Use yt-dlp stable", this);
  versionGroup = new QActionGroup(this);
  savePlaylistInFolderAction = new QAction("Save playlists in folder", this);

  savePlaylistInFolderAction->setCheckable(true);
  savePlaylistInFolderAction->setChecked(savePlaylistInFolder);
  chooseNightlyAction->setCheckable(true);
  chooseStableAction->setCheckable(true);
  chooseNightlyAction->setChecked(true);
  versionGroup->addAction(chooseNightlyAction);
  versionGroup->addAction(chooseStableAction);

  optionsMenu->addAction(chooseLocationAction);
  optionsMenu->addAction(savePlaylistInFolderAction);
  buildMenu->addAction(chooseNightlyAction);
  buildMenu->addAction(chooseStableAction);

  linkBox->setPlaceholderText("Enter link...");
  downloadButton->setText("Download");
  downloadButton->setEnabled(false);

  getEngineButton->setText("Update yt-dlp");
  
  layout->addLayout(linkLayout);
  linkLayout->addWidget(linkBox);
  layout->addLayout(optionsLayout);
  linkLayout->addWidget(downloadButton);
  layout->addWidget(titleLabel);
  layout->addWidget(statusLabel);
  layout->addWidget(progressBar);
  layout->addWidget(getEngineButton);
  titleLabel->hide();
  progressBar->hide();

  optionsLayout->addWidget(bothButton);
  optionsLayout->addWidget(videoButton);
  optionsLayout->addWidget(audioButton);

  qualityBox = new QComboBox(this);
  qualityBox->setEditable(true);
  qualityBox->setInsertPolicy(QComboBox::NoInsert);
  qualityBox->addItem("Best");
  qualityBox->addItem("2160p");
  qualityBox->addItem("1440p");
  qualityBox->addItem("1080p");
  qualityBox->addItem("720p");
  qualityBox->addItem("480p");
  optionsLayout->addWidget(qualityBox);

  conversionBox = new QComboBox(this);
  conversionBox->setEditable(true);
  conversionBox->setInsertPolicy(QComboBox::NoInsert);
  conversionBox->addItem("Original");
  conversionBox->addItem(".mp4");
  conversionBox->addItem(".mkv");
  conversionBox->addItem(".avi");
  conversionBox->addItem(".webm");
  optionsLayout->addWidget(conversionBox);

  QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  optionsLayout->addItem(spacer);

  bothButton->setChecked(true);

  this->setWindowTitle("Phoca");
  setCentralWidget(centralWidget);
  this->layout->setSizeConstraint(QLayout::SetMinimumSize);

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

  connect(bothButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);
  connect(videoButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);
  connect(audioButton, &QPushButton::clicked, this, 
          &MainWindow::toggleQualityOptions);

  connect(bothButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({"Original", ".mp4", ".mkv", ".webm"}); 
});
  connect(videoButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({"Original", ".mp4", ".mkv", ".webm"}); 
});
connect(audioButton, &QRadioButton::clicked, this, [this]() {
    conversionBox->clear();
    conversionBox->addItems({"Original", ".mp3", ".wav", ".flac", ".m4a"}); 
});

connect(savePlaylistInFolderAction, &QAction::triggered, this, &MainWindow::changeSavePlaylistInFolder);

  /* Service */
  downloadPhase = "";
  service = new Service(this);

  connect(downloadButton, &QPushButton::clicked, this,
          &MainWindow::startDownload);
  connect(service, &Service::downloadStarted, this,
          &MainWindow::downloadStarted);
  connect(service, &Service::downloadFinished, this,
          &MainWindow::downloadFinished);
  connect(service, &Service::processFailed, this,
          &MainWindow::downloadProcessFailed);
  connect(service, &Service::percentageUpdated, this,
          &MainWindow::downloadProgress);
  connect(service, &Service::phaseUpdated, this,
          &MainWindow::downloadPhaseUpdated);
  connect(service, &Service::titleUpdated, this, 
          &MainWindow::onTitleUpdated);

  this->statusBar()->showMessage("Download location: " + downloadLocation);
}

void MainWindow::engineDownloading() { statusLabel->setText("Downloading..."); }

void MainWindow::engineDownloaded(int exit) {
  switch (exit) {
  case 0:
    statusLabel->setText("yt-dlp downloaded successfully");
    break;
  case 1:
    statusLabel->setText("Download failed: Network error");
    break;
  case 2:
    statusLabel->setText("Download failed: Need permissions to write");
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
}

void MainWindow::changeLocation() {
  downloadLocation = QFileDialog::getExistingDirectory(
      this, "Choose where to save files", QDir::homePath(),
      QFileDialog::ShowDirsOnly);

  if (!downloadLocation.isEmpty()) {
    QSettings settings("MaximoPardo", "Phoca");
    
    settings.setValue("downloadLocation", downloadLocation);

    qDebug() << "Settings saved. Chosen folder:" << downloadLocation;
    this->statusBar()->showMessage("Download location: " + downloadLocation);
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
    msgBox.setWindowTitle("Playlist detected");
    msgBox.setText("This link contains a playlist.\n¿Download the whole list?");
    
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

  service->startDownload(linkBox->text(), downloadLocation, format, qualityBox->currentText(), conversionBox->currentText(), playlist, savePlaylistInFolder);

  titleLabel->hide();
}

void MainWindow::downloadStarted() {
  statusLabel->setText("Download started");
  if (progressBar->maximum() == 0) {
        progressBar->setRange(0, 100);
    }
  progressBar->setValue(0);
  progressBar->show();
}

void MainWindow::downloadProgress(int percentage) { // Percentage updated
  statusLabel->setText(downloadPhase);
  progressBar->setValue(percentage);
}

void MainWindow::downloadFinished(int exit) {
  if (exit == 0) {
    statusLabel->setText("Download finished successfully at: " + downloadLocation);
  } else if (exit == 1) {
    statusLabel->setText("Download failed, error code: " +
                         QString::number(exit));
  }
  progressBar->hide();
}

void MainWindow::downloadProcessFailed(QString error) {
  statusLabel->setText(error);
  progressBar->hide();
}

void MainWindow::downloadPhaseUpdated(QString phase) {
    downloadPhase = phase;

    if (phase == "Processing...") {
        progressBar->setRange(0, 0);
    } else {
        progressBar->setRange(0, 100);
    }
}

void MainWindow::onTitleUpdated(QString title) {
  this->statusBar()->showMessage(title.toUpper());
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