#pragma once
#include "Service.h"
#include "DownloadConfig.h"
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QContextMenuEvent>

class DownloadItem : public QWidget {
Q_OBJECT
public:
    explicit DownloadItem (DownloadConfig config, QWidget *parent = nullptr);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    Service *service;

    QLabel *titleLabel;
    QLabel *sizeLabel;
    QProgressBar *progressBar;

    QString downloadPhase;
    QString downloadLocation;
    QString downloadedSize;

private slots:
    void downloadStarted();
    void downloadFinished(int exit);
    void downloadProgress(int percentage);
    void onTitleUpdated(QString title);
    void downloadPhaseUpdated(QString phase);
    void downloadProcessFailed(QString error);

    void onSizeUpdated(QString cleanSize);

    void stopDownload();
};