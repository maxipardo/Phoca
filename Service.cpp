#include "Service.h"
#include "ServiceMaintainer.h"

Service::Service(QObject *parent) {
    downloadProcess = new QProcess(this);

    /* Connections */
    connect(downloadProcess, &QProcess::started, this, &Service::downloadStarted);
    connect(downloadProcess, &QProcess::finished, this, &Service::onProcessFinish);
    connect(downloadProcess, &QProcess::errorOccurred, this, &Service::downloadFailed);
    
    connect(downloadProcess, &QProcess::readyReadStandardOutput, this, &Service::readOutput);
};

void Service::startDownload(QString link, QString location) {
    QString executable = ServiceMaintainer::getServiceLocation();
    QStringList arguments;
    QString outputPath = location + "/%(title)s.%(ext)s";
    arguments << "--newline" << "--no-colors" << "-o" << outputPath << link;

    downloadProcess->start(executable, arguments);
}

void Service::onProcessFinish(int exitCode, QProcess::ExitStatus status) {
    if (exitCode == 0) {
        emit downloadFinished(0);
    } else if (exitCode == 1) {
        emit downloadFinished(1);
    }
}

void Service::downloadFailed(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        emit processFailed("Couldn't find yt-dlp");
    } else {
        emit processFailed("Unexpected error");
    }
}

void Service::readOutput() {
    while (downloadProcess->canReadLine()) {
        QString line = QString::fromLocal8Bit(downloadProcess->readLine()).trimmed();
        QRegularExpression regexProgress("^\\[download\\]\\s+(\\d+\\.?\\d*)%");
        QRegularExpressionMatch matchProgress = regexProgress.match(line);

        if (matchProgress.hasMatch()) {
            QString textNumber = matchProgress.captured(1);
            
            int percentage = qRound(textNumber.toDouble());
            
            emit percentageUpdated(percentage);
        }
    }
}