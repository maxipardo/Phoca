#include "DownloadItem.h"
#include <QLayout>
#include <QMenu>

DownloadItem::DownloadItem (const DownloadConfig config, QWidget *parent) : QWidget(parent) {
    service = new Service(this);
    service->startDownload(config.link, config.downloadLocation, config.format, 
                        config.quality, config.conversion, 
                        config.playlist, config.savePlaylistInFolder, config.saveThumbnail);

    titleLabel = new QLabel(this);
    sizeLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setMaximumWidth(100);
    progressBar->setTextVisible(false);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(titleLabel);
    QSpacerItem *spacer = new QSpacerItem(40, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addItem(spacer);
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


void DownloadItem::downloadStarted() {
      progressBar->setTextVisible(true);
      titleLabel->setText(tr("Download started"));
      if (progressBar->maximum() == 0) {
            progressBar->setRange(0, 100);
      }
      progressBar->setValue(0);
}

void DownloadItem::downloadFinished(int exit) {
      if (exit == 0) {
            progressBar->setRange(0, 100);
            progressBar->setValue(100);
            if (titleLabel->text() == tr("Download started")) {
                  titleLabel->setText("Download finished"); // Already downloaded or couldn't get title
                  progressBar->setTextVisible(false);
            }
      } else if (exit == 9) {
            titleLabel->setText(tr("Download stopped"));
      } else {
            titleLabel->setText(tr("Download failed, error code: %1").arg(QString::number(exit)));
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
      titleLabel->setText(title);
};
void DownloadItem::downloadPhaseUpdated(QString phase) {
      downloadPhase = phase;

      if (phase == tr("Processing...")) {
            progressBar->setRange(0, 0);
      } else {
            progressBar->setRange(0, 100);
      }
};

void DownloadItem::downloadProcessFailed(QString error) {
      titleLabel->setText(tr("Process failed: %1").arg(error));
}

void DownloadItem::stopDownload() {
      service->stopDownload();
}

void DownloadItem::contextMenuEvent(QContextMenuEvent *event) {
      QMenu menu(this);
      
      QAction *cancelAction = menu.addAction(tr("Cancel download"));
      QAction *accionSeleccionada = menu.exec(event->globalPos());

      if (accionSeleccionada == cancelAction) {
            stopDownload();
      }
}