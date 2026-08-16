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

void Service::startDownload(QString link, QString location, int format, QString quality, QString conversion) {
    QString executable = ServiceMaintainer::getServiceLocation();
    QStringList arguments;
    QString outputPath = location + "/%(title)s.%(ext)s";
    
    arguments << "--newline" << "--no-colors" << "-o" << outputPath;

    QString videoFilter = "bv*"; 
    
    if (quality != "Best" && !quality.isEmpty()) {
        QString height = quality;
        height.remove("p");
        videoFilter = "bv*[height<=" + height + "]";
    }

    switch (format) {
        case 0: // both
            arguments << "-f" << videoFilter + "+ba/b";
            break;
            
        case 1: // video only
            arguments << "-f" << videoFilter;
            break;
            
        case 2: // audio only
            arguments << "-x" << "--audio-format" << "mp3";
            break;
    }

    if (conversion != "Original" && !conversion.isEmpty() && format != 2) {
        
        QString targetFormat = conversion;
        targetFormat.remove("."); 

        arguments << "--merge-output-format" << targetFormat;
        arguments << "--remux-video" << targetFormat;
    }

    arguments << link;
    
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
                emit phaseUpdated("Downloading...");
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