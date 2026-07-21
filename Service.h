#pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QStringList>

class Service : public QObject {
Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    void startDownload(QString link, QString location);
private:
    QProcess *downloadProcess;
signals:
    void downloadStarted();
    void downloadFinished();
};