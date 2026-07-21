#include "Service.h"
#include "ServiceMaintainer.h"

Service::Service(QObject *parent) {
    downloadProcess = new QProcess(this);

    /* Connects */
};

void Service::startDownload(QString link, QString location) {
    QString executable = ServiceMaintainer::getServiceLocation();
    QStringList arguments;
    QString outputPath = location + "/%(title)s.%(ext)s";
    arguments << "-o" << outputPath << link;

    downloadProcess->start(executable, arguments);
}