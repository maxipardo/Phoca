#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegularExpression>

class Service : public QObject {
Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    void startDownload(QString link, QString location);
private:
    QProcess *downloadProcess;
    int partCounter;
private slots:
    void onProcessFinish(int exitCode, QProcess::ExitStatus status);
    void downloadFailed(QProcess::ProcessError error);
    void readOutput();
signals:
    void downloadStarted();
    void downloadFinished(int exit);
    void processFailed(QString error);
    void percentageUpdated(int percentage);
    void phaseUpdated(QString phase);
};