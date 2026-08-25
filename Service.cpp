#include "Service.h"
#include "ServiceMaintainer.h"

Service::Service(QObject *parent) {
    downloadProcess = new QProcess(this);

    downloadProcess->setProcessChannelMode(QProcess::MergedChannels);

    /* Connections */
    connect(downloadProcess, &QProcess::started, this, &Service::downloadStarted);
    connect(downloadProcess, &QProcess::finished, this, &Service::onProcessFinish);
    connect(downloadProcess, &QProcess::errorOccurred, this, &Service::downloadFailed);
    
    connect(downloadProcess, &QProcess::readyReadStandardOutput, this, &Service::readOutput);
}

void Service::startDownload(QString link, QString location, int format, QString quality, QString conversion, bool playlist, bool savePlaylistInFolder, bool saveThumbnail) {
    QString executable = ServiceMaintainer::getServiceLocation();
    QStringList arguments;
    QString outputPath;
    playlistStatus = "";
    
    if (playlist) {
        arguments << "--yes-playlist";
        if (savePlaylistInFolder) {
            outputPath = location + "/%(playlist_title)s/%(playlist_index)s - %(title)s.%(ext)s";
        } else {
            outputPath = location + "/%(title)s.%(ext)s";
        }
    } else {
        arguments << "--no-playlist";
        outputPath = location + "/%(title)s.%(ext)s";
    }

    if (saveThumbnail) {
        arguments << "--write-thumbnail";
    }

    arguments << "--newline" << "--no-colors" << "-o" << outputPath;

    // PATH flatpak or .deb
    QString ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    
    // Windows, AppImage, binary
    if (ffmpegPath.isEmpty()) {
        QString appPath = QCoreApplication::applicationDirPath();
        ffmpegPath = QStandardPaths::findExecutable("ffmpeg", QStringList() << appPath + "/bin");
    }

    if (!ffmpegPath.isEmpty()) {
        arguments << "--ffmpeg-location" << ffmpegPath; 
    }

    QString videoFilter = "bv*"; 
    
    if (quality != "0" && !quality.isEmpty()) {
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
            arguments << "-x"; 
            
            if (conversion != "0" && !conversion.isEmpty()) {
                QString targetFormat = conversion;
                targetFormat.remove(".");
                
                arguments << "--audio-format" << targetFormat;
            }
            break;
    }

    if (conversion != "0" && !conversion.isEmpty() && format != 2) {
        
        QString targetFormat = conversion;
        targetFormat.remove("."); 

        arguments << "--merge-output-format" << targetFormat;
        arguments << "--remux-video" << targetFormat;
    }

    arguments << link;
    
    partCounter = 0;
    downloadProcess->start(executable, arguments);
}

void Service::readOutput() {
    QRegularExpression regexDestination("^\\[download\\] Destination:\\s+(.+)$");
    QRegularExpression regexAlready("^\\[download\\]\\s+(.+)\\s+has already been downloaded");
    QRegularExpression regexProgress("^\\[download\\]\\s+(\\d+\\.?\\d*)%(?:\\s+of\\s+~?\\s*([0-9.]+)([a-zA-Z]+))?");
    QRegularExpression regexTitle("^(.+?)(?:\\.f[a-zA-Z0-9]+)?\\.\\w+$");
    QRegularExpression regexPlaylist("^\\[download\\] Downloading (?:video|item) (\\d+) of (\\d+)");

    while (downloadProcess->canReadLine()) {
        QString line = QString::fromLocal8Bit(downloadProcess->readLine()).trimmed();
        if (line.startsWith("ERROR:")) {
            qDebug() << "yt-dlp [ERROR]:" << line;
            emit processFailed(line);
            continue;
        }
        
        QRegularExpressionMatch matchAlready = regexAlready.match(line);
        if (matchAlready.hasMatch()) {
            QString fullPath = matchAlready.captured(1);
            QString fileName = QFileInfo(fullPath).fileName();
            
            QRegularExpressionMatch matchTitle = regexTitle.match(fileName);
            QString cleanTitle;
            
            if (matchTitle.hasMatch()) {
                cleanTitle = matchTitle.captured(1);
            } else {
                cleanTitle = fileName;
            }

            cleanTitle.remove(QRegularExpression("^\\d+\\s*-\\s*"));

            if (!playlistStatus.isEmpty()) {
                cleanTitle = QString("%1 %2").arg(playlistStatus, cleanTitle);
            }
            
            emit titleUpdated(cleanTitle); 
            emit phaseUpdated(tr("Already downloaded"));
            continue;
        }

        // Format (1/50)
        QRegularExpressionMatch matchPlaylist = regexPlaylist.match(line);
        if (matchPlaylist.hasMatch()) {
            QString current = matchPlaylist.captured(1);
            QString total = matchPlaylist.captured(2);
            playlistStatus = QString("(%1/%2)").arg(current, total);
            
            partCounter = 0; 
            continue;
        }

        QRegularExpressionMatch matchDestination = regexDestination.match(line);
        if (matchDestination.hasMatch()) {
            savedSizeMiB += currentPartMiB;
            currentPartMiB = 0.0;
            partCounter++;

            QString fullPath = matchDestination.captured(1);
            QString fileName = QFileInfo(fullPath).fileName();
            
            QRegularExpressionMatch matchTitle = regexTitle.match(fileName);
            QString cleanTitle;
            
            if (matchTitle.hasMatch()) {
                cleanTitle = matchTitle.captured(1);
            } else {
                cleanTitle = fileName;
            }

            cleanTitle.remove(QRegularExpression("^\\d+\\s*-\\s*"));

            if (!playlistStatus.isEmpty()) {
                cleanTitle = QString("%1 %2").arg(playlistStatus, cleanTitle);
            }
            
            emit titleUpdated(cleanTitle);
            
            if (partCounter == 1) {
                emit phaseUpdated(tr("Downloading..."));
            } else if (partCounter > 1) {
                emit phaseUpdated(tr("Downloading audio..."));
            }
            continue;
        }
        
        QRegularExpressionMatch matchProgress = regexProgress.match(line);
        if (matchProgress.hasMatch()) {
            QString textNumber = matchProgress.captured(1);
            int percentage = qRound(textNumber.toDouble());
            
            bool isAudioPart = (partCounter > 1);
            QString currentPhase = isAudioPart ? tr("Downloading audio...") : tr("Downloading...");
            
            if (!matchProgress.captured(2).isEmpty()) {
                double totalSize = matchProgress.captured(2).toDouble();
                QString unit = matchProgress.captured(3); 

                double sizeInMiB = totalSize;
                if (unit == "KiB") sizeInMiB /= 1024.0;
                else if (unit == "GiB") sizeInMiB *= 1024.0;
                currentPartMiB = sizeInMiB;
                
                double downloaded = (textNumber.toDouble() / 100.0) * totalSize;
                
                QString strDownloaded = QString::number(downloaded, 'f', 2);
                QString strTotal = matchProgress.captured(2); 
                
                QString statsText = QString("(%1 %2 / %3 %2)").arg(strDownloaded, unit, strTotal);
                emit phaseUpdated(currentPhase + " " + statsText);
                
                emit sizeUpdated(strDownloaded + " " + unit);
                
            } else {
                emit phaseUpdated(currentPhase);
            }
            
            emit percentageUpdated(percentage);
            
            if (percentage == 100) {
                emit phaseUpdated(tr("Processing..."));
            }
            continue;
        }
    }
}

void Service::onProcessFinish(int exitCode, QProcess::ExitStatus status) {
    if (exitCode == 0) {
        double pesoTotal = savedSizeMiB + currentPartMiB;
        emit sizeUpdated(QString::number(pesoTotal, 'f', 2) + " MiB");
        emit downloadFinished(0);
    } else {
        emit downloadFinished(exitCode);
    }
}

void Service::downloadFailed(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        emit processFailed(tr("Couldn't find yt-dlp"));
    } else {
        emit processFailed(tr("Unexpected error"));
    }
}

void Service::stopDownload() {
    if (downloadProcess->state() == QProcess::Running) {
        downloadProcess->kill();
    }
}