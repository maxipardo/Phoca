#include "Service.h"
#include "ServiceMaintainer.h"
#include <QFileInfo>


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
    partCounter = 0;
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
    QRegularExpression regexDestination("^\\[download\\] Destination:\\s+(.+)$");
    QRegularExpression regexProgress("^\\[download\\]\\s+(\\d+\\.?\\d*)%");
    QRegularExpression regexMerger("^\\[Merger\\]");
    
    QRegularExpression regexTitle("^(.+?)(?:\\.f[a-zA-Z0-9]+)?\\.\\w+$");

    while (downloadProcess->canReadLine()) {
        QString line = QString::fromLocal8Bit(downloadProcess->readLine()).trimmed();

        QRegularExpressionMatch matchDestination = regexDestination.match(line);
        if (matchDestination.hasMatch()) {
            partCounter++;

            // get title
            QString fullPath = matchDestination.captured(1);
            QString fileName = QFileInfo(fullPath).fileName();
            
            QRegularExpressionMatch matchTitle = regexTitle.match(fileName);
            QString cleanTitle;
            
            if (matchTitle.hasMatch()) {
                cleanTitle = matchTitle.captured(1);
            } else {
                cleanTitle = fileName;
            }

            emit titleUpdated(cleanTitle);

            if (partCounter == 1) {
                emit phaseUpdated("Downloading video...");
            } else if (partCounter == 2) {
                emit phaseUpdated("Downloading audio...");
            }
            continue;
        }

        QRegularExpressionMatch matchProgress = regexProgress.match(line);
        if (matchProgress.hasMatch()) {
            QString textNumber = matchProgress.captured(1);
            int percentage = qRound(textNumber.toDouble());
            
            emit percentageUpdated(percentage);
            continue;
        }

        QRegularExpressionMatch matchMerger = regexMerger.match(line);
        if (matchMerger.hasMatch()) {
            emit phaseUpdated("Merging formats...");
            continue;
        }
    }
}