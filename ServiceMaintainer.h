#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QCoreApplication>

class ServiceMaintainer : public QObject {
Q_OBJECT

public:
    ServiceMaintainer(QObject *parent = nullptr);
    bool exists();
    void getService(bool nightly);

private:
    QProcess *downloadProcess;
    QString programLocation {QCoreApplication::applicationDirPath()};
    QString serviceDirectory {programLocation + "/bin"};
    QString serviceFile {serviceDirectory + "/yt-dlp"};
private slots:
    void onProcessFinish(int exitCode, QProcess::ExitStatus status);
signals:
    void started();
    void finished(int exit);
};