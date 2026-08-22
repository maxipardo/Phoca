#include "DownloadItem.h"
#include <QLayout>
#include <QMenu>
#include <QDesktopServices>
#include <QStyle>

DownloadItem::DownloadItem (const DownloadConfig config, QWidget *parent) : QWidget(parent) {
    service = new Service(this);
    service->startDownload(config.link, config.downloadLocation, config.format, 
                        config.quality, config.conversion, 
                        config.playlist, config.savePlaylistInFolder, config.saveThumbnail);

    titleLabel = new QLabel(this);
    titleLabel->setMinimumWidth(50); 
    
    sizeLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setMaximumWidth(100);
    progressBar->setTextVisible(false);

    QHBoxLayout *layout = new QHBoxLayout(this);
    
    layout->setContentsMargins(6, 0, 6, 0);

    layout->addWidget(titleLabel, 1); 

    layout->addWidget(sizeLabel);
    layout->addWidget(progressBar);

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
}

// Nueva función de ayuda para centralizar los cambios de texto
void DownloadItem::updateTitleText(const QString &text) {
    fullTitle = text;
    updateElidedText();
}

void DownloadItem::downloadStarted() {
      progressBar->setTextVisible(true);
      updateTitleText(tr("Download started"));
      if (progressBar->maximum() == 0) {
            progressBar->setRange(0, 100);
      }
      progressBar->setValue(0);
}

void DownloadItem::downloadFinished(int exit) {
      if (exit == 0) {
            progressBar->setRange(0, 100);
            progressBar->setValue(100);
            if (fullTitle == tr("Download started")) {
                  updateTitleText(tr("Download finished")); // Ya bajado o sin título
            }
            progressBar->setTextVisible(false);
      } else if (exit == 9) {
            updateTitleText(tr("Download stopped"));
      } else {
            updateTitleText(tr("Download failed, error code: %1").arg(QString::number(exit)));
      }
};

void DownloadItem::downloadProgress(int percentage) {
    if (percentage >= progressBar->value() || (progressBar->value() - percentage) > 50) {
        progressBar->setValue(percentage);
    }
}

void DownloadItem::onSizeUpdated(QString cleanSize) {
    downloadedSize = cleanSize;
    sizeLabel->setText(downloadedSize);
}

void DownloadItem::onTitleUpdated(QString title) {
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
}

void DownloadItem::contextMenuEvent(QContextMenuEvent *event) {
      QMenu menu(this);
      
      QAction *openLocation = menu.addAction(tr("Open file location"));
      QIcon folderIcon = QIcon::fromTheme("document-open-folder");
      if (folderIcon.isNull()) {
            folderIcon = style()->standardIcon(QStyle::SP_DirOpenIcon);
      }
      openLocation->setIcon(folderIcon); 

      QAction *cancelAction = menu.addAction(tr("Cancel download"));
      QIcon cancelIcon = QIcon::fromTheme("process-stop");
      if (cancelIcon.isNull()) {
            cancelIcon = style()->standardIcon(QStyle::SP_BrowserStop);
      }
      cancelAction->setIcon(cancelIcon); 

      QAction *selectedAction = menu.exec(event->globalPos());

      if (selectedAction == cancelAction) {
            stopDownload();
      } else if (selectedAction == openLocation) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(downloadLocation));
      }
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