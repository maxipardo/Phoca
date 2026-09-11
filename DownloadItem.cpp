#include "DownloadItem.h"
#include <QLayout>
#include <QMenu>
#include <QDesktopServices>
#include <QStyle>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>
#include <QProcess>
#include <QDir>
#include <QUrl>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QFile>
#ifdef Q_OS_LINUX
#include <QDBusMessage>
#include <QDBusConnection>
#endif

DownloadItem::DownloadItem (const DownloadConfig config, QWidget *parent) : QWidget(parent) {
      
    DownloadConfig ServiceConfig {config};
    fullFilePath = ""; // Set on download finish
    discardText = (tr("Cancel download\tDel"));

    service = new Service(this);
    
    titleLabel = new QLabel(this);
    titleLabel->setMinimumWidth(50); 
    
    sizeLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setTextVisible(false);

    percentageLabel = new QLabel(this);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    
    layout->setContentsMargins(6, 0, 6, 0);

    infoIcon = new QLabel(this);
    restartButton = new QPushButton(this);
    discardButton = new QPushButton(this);

    restartButton->setIcon(QIcon::fromTheme("view-refresh"));
    discardButton->setIcon(QIcon::fromTheme("window-close"));
    infoIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(16, 16));
    infoIcon->setVisible(false);
    restartButton->setVisible(false);
    discardButton->setVisible(false);
    
    restartButton->setText("Retry");
    discardButton->setText("Discard");
    
    layout->addWidget(titleLabel, 3); 
    layout->addWidget(infoIcon);
    layout->addWidget(restartButton, 1);
    layout->addWidget(discardButton, 1);
    layout->addWidget(sizeLabel);
    layout->addWidget(progressBar, 1);
    layout->addWidget(percentageLabel);

    percentageLabel->setVisible(false);
    percentageLabel->setMargin(4);
    
    downloadLocation = config.downloadLocation;
    downloadPhase = "";
    
    /* Service */
      connect(service, &Service::downloadStarted, this,
            &DownloadItem::downloadStarted);
      connect(service, &Service::downloadFinished, this,
                  &DownloadItem::downloadFinished);
      connect(service, &Service::processFailed, this,
            &DownloadItem::downloadProcessFailed);
      connect(service, &Service::percentageUpdated, this,
                  &DownloadItem::downloadProgress);
      connect(service, &Service::phaseUpdated, this,
            &DownloadItem::downloadPhaseUpdated);
      connect(service, &Service::titleUpdated, this, 
                  &DownloadItem::onTitleUpdated);
      connect(service, &Service::sizeUpdated, this, 
                        &DownloadItem::onSizeUpdated);
      connect(service, &Service::filePath, this, &DownloadItem::onFullPathUpdated);

      connect(restartButton, &QPushButton::clicked, this, &DownloadItem::retryDownload);
      connect(discardButton, &QPushButton::clicked, this, &DownloadItem::stopDownload);


      service->startDownload(config.link, config.downloadLocation, config.format, 
                              config.quality, config.conversion, 
                              config.playlist, config.savePlaylistInFolder, config.saveThumbnail);
}                  
// Service
void DownloadItem::downloadStarted() {
      updateTitleText(tr("Download started"));
      if (progressBar->maximum() == 0) {
            progressBar->setRange(0, 100);
      }
      progressBar->setValue(0);
      percentageLabel->setText("0%");
      percentageLabel->setVisible(true);
      updateElidedText();
}

void DownloadItem::downloadFinished(int exit) {
      discardText = (tr("Discard download\tDel"));
      if (exit == 0) {
            progressBar->setRange(0, 100);
            progressBar->setValue(100);
            
            if (fullTitle == tr("Download started")) {
                  if (downloadPhase == tr("Already downloaded")) {
                        updateTitleText(tr("Already downloaded")); 
                  } else {
                        updateTitleText(tr("Download finished"));
                  }
            }
            
            QTimer::singleShot(0, this, &DownloadItem::updateElidedText);
            downloadFinishedState = true;
            emit finishedSignal();
            return;
      } else if (exit == 9) {
            updateTitleText(tr("Download stopped"));
      } else if (exit == -1) {
            updateTitleText(tr("Download failed: process crashed"));
      } else {
            updateTitleText(tr("Download failed, error code: %1").arg(QString::number(exit)));
            infoIcon->setVisible(true);
            restartButton->setVisible(true);
            discardButton->setVisible(true);
            sizeLabel->setVisible(false);
            progressBar->setVisible(false);
            percentageLabel->setVisible(false);
      }
      downloadFinishedState = true;
      emit finishedSignal();
};

void DownloadItem::downloadProgress(int percentage) {
    if (percentage >= progressBar->value() || (progressBar->value() - percentage) > 50) {
        progressBar->setValue(percentage);
    }
    percentageLabel->setText(QString::number(percentage) + "%");
    updateElidedText();
}

void DownloadItem::onSizeUpdated(QString cleanSize) {
    downloadedSize = cleanSize;
    sizeLabel->setText(downloadedSize);
}

void DownloadItem::onTitleUpdated(QString title) {
      title.remove(QRegularExpression("\\.f\\d+.*$"));
      updateTitleText(title);
}

void DownloadItem::downloadPhaseUpdated(QString phase) {
      downloadPhase = phase;

      if (phase == tr("Processing...")) {
            progressBar->setRange(0, 0);
      } else {
            progressBar->setRange(0, 100);
      }
}

void DownloadItem::downloadProcessFailed(QString error) {
      infoIcon->setToolTip(error);
}

void DownloadItem::stopDownload() {
      service->stopDownload();
      emit removeRequested();
}

// Text changes centralized
void DownloadItem::updateTitleText(const QString &text) {
    fullTitle = text;
    updateElidedText();
}

void DownloadItem::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); 
    updateElidedText();          
}

void DownloadItem::updateElidedText() {
    if (fullTitle.isEmpty()) return;
    
    QFontMetrics metrics(titleLabel->font());
    QString elidedTitle = metrics.elidedText(fullTitle, Qt::ElideRight, titleLabel->width());
    
    titleLabel->setText(elidedTitle);
}

// Context menu actions
void DownloadItem::contextMenuEvent(QContextMenuEvent *event) {
      QMenu *menu = new QMenu(this);
      
      menu->setAttribute(Qt::WA_DeleteOnClose);
      
      // Check if app is in dark mode
      bool isDarkMode = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
      
      QAction *openLocation = menu->addAction(tr("Open file location"));
      QIcon folderIcon = QIcon::fromTheme("document-open-folder");
      if (folderIcon.isNull()) {
            if (isDarkMode) {
                  folderIcon = QIcon(":/folder_light.svg");
            } else {
                  folderIcon = QIcon(":/folder_dark.svg");
            }
      }
      openLocation->setIcon(folderIcon);

      QAction *cancelAction = menu->addAction(discardText);
      QIcon cancelIcon = QIcon::fromTheme("process-stop");
      if (cancelIcon.isNull()) {
            if (isDarkMode) {
                  cancelIcon = QIcon(":/cancel_light.svg");
            } else {
                  cancelIcon = QIcon(":/cancel_dark.svg");
            }
      }
      cancelAction->setIcon(cancelIcon); 

      QAction *deleteFileAction;
      QIcon deleteIcon;
      if (!fullFilePath.isEmpty()) {
            deleteFileAction = menu->addAction(tr("Delete file"));
            deleteIcon = QIcon::fromTheme("edit-delete");
            if (deleteIcon.isNull()) {
                  if (isDarkMode) {
                        deleteIcon = QIcon(":/delete_light.svg");
                  } else {
                        deleteIcon = QIcon(":/delete_dark.svg");
                  }
            }
            deleteFileAction->setIcon(deleteIcon);
      }

      connect(cancelAction, &QAction::triggered, this, &DownloadItem::stopDownload);
      if (!fullFilePath.isEmpty()) {
            connect(openLocation, &QAction::triggered, this, &DownloadItem::openFileLocation);
            connect(deleteFileAction, &QAction::triggered, this, &DownloadItem::deleteFile);
      } else {
            connect(openLocation, &QAction::triggered, this, &DownloadItem::openDownloadLocation);
      }
      
      // Asyncronus menu
      menu->popup(event->globalPos());
}

void DownloadItem::retryDownload() {
      infoIcon->setVisible(false);
      restartButton->setVisible(false);
      discardButton->setVisible(false);
      sizeLabel->setVisible(true);
      progressBar->setVisible(true);
      percentageLabel->setVisible(true);

      service->startDownload(ServiceConfig.link, ServiceConfig.downloadLocation, ServiceConfig.format, 
                              ServiceConfig.quality, ServiceConfig.conversion, 
                              ServiceConfig.playlist, ServiceConfig.savePlaylistInFolder, ServiceConfig.saveThumbnail);
}

void DownloadItem::onFullPathUpdated(QString fullPath) {
      fullFilePath = fullPath;
      qDebug() << fullPath;
}

void DownloadItem::openDownloadLocation() {
      QDesktopServices::openUrl(QUrl::fromLocalFile(downloadLocation));
}

void DownloadItem::openFileLocation() {
      #if defined(Q_OS_WIN)
        // Windows
        QString windowsPath = QDir::toNativeSeparators(fullFilePath);
        
        QString command = QString("explorer.exe /select,\"%1\"").arg(windowsPath);
        
        QProcess::startDetached(command);

      #elif defined(Q_OS_LINUX)
        // Check for DBus
        if (QDBusConnection::sessionBus().isConnected()) {
            QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.FileManager1",
                "/org/freedesktop/FileManager1",
                "org.freedesktop.FileManager1",
                "ShowItems"
            );

            QStringList uris;
            uris << QUrl::fromLocalFile(fullFilePath).toString();
            msg << uris << QString("");

            QDBusConnection::sessionBus().send(msg);
        } else {
            // If not available
            QDesktopServices::openUrl(QUrl::fromLocalFile(downloadLocation));
        }
      #else
        // Other
        QDesktopServices::openUrl(QUrl::fromLocalFile(downloadLocation));
      #endif
}

void DownloadItem::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !fullFilePath.isEmpty()) {
        openFileLocation();

    }
    QWidget::mouseDoubleClickEvent(event);
}

void DownloadItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    // Continues
    QWidget::mousePressEvent(event);
}

void DownloadItem::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // If download hasn't finished
    if (fullFilePath.isEmpty()) {
        return; 
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(fullFilePath);
    mimeData->setUrls(urls);
    
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);
}

void DownloadItem::deleteFile() {
      if (!fullFilePath.isEmpty()) {
            QFile file(fullFilePath);

            if (file.exists()) {
                  if (file.remove()) {
                        qDebug() << "File succesfully deleted:" << fullFilePath;
                        emit removeRequested();
                  } else {
                        qDebug() << "Error: Couldn't delete file.";
                        titleLabel->setText(tr("Couldn't delete file"));
                  }
            } else {
                  qDebug() << "File does not exist.";
                  titleLabel->setText(tr("Couldn't delete file"));
            }
      }
}