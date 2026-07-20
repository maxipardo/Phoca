#include "ServiceMaintainer.h"

ServiceMaintainer::ServiceMaintainer(QObject *parent) {
    downloadProcess = new QProcess(this);

    connect(downloadProcess, &QProcess::started, this, &ServiceMaintainer::started);
    connect(downloadProcess, &QProcess::finished, this, &ServiceMaintainer::finished);
    connect(downloadProcess, qOverload<int,QProcess::ExitStatus>(&QProcess::finished), 
           this, &ServiceMaintainer::onProcessFinish);
}

void ServiceMaintainer::getService(bool nightly) {
    QDir directory;
    if (!directory.exists(serviceDirectory)) {
        if (!directory.mkdir(serviceDirectory)) {
            emit finished(2);
            return;
        }
    }
    QString command;
    if (nightly) {
        command = "wget -O \"" + serviceFile + "\" "
                          "https://github.com/yt-dlp/yt-dlp-nightly-builds/releases/latest/download/yt-dlp && "
                          "chmod a+rx \"" + serviceFile + "\"";
    } else {
        command = "wget -O \"" + serviceFile + "\" "
                          "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp && "
                          "chmod a+rx \"" + serviceFile + "\"";
    }

    downloadProcess->start("bash", {"-c", command});
}

void ServiceMaintainer::onProcessFinish(int exitCode, QProcess::ExitStatus status) {
    // If process returned 0 (success)
    if (exitCode == 0 && status == QProcess::NormalExit) {
        qDebug() << "[ServiceMaintainer] yt-dlp descargado. (wget)";
        emit finished(0);
    } else {
        qDebug() << "[ServiceMaintainer] Fallo en la descarga (wget). Exit code:" << exitCode;
        emit finished(1);
    }
}

bool ServiceMaintainer::exists() {
    return QFile::exists(serviceFile); // Indica si existe el archivo yt-dlp
}