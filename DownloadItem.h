#pragma once
#include "Service.h"
#include "DownloadConfig.h"
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QPushButton>


class DownloadItem : public QWidget {
Q_OBJECT
public:
    explicit DownloadItem (DownloadConfig config, QWidget *parent = nullptr);
    bool isFinished() const { return downloadFinishedState; }
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Service *service;

    QLabel *titleLabel;
    QLabel *sizeLabel;
    QProgressBar *progressBar;
    QLabel *percentageLabel;

    QString downloadPhase;
    QString downloadLocation;
    QString downloadedSize;
    QString fullTitle;

    QLabel *infoIcon;
    QPushButton *restartButton;
    QPushButton *discardButton;

    DownloadConfig ServiceConfig;

    bool downloadFinishedState = false;
    QString fullFilePath;

    QPoint dragStartPosition;

    QString discardText;
    QString playlistStatus;
    QString toolTipErrors;
public slots:
    void stopDownload();
private slots:
    void downloadStarted();
    void downloadFinished(int exit);
    void downloadProgress(int percentage);
    void onTitleUpdated(QString title);
    void downloadPhaseUpdated(QString phase);
    void downloadProcessFailed(QString error);

    void onSizeUpdated(QString cleanSize);

    void updateElidedText();
    void updateTitleText(const QString &text);
    void retryDownload();
    void onFullPathUpdated(QString fullPath);
    void openFileLocation();
    void openDownloadLocation();
    void deleteFile();
    void playlistItemUpdated(QString status);
signals:
    void removeRequested();
    void finishedSignal();
};