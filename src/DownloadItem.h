#pragma once
#include "Service.h"
#include "DownloadConfig.h"
#include "DownloadError.h"
#include "ServiceMaintainer.h"
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QPushButton>


class DownloadItem : public QWidget {
Q_OBJECT
public:
    explicit DownloadItem (const DownloadConfig &config, QWidget *parent = nullptr);
    bool isFinished() const { return m_downloadFinishedState; }
    void changeThumbnailVisibility(bool enabled);
    void updateConfig(const DownloadConfig &config);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Service *m_service;
    ServiceMaintainer *m_maintainer;

    QLabel *m_thumbnailLabel;
    QLabel *m_titleLabel;
    QLabel *m_sizeLabel;
    QProgressBar *m_progressBar;
    QLabel *m_percentageLabel;

    QString m_downloadPhase;
    QString m_downloadLocation;
    QString m_downloadedSize;
    QString m_fullTitle;

    QLabel *m_infoIcon;
    QPushButton *m_restartButton;
    QPushButton *m_discardButton;

    DownloadConfig m_ServiceConfig;

    bool m_downloadFinishedState = false;
    DownloadError m_lastError = DownloadError::None;
    QString m_fullFilePath;

    QPoint m_dragStartPosition;

    QString m_discardText;
    QString m_playlistStatus;
    QString m_toolTipErrors;
    QNetworkAccessManager *m_networkManager;
public slots:
    void stopDownload();
private slots:
    void downloadStarted();
    void downloadFinished(int exit);
    void downloadProgress(int percentage);
    void onTitleUpdated(QString title);
    void downloadPhaseUpdated(QString phase);
    void onError(DownloadError error, QString detail);
    void downloadStalled();

    void onSizeUpdated(QString cleanSize);

    void updateElidedText();
    void updateTitleText(const QString &text);
    void retryDownload();
    void showErrorState();
    void onFullPathUpdated(QString fullPath);
    void openFileLocation();
    void openDownloadLocation();
    void deleteFile();
    void playlistItemUpdated(QString status);
    void onThumbnailUrlReceived(const QString &link);
signals:
    void removeRequested();
    void finishedSignal();
    void retryRequested(DownloadItem *item);
};