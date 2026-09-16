#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>
#include <QTimer>

class Service : public QObject {
Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    void startDownload(QString link, QString location, int format, QString quality, QString conversion, bool playlist, bool savePlaylistInFolder, bool saveThumbnail, bool saveSubtitles, bool forceIPv4);
    void stopDownload();
    void fetchThumbnailUrl(const QString &link);
private:
    QProcess *downloadProcess;
    QTimer *stallTimer;
    QTimer *killTimer;
    int partCounter;
    QString playlistStatus;
    double savedSizeMiB = 0.0;
    double currentPartMiB = 0.0;
    bool stallEmitted = false;
    bool killedByTimeout = false;
    QString currentPartFile;

    void resetStallTimer();
private slots:
    void onProcessFinish(int exitCode, QProcess::ExitStatus status);
    void downloadFailed(QProcess::ProcessError error);
    void readOutput();
    void onStallTimeout();
    void onKillTimeout();
signals:
    void titleUpdated(QString title);
    void downloadStarted();
    void downloadFinished(int exit);
    void processFailed(QString error);
    void percentageUpdated(int percentage);
    void phaseUpdated(QString phase);
    void sizeUpdated(QString size);
    void downloadStalled();
    void filePath(QString fullPath);
    void playlistItemUpdated(QString status);
    void thumbnailUrlReceived(QString url);
};