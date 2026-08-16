#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>

class Service : public QObject {
Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    void startDownload(QString link, QString location, int format, QString quality, QString conversion, bool playlist, bool savePlaylistInFolder);
private:
    QProcess *downloadProcess;
    int partCounter;
    QString playlistStatus;
private slots:
    void onProcessFinish(int exitCode, QProcess::ExitStatus status);
    void downloadFailed(QProcess::ProcessError error);
    void readOutput();
signals:
    void titleUpdated(QString title);
    void downloadStarted();
    void downloadFinished(int exit);
    void processFailed(QString error);
    void percentageUpdated(int percentage);
    void phaseUpdated(QString phase);
};