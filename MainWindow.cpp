#include "MainWindow.h"
#include "Service.h"

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    
    layout = new QVBoxLayout(centralWidget);
    linkLayout = new QHBoxLayout(centralWidget);
    linkBox = new QLineEdit(centralWidget);
    downloadButton = new QPushButton(centralWidget);
    getEngineButton = new QPushButton(centralWidget);
    statusLabel = new QLabel(centralWidget);
    maintainer = new ServiceMaintainer(this);
    chosenDirectory = QDir::homePath();

    /* Menu */
    optionsMenu = new QMenu("Options", this);
    buildMenu = new QMenu("Choose yt-dlp version", optionsMenu);
    menuBar()->addMenu(optionsMenu);
    optionsMenu->addMenu(buildMenu);

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
    downloadButton->setText("Download link");
    downloadButton->setEnabled(false);
    getEngineButton->setText("Update yt-dlp");
    layout->addLayout(linkLayout);
    linkLayout->addWidget(linkBox);
    linkLayout->addWidget(downloadButton);
    layout->addWidget(statusLabel);
    layout->addWidget(getEngineButton);

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
        downloadLocation = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        qDebug() << "file not found. Using: " << downloadLocation;
    }

    connect(linkBox, &QLineEdit::textChanged, this, &MainWindow::setDownloadReadiness);
    connect(getEngineButton, &QPushButton::clicked, this, &MainWindow::getServiceSlot); 
    connect(maintainer, &ServiceMaintainer::started, this, &MainWindow::engineDownloading); 
    connect(maintainer, &ServiceMaintainer::finished, this, &MainWindow::engineDownloaded); 
    connect(chooseStableAction, &QAction::triggered, this, &MainWindow::getServiceSlot);
    connect(chooseNightlyAction, &QAction::triggered, this, &MainWindow::getServiceSlot);
    connect(chooseLocationAction, &QAction::triggered, this, &MainWindow::changeLocation);

    /* Service */
    service = new Service(this);

    connect(downloadButton, &QPushButton::clicked, this, &MainWindow::startDownload);
    connect(service, &Service::downloadStarted, this, &MainWindow::downloadStarted);
    connect(service, &Service::downloadFinished, this, &MainWindow::downloadFinished);
}

void MainWindow::engineDownloading() {
    statusLabel->setText("Downloading...");
}

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
    if (!linkBox->text().isEmpty() && maintainer->exists())
    {
        downloadButton->setEnabled(true);
    }
    else 
    {
        downloadButton->setEnabled(false);
    }
}

void MainWindow::getServiceSlot() {
    bool nightly {chooseNightlyAction->isChecked()};
    maintainer->getService(nightly);
}

void MainWindow::changeLocation() {
    downloadLocation = QFileDialog::getExistingDirectory(
    this,
    "Elegí dónde guardar los videos",
    QDir::homePath(),
    QFileDialog::ShowDirsOnly
);
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
    }
}

void MainWindow::startDownload() {
    service->startDownload(linkBox->text(), downloadLocation);
}

void MainWindow::downloadStarted() {
    statusLabel->setText("Download started");
}

void MainWindow::downloadFinished(int exit) {
    if (exit == 0) {
        statusLabel->setText("Download finished successfully");
    } else if (exit == 1) {
        statusLabel->setText("Download failed");
    }
}