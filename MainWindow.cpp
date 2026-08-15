#include "MainWindow.h"
#include "Service.h"
#include <qaction.h>
#include <qobject.h>
#include <QSpacerItem>
#include <QSizePolicy>

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
  this->resize(500, 100);
  this->setMinimumWidth(400);

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

  chooseNightlyAction->setCheckable(true);
  chooseStableAction->setCheckable(true);
  chooseNightlyAction->setChecked(true);
  versionGroup->addAction(chooseNightlyAction);
  versionGroup->addAction(chooseStableAction);

  optionsMenu->addAction(chooseLocationAction);
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

  QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  optionsLayout->addItem(spacer);

  bothButton->setChecked(true);

  this->setWindowTitle("Phoca");
  setCentralWidget(centralWidget);

  /* Download location */
  QFile file("config.txt");

  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream entrada(&file);
    downloadLocation = entrada.readAll().trimmed();
    file.close();
    qDebug() << "Download location found:" << downloadLocation;
  } else {
    downloadLocation =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    qDebug() << "file not found. Using: " << downloadLocation;
  }

  connect(linkBox, &QLineEdit::textChanged, this,
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
  connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutPage);

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
  if (!linkBox->text().isEmpty() && maintainer->exists()) {
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
    QFile file("config.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

      QTextStream salida(&file);
      salida << downloadLocation;
      file.close();
      qDebug() << "Settings saved.";

    } else {
      qDebug() << "Error: Couldn't create settings file.";
    }

    qDebug() << "Chosen folder:" << downloadLocation;
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

  service->startDownload(linkBox->text(), downloadLocation, format);
  titleLabel->hide();
}

void MainWindow::downloadStarted() {
  statusLabel->setText("Download started");
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

void MainWindow::downloadPhaseUpdated(QString phase) { downloadPhase = phase; }

void MainWindow::onTitleUpdated(QString title) {
  this->statusBar()->showMessage(title.toUpper());
}

void MainWindow::aboutPage() {
  About aboutWindow(this);
  aboutWindow.exec();
}