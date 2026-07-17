#include "MainWindow.h"
#include "ServiceMaintainer.h"

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent)
{
    // 1. Creamos el contenedor central
    QWidget *centralWidget = new QWidget(this);
    
    // 2. Inicializamos TU variable layout, atándola al centralWidget
    layout = new QVBoxLayout(centralWidget);
    
    // 3. Creamos los elementos (como parent le podés pasar el centralWidget)
    linkBox = new QLineEdit(centralWidget);
    downloadButton = new QPushButton(centralWidget);
    getEngineButton = new QPushButton(centralWidget);
    statusLabel = new QLabel(centralWidget);
    maintainer = new ServiceMaintainer(this);

    linkBox->setPlaceholderText("Enter link...");
    downloadButton->setText("Download link");
    downloadButton->setEnabled(false);
    getEngineButton->setText("Update yt-dlp");

    // 4. Metemos todo en el layout
    layout->addWidget(linkBox);
    layout->addWidget(statusLabel);
    layout->addWidget(downloadButton);
    layout->addWidget(getEngineButton);

    // 5. Configuración final de la ventana (¡sin this->setLayout!)
    this->setWindowTitle("Phoca");
    setCentralWidget(centralWidget);

    // 6. Conexiones
    connect(linkBox, &QLineEdit::textChanged, this, &MainWindow::setDownloadReadiness);
    connect(getEngineButton, &QPushButton::clicked, maintainer, &ServiceMaintainer::getService); 
    connect(maintainer, &ServiceMaintainer::started, this, &MainWindow::engineDownloading); 
    connect(maintainer, &ServiceMaintainer::finished, this, &MainWindow::engineDownloaded); 
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