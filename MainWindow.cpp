#include "MainWindow.h"
#include "ServiceMaintainer.h"

MainWindow::MainWindow(QWidget *parent) // Constructor
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    linkBox = new QLineEdit(this);
    downloadButton = new QPushButton(this);
    getEngineButton = new QPushButton(this);
    statusLabel = new QLabel(this);

    maintainer = new ServiceMaintainer(this);

    linkBox->setPlaceholderText("Enter link...");
    downloadButton->setText("Download link");
    downloadButton->setEnabled(false);
    getEngineButton->setText("Update yt-dlp");

    layout->addWidget(linkBox);
    layout->addWidget(statusLabel);
    layout->addWidget(downloadButton);
    layout->addWidget(getEngineButton);

    this->setLayout(layout);
    this->setWindowTitle("Phoca");

    connect(linkBox, &QLineEdit::textChanged, this, &MainWindow::setDownloadReadiness); // Update downlaod button
    connect(getEngineButton, &QPushButton::clicked, maintainer, &ServiceMaintainer::getService); // Download yt-dlp
    connect(maintainer, &ServiceMaintainer::started, this, &MainWindow::engineDownloading); // yt-dlp finished downloading
    connect(maintainer, &ServiceMaintainer::finished, this, &MainWindow::engineDownloaded); // yt-dlp finished downloading

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