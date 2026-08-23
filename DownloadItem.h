#pragma once
#include "Service.h"
#include "DownloadConfig.h"
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QContextMenuEvent>
#include <QResizeEvent>

class DownloadItem : public QWidget {
Q_OBJECT
public:
    explicit DownloadItem (DownloadConfig config, QWidget *parent = nullptr);
    bool isFinished() const { return downloadFinishedState; }
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    Service *service;

    QLabel *titleLabel;
    QLabel *sizeLabel;
    QProgressBar *progressBar;

    QString downloadPhase;
    QString downloadLocation;
    QString downloadedSize;
    QString fullTitle;

    bool downloadFinishedState = false;
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
signals:
    void removeRequested();
    void finishedSignal();
};