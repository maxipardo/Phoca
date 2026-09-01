#include "DownloadItem.h"
#include <QLayout>
#include <QMenu>
#include <QDesktopServices>
#include <QStyle>
#include <QGuiApplication>
#include <QStyleHints>

DownloadItem::DownloadItem (const DownloadConfig config, QWidget *parent) : QWidget(parent) {
    service = new Service(this);
    
    titleLabel = new QLabel(this);
    titleLabel->setMinimumWidth(50); 
    
    sizeLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    //progressBar->setMaximumWidth(100);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    
    layout->setContentsMargins(6, 0, 6, 0);
    
    layout->addWidget(titleLabel, 3); 
    
    layout->addWidget(sizeLabel);
    layout->addWidget(progressBar, 1);
    
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

      service->startDownload(config.link, config.downloadLocation, config.format, 
                              config.quality, config.conversion, 
                              config.playlist, config.savePlaylistInFolder, config.saveThumbnail);
      progressBar->setTextVisible(false);
}                  
// Service
void DownloadItem::downloadStarted() {
      progressBar->setTextVisible(true);
      updateTitleText(tr("Download started"));
      if (progressBar->maximum() == 0) {
            progressBar->setRange(0, 100);
      }
      progressBar->setValue(0);
      updateElidedText();
}

void DownloadItem::downloadFinished(int exit) {
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
            progressBar->setTextVisible(false);
      } else if (exit == 9) {
            updateTitleText(tr("Download stopped"));
      } else if (exit == -1) {
            updateTitleText(tr("Download failed: process crashed"));
      } else {
            updateTitleText(tr("Download failed, error code: %1").arg(QString::number(exit)));
      }
      downloadFinishedState = true;
      emit finishedSignal();
};

void DownloadItem::downloadProgress(int percentage) {
    if (percentage >= progressBar->value() || (progressBar->value() - percentage) > 50) {
        progressBar->setValue(percentage);
    }
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
      updateTitleText(tr("Process failed: %1").arg(error));
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

      QAction *cancelAction = menu->addAction(tr("Delete download\tDel"));
      QIcon cancelIcon = QIcon::fromTheme("process-stop");
      if (cancelIcon.isNull()) {
            if (isDarkMode) {
                  cancelIcon = QIcon(":/cancel_light.svg");
            } else {
                  cancelIcon = QIcon(":/cancel_dark.svg");
            }
      }
      cancelAction->setIcon(cancelIcon); 

      connect(cancelAction, &QAction::triggered, this, &DownloadItem::stopDownload);
      connect(openLocation, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(downloadLocation));
      });

      // Asyncronus menu
      menu->popup(event->globalPos());
}