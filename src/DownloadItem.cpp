#include "DownloadItem.h"
#include "DownloadError.h"
#include "ServiceMaintainer.h"
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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#ifdef Q_OS_LINUX
#include <QDBusMessage>
#include <QDBusConnection>
#endif

DownloadItem::DownloadItem (const DownloadConfig &config, QWidget *parent) : QWidget(parent) {
    
    m_maintainer = new ServiceMaintainer(this);
    m_ServiceConfig = config;
    m_fullFilePath = ""; // Set on download finish
    m_discardText = (tr("Cancel download\tDel"));
    m_playlistStatus = "";
    m_toolTipErrors = "";
    m_networkManager = new QNetworkAccessManager(this);
    
    m_service = new Service(this);
    
    m_titleLabel = new QLabel(this);
    m_titleLabel->setMinimumWidth(50); 
    m_titleLabel->setContentsMargins(4, 0, 0, 0);
    
    m_thumbnailLabel = new QLabel(this);
    m_thumbnailLabel->setVisible(config.thumbnailVisibility);
    m_thumbnailLabel->setFixedSize(0, 0);
    
    m_sizeLabel = new QLabel(this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setTextVisible(false);
    
    m_percentageLabel = new QLabel(this);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    QHBoxLayout *failLayout = new QHBoxLayout();
    
    layout->setContentsMargins(2, 0, 6, 0);
    failLayout->setSpacing(0);
    
    m_infoIcon = new QLabel(this);
    
    m_restartButton = new QPushButton(this);
    m_discardButton = new QPushButton(this);
    
    m_restartButton->setIcon(QIcon::fromTheme("view-refresh"));
    m_discardButton->setIcon(QIcon::fromTheme("window-close"));
    
    #ifdef Q_OS_WIN
    m_restartButton->setIconSize(QSize(12, 12));
    m_discardButton->setIconSize(QSize(12, 12));
    m_infoIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(12, 12));
    #else
    m_restartButton->setIconSize(QSize(14, 14));
    m_discardButton->setIconSize(QSize(14, 14));
    m_infoIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(16, 16));
    #endif
    
    m_infoIcon->setVisible(false);
    m_restartButton->setVisible(false);
    m_discardButton->setVisible(false);
    
    m_restartButton->setText(tr("Retry"));
    m_discardButton->setText(tr("Discard"));
    
    layout->addWidget(m_thumbnailLabel);
    layout->addWidget(m_titleLabel, 3); 
    layout->addWidget(m_infoIcon);
    layout->addLayout(failLayout);
    failLayout->addWidget(m_restartButton, 1);
    failLayout->addWidget(m_discardButton, 1);
    layout->addWidget(m_sizeLabel);
    layout->addWidget(m_progressBar, 1);
    layout->addWidget(m_percentageLabel);
    
    m_percentageLabel->setVisible(false);
    m_percentageLabel->setMargin(4);
    
    m_downloadLocation = config.downloadLocation;
    m_downloadPhase = "";
    
    /* Service */
    connect(m_service, &Service::downloadStarted, this, &DownloadItem::downloadStarted);
    connect(m_service, &Service::downloadFinished, this, &DownloadItem::downloadFinished);
    connect(m_service, &Service::errorOccurred, this, &DownloadItem::onError);
    connect(m_service, &Service::percentageUpdated, this, &DownloadItem::downloadProgress);
    connect(m_service, &Service::phaseUpdated, this, &DownloadItem::downloadPhaseUpdated);
    connect(m_service, &Service::titleUpdated, this, &DownloadItem::onTitleUpdated);
    connect(m_service, &Service::sizeUpdated, this, &DownloadItem::onSizeUpdated);
    connect(m_service, &Service::downloadStalled, this, &DownloadItem::downloadStalled);
    connect(m_service, &Service::filePath, this, &DownloadItem::onFullPathUpdated);
    connect(m_service, &Service::playlistItemUpdated, this, &DownloadItem::playlistItemUpdated);

    connect(m_restartButton, &QPushButton::clicked, this, &DownloadItem::retryDownload);
    connect(m_discardButton, &QPushButton::clicked, this, &DownloadItem::stopDownload);

    connect(m_service, &Service::thumbnailUrlReceived, this, &DownloadItem::onThumbnailUrlReceived);

    if (config.playlist == false) {
        m_service->fetchThumbnailUrl(config.link);
    }

    m_service->startDownload(config.link, config.downloadLocation, config.format, 
                            config.quality, config.conversion, 
                            config.playlist, config.savePlaylistInFolder,
                            config.saveThumbnail, config.saveSubtitles, config.forceIPv4, config.cookies);
}                  
// Service
void DownloadItem::downloadStarted() {
    updateTitleText(tr("Download started"));
    if (m_progressBar->maximum() == 0) {
        m_progressBar->setRange(0, 100);
    }
    m_progressBar->setValue(0);
    m_percentageLabel->setText("0%");
    m_percentageLabel->setVisible(true);
    updateElidedText();
}

void DownloadItem::downloadFinished(int exit) {
    m_progressBar->setRange(0, 100);
    m_discardText = (tr("Discard download\tDel"));
    if (exit == 0) {
        m_progressBar->setValue(100);
        if (m_fullTitle == tr("Download started")) {
            if (m_downloadPhase == tr("Already downloaded")) {
                updateTitleText(tr("Already downloaded")); 
            } else {
                updateTitleText(tr("Download finished"));
            }
        }
        
        m_percentageLabel->setVisible(false);
        QTimer::singleShot(0, this, &DownloadItem::updateElidedText);
        m_downloadFinishedState = true;
        emit finishedSignal();
        return;
    } else if (exit == 9) {
        updateTitleText(tr("Download stopped"));
    } else {
        // Fallback title if onError was never called
        if (m_fullTitle == tr("Download started") || m_fullTitle.isEmpty()) {
            updateTitleText(tr("Download failed"));
        }
        showErrorState();
    }
    m_downloadFinishedState = true;
    emit finishedSignal();
}

void DownloadItem::downloadProgress(int percentage) {
    if (percentage >= m_progressBar->value() || (m_progressBar->value() - percentage) > 50) {
        m_progressBar->setValue(percentage);
    }
    m_percentageLabel->setVisible(true);
    m_percentageLabel->setText(QString::number(percentage) + "%");
    updateElidedText();
}

void DownloadItem::onSizeUpdated(QString cleanSize) {
    m_downloadedSize = cleanSize;
    m_sizeLabel->setText(m_downloadedSize);
}

void DownloadItem::onTitleUpdated(QString title) {
    title.remove(QRegularExpression("\\.f\\d+\\.[a-zA-Z0-9]+$"));
    updateTitleText(title);
}

void DownloadItem::downloadPhaseUpdated(QString phase) {
    m_downloadPhase = phase;
    
    if (phase == tr("Processing...")) {
        m_progressBar->setRange(0, 0);
    } else {
        m_progressBar->setRange(0, 100);
    }
}

void DownloadItem::onError(DownloadError error, QString detail) {
    if (m_lastError == DownloadError::None || (error != DownloadError::Unknown && error != DownloadError::GenericYtdlp)) {
        m_lastError = error;

        switch (error) {
        case DownloadError::CookiesNotFound:
            updateTitleText(tr("Cookies not found for the selected browser"));
            break;
        case DownloadError::AgeVerification:
            updateTitleText(tr("Sign in to confirm your age"));
            break;
        case DownloadError::Forbidden:
            updateTitleText(tr("Access denied (403)"));
            #if defined(Q_OS_LINUX) && !defined(FLATPAK_BUILD)
                m_restartButton->setText(tr("Update yt-dlp"));
            #endif
            break;
        case DownloadError::NetworkTimeout:
            updateTitleText(tr("Download failed: network timeout"));
            break;
        case DownloadError::ProcessCrashed:
            updateTitleText(tr("Download failed: process crashed"));
            break;
        case DownloadError::ProcessNotFound:
            updateTitleText(tr("yt-dlp not found"));
            break;
        case DownloadError::GenericYtdlp:
        case DownloadError::Unknown:
        default:
            updateTitleText(tr("Download failed"));
            break;
        }

    }
    
    if (!detail.isEmpty()) {
        if (!m_playlistStatus.isEmpty()) {
            m_toolTipErrors.append(m_playlistStatus + " ");
        }
        m_toolTipErrors.append(detail + "\n");
        m_infoIcon->setToolTip(m_toolTipErrors);
        m_infoIcon->setVisible(true);
    }
}

void DownloadItem::showErrorState() {
    m_restartButton->setVisible(true);
    m_discardButton->setVisible(true);
    m_sizeLabel->setVisible(false);
    m_progressBar->setVisible(false);
    m_percentageLabel->setVisible(false);
}

void DownloadItem::downloadStalled() {
    m_progressBar->setRange(0, 0);
    m_percentageLabel->setVisible(false);
    QTimer::singleShot(0, this, &DownloadItem::updateElidedText);
}

void DownloadItem::stopDownload() {
    m_service->stopDownload();
    emit removeRequested();
}

// Text changes centralized
void DownloadItem::updateTitleText(const QString &text) {
    m_fullTitle = text;
    updateElidedText();
}

void DownloadItem::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); 
    updateElidedText();          
}

void DownloadItem::updateElidedText() {
    if (m_fullTitle.isEmpty()) return;
    
    QFontMetrics metrics(m_titleLabel->font());
    QString elidedTitle = metrics.elidedText(m_fullTitle, Qt::ElideRight, m_titleLabel->width());
    
    m_titleLabel->setText(elidedTitle);
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
    
    QAction *cancelAction = menu->addAction(m_discardText);
    QIcon cancelIcon = QIcon::fromTheme("process-stop");
    if (cancelIcon.isNull()) {
        if (isDarkMode) {
            cancelIcon = QIcon(":/cancel_light.svg");
        } else {
            cancelIcon = QIcon(":/cancel_dark.svg");
        }
    }
    cancelAction->setIcon(cancelIcon); 
    
    QAction *deleteFileAction = nullptr;
    QIcon deleteIcon;
    if (!m_fullFilePath.isEmpty() && m_ServiceConfig.playlist == false) {
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
        connect(deleteFileAction, &QAction::triggered, this, &DownloadItem::deleteFile);
    }
    
    connect(cancelAction, &QAction::triggered, this, &DownloadItem::stopDownload);
    if (!m_fullFilePath.isEmpty()) {
        connect(openLocation, &QAction::triggered, this, &DownloadItem::openFileLocation);
    } else {
        connect(openLocation, &QAction::triggered, this, &DownloadItem::openDownloadLocation);
    }
    
    // Asynchronous menu
    menu->popup(event->globalPos());
}

void DownloadItem::retryDownload() {
    #if defined(Q_OS_LINUX) && !defined(FLATPAK_BUILD)
    if (m_lastError == DownloadError::Forbidden) {
        m_maintainer->getService(true);
        m_lastError = DownloadError::None;
        m_restartButton->setText(tr("Retry"));
        return;
    }
    #endif
    
    m_restartButton->setText(tr("Retry"));
    m_lastError = DownloadError::None;
    m_downloadFinishedState = false;
    m_toolTipErrors = "";
    m_infoIcon->setVisible(false);
    m_restartButton->setVisible(false);
    m_discardButton->setVisible(false);
    m_sizeLabel->setVisible(true);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_percentageLabel->setVisible(true);
    
    m_service->startDownload(m_ServiceConfig.link, m_ServiceConfig.downloadLocation, m_ServiceConfig.format, 
        m_ServiceConfig.quality, m_ServiceConfig.conversion, 
        m_ServiceConfig.playlist, m_ServiceConfig.savePlaylistInFolder, 
        m_ServiceConfig.saveThumbnail, m_ServiceConfig.saveSubtitles, m_ServiceConfig.forceIPv4, m_ServiceConfig.cookies);
}

void DownloadItem::onFullPathUpdated(QString fullPath) {
    m_fullFilePath = fullPath;
    qDebug() << fullPath;
}

void DownloadItem::openDownloadLocation() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadLocation));
}

void DownloadItem::openFileLocation() {
    #if defined(Q_OS_WIN)
    // Windows
    QString windowsPath = QDir::toNativeSeparators(m_fullFilePath);
    
    QStringList args;
    args << "/select," << windowsPath;
    
    QProcess::startDetached("explorer.exe", args);
    
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
        uris << QUrl::fromLocalFile(m_fullFilePath).toString();
        msg << uris << QString("");
        
        QDBusConnection::sessionBus().send(msg);
    } else {
        // If not available
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadLocation));
    }
    #else
    // Other
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadLocation));
    #endif
}

void DownloadItem::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !m_fullFilePath.isEmpty()) {
        openFileLocation();
        
    }
    QWidget::mouseDoubleClickEvent(event);
}

void DownloadItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    // Continues
    QWidget::mousePressEvent(event);
}

void DownloadItem::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }
    
    // If download hasn't finished
    if (m_fullFilePath.isEmpty()) {
        return; 
    }
    
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(m_fullFilePath);
    mimeData->setUrls(urls);
    
    drag->setMimeData(mimeData);
    
    drag->exec(Qt::CopyAction);
}

void DownloadItem::deleteFile() {
    QString cleanPath = m_fullFilePath.trimmed();
    
    if (!cleanPath.isEmpty()) {
        QFile file(cleanPath);

        if (file.exists()) {
            if (file.moveToTrash()) {
                qDebug() << "File successfully moved to trash:" << cleanPath;
                m_fullFilePath.clear();
                emit removeRequested();
            } else {
                qDebug() << "Couldn't move to trash, trying hard delete...";
                if (file.remove()) {
                    qDebug() << "File successfully deleted (hard):" << cleanPath;
                    m_fullFilePath.clear();
                    emit removeRequested();
                } else {
                    qDebug() << "Error: Couldn't delete file.";
                    updateTitleText(tr("Couldn't delete file"));
                }
            }
        } else {
            qDebug() << "File does not exist.";
            updateTitleText(tr("Couldn't delete file"));
        }
    }
}

void DownloadItem::playlistItemUpdated(QString status) {
    m_playlistStatus = status;
}

void DownloadItem::onThumbnailUrlReceived(const QString &link) {
    if (link.isEmpty()) {
        return;
    }
    
    QUrl url(link);
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater(); 
        
        if (reply->error() == QNetworkReply::NoError) {
            
            QByteArray imageData = reply->readAll(); 
            
            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                int targetHeight = qMin(this->height(), 120);
                QPixmap scaledPixmap = pixmap.scaledToHeight(targetHeight, Qt::SmoothTransformation);
                
                m_thumbnailLabel->setFixedSize(scaledPixmap.size());
                m_thumbnailLabel->setPixmap(scaledPixmap);
            }
        } else {
            qDebug() << "Failed downloading thumbnail:" << reply->errorString();
        }
    });
}

void DownloadItem::changeThumbnailVisibility(bool enabled) {
    m_thumbnailLabel->setVisible(enabled);
    QTimer::singleShot(0, this, &DownloadItem::updateElidedText);
}