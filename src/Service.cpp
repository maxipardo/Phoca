/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "Service.h"
#include "ServiceMaintainer.h"

Service::Service(QObject *parent) : QObject(parent) {
    downloadProcess = new QProcess(this);

    downloadProcess->setProcessChannelMode(QProcess::MergedChannels);

    /* Stall timer */
    stallTimer = new QTimer(this);
    stallTimer->setInterval(5000); // 5 s
    stallTimer->setSingleShot(true);
    connect(stallTimer, &QTimer::timeout, this, &Service::onStallTimeout);

    killTimer = new QTimer(this);
    killTimer->setInterval(20000); // 20 s
    killTimer->setSingleShot(true);
    connect(killTimer, &QTimer::timeout, this, &Service::onKillTimeout);

    /* Connections */
    connect(downloadProcess, &QProcess::started, this, &Service::downloadStarted);
    connect(downloadProcess, &QProcess::finished, this, &Service::onProcessFinish);
    connect(downloadProcess, &QProcess::errorOccurred, this, &Service::downloadFailed);
    
    connect(downloadProcess, &QProcess::readyReadStandardOutput, this, &Service::readOutput);
}

void Service::startDownload(const QString &link, const QString &location, int format, const QString &quality, const QString &conversion, bool playlist, bool savePlaylistInFolder, bool saveThumbnail, bool saveSubtitles, bool forceIPv4, const QString &cookies, const QString &cookiesFile) {
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

    // Print full directory
    arguments << "--no-quiet";
    arguments << "--print" << "after_move:FINALPATH:%(filepath)s";
    arguments << "--no-simulate";

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

    if (saveSubtitles == true) {
        arguments << "--write-subs";
    }

    if (forceIPv4 == true) {
        arguments << "-4";
    }

    if (!cookiesFile.isEmpty()) {
        arguments << "--cookies" << cookiesFile;
    } else if (!cookies.isEmpty()) {
        arguments << "--cookies-from-browser" << cookies;
    }

#ifdef Q_OS_WIN
    arguments << "--windows-filenames";
#endif

    arguments << link;
    
    partCounter = 0;
    savedSizeMiB = 0.0;
    currentPartMiB = 0.0;
    stallEmitted = false;
    killedByTimeout = false;

    qDebug() << "Starting yt-dlp download with command: " << arguments;
    downloadProcess->start(executable, arguments);
    stallTimer->start();
    killTimer->start();
}

void Service::readOutput() {
    static const QRegularExpression regexDestination("^\\[download\\] Destination:\\s+(.+)$");
    static const QRegularExpression regexAlready("^\\[download\\]\\s+(.+)\\s+has already been downloaded");
    static const QRegularExpression regexProgress("^\\[download\\]\\s+(\\d+\\.?\\d*)%(?:\\s+of\\s+~?\\s*([0-9.]+)([a-zA-Z]+))?");
    static const QRegularExpression regexTitle("^(.+?)(?:\\.f[a-zA-Z0-9\\-]+)?\\.\\w+$");
    static const QRegularExpression regexPlaylist("^\\[download\\] Downloading (?:video|item) (\\d+) of (\\d+)");
    static const QRegularExpression finalPath("^FINALPATH:(.+)$");

    while (downloadProcess->canReadLine()) {
        QString line = QString::fromLocal8Bit(downloadProcess->readLine()).trimmed();

        resetStallTimer();

        if (line.startsWith("ERROR:")) {
            qDebug() << "yt-dlp [ERROR]:" << line;

            DownloadError errorType = DownloadError::GenericYtdlp;
            if (line.contains("could not find") && line.contains("cookies")) {
                errorType = DownloadError::CookiesNotFound;
            } else if (line.contains("Forbidden") || line.contains("403")) {
                errorType = DownloadError::Forbidden;
            } else if (line.contains("Sign in to confirm your age")) {
                errorType = DownloadError::AgeVerification;
            }

            emit errorOccurred(errorType, line);
            continue;
        }
        
        QRegularExpressionMatch matchAlready = regexAlready.match(line);
        if (matchAlready.hasMatch()) {
            QString fullPath = matchAlready.captured(1);

            currentPartFile = fullPath;

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
            
            emit playlistItemUpdated(playlistStatus);

            partCounter = 0; 
            continue;
        }

        QRegularExpressionMatch matchDestination = regexDestination.match(line);
        if (matchDestination.hasMatch()) {
            savedSizeMiB += currentPartMiB;
            currentPartMiB = 0.0;
            partCounter++;

            QString fullPath = matchDestination.captured(1);
            currentPartFile = fullPath;
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

        QRegularExpressionMatch matchFinalPath = finalPath.match(line);
        if (matchFinalPath.hasMatch()) {
            QString fullPath = matchFinalPath.captured(1).trimmed();
            
            emit filePath(fullPath);
            continue;
        }
    }
}

void Service::onProcessFinish(int exitCode, QProcess::ExitStatus status) {
    stallTimer->stop();
    killTimer->stop();

    // Drain any remaining buffered output
    readOutput();

    if (killedByTimeout) {
        emit errorOccurred(DownloadError::NetworkTimeout, tr("Download killed after network timeout"));
        emit downloadFinished(-2);
        return;
    }

    if (status == QProcess::CrashExit) {
        emit errorOccurred(DownloadError::ProcessCrashed, tr("yt-dlp process crashed"));
        emit downloadFinished(-1);
        return;
    }

    if (exitCode == 0) {
        double totalMiB = savedSizeMiB + currentPartMiB;
        QString sizeStr;
        if (totalMiB >= 1024.0) {
            sizeStr = QString::number(totalMiB / 1024.0, 'f', 2) + " GiB";
        } else if (totalMiB < 1.0) {
            sizeStr = QString::number(totalMiB * 1024.0, 'f', 2) + " KiB";
        } else {
            sizeStr = QString::number(totalMiB, 'f', 2) + " MiB";
        }
        emit sizeUpdated(sizeStr);
        emit downloadFinished(0);
    } else {
        emit errorOccurred(DownloadError::Unknown, tr("yt-dlp exited with code %1").arg(exitCode));
        emit downloadFinished(exitCode);
    }
}

void Service::downloadFailed(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        emit errorOccurred(DownloadError::ProcessNotFound, tr("Couldn't find yt-dlp"));
    } else {
        emit errorOccurred(DownloadError::Unknown, tr("Process error: %1").arg(error));
    }
}

void Service::stopDownload() {
    stallTimer->stop();
    killTimer->stop();
    if (downloadProcess->state() == QProcess::Running) {
        downloadProcess->terminate();
        if (!downloadProcess->waitForFinished(500)) {
            downloadProcess->kill();
            downloadProcess->waitForFinished(500);
        }
    }

    if (!currentPartFile.isEmpty()) {
        QStringList trash;
        trash << currentPartFile
                << currentPartFile + ".part"
                << currentPartFile + ".ytdl";

        for (const QString &archivo : trash) {
            if (QFile::exists(archivo)) {
                QFile::remove(archivo);
                qDebug() << "Deleted temporal files:" << archivo;
            } else {
                qDebug() << "Could not find:" << archivo;
            }
        }
    }
}

void Service::resetStallTimer() {
    stallEmitted = false;
    stallTimer->start(); // restart
    killTimer->start();
}

void Service::onStallTimeout() {
    if (!stallEmitted && downloadProcess->state() == QProcess::Running) {
        stallEmitted = true;
        emit downloadStalled();
    }
}

void Service::onKillTimeout() {
    if (downloadProcess->state() == QProcess::Running) {
        killedByTimeout = true;
        downloadProcess->kill();
    }
}

void Service::fetchThumbnailUrl(const QString &link) {
    QString executable = ServiceMaintainer::getServiceLocation();
    QProcess *thumbProcess = new QProcess(this);
    
    QStringList args;
    args << "--no-download" << "--no-playlist" << "--print" << "thumbnail" << link;

    connect(thumbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            this, [this, thumbProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            QString url = QString::fromLocal8Bit(thumbProcess->readAllStandardOutput()).trimmed();
            qDebug() << "THUMBNAIL: " << url;
            if (!url.isEmpty()) {
                emit thumbnailUrlReceived(url);
            }
        }
        
        thumbProcess->deleteLater();
    });

    thumbProcess->start(executable, args); 
}