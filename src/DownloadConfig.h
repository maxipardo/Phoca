/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once
#include <QString>
#include <QStringList>

struct DownloadConfig {
    QString link;
    QString downloadLocation;
    int format = 0;
    QString quality;
    QString conversion;
    bool playlist = false;
    bool savePlaylistInFolder = true;
    bool saveThumbnail = false;
    bool saveSubtitles = false;
    bool forceIPv4 = true;
    bool thumbnailVisibility = true;
    QString cookies = "";
    QString cookiesFile = "";
    bool embedMetadata = false;
    QString customOptions = "";

    // Adjusts config when customOptions contain flags that override UI values.
    void reconcileCustomOptions() {
        if (customOptions.isEmpty()) return;

        const QStringList tokens = customOptions.split(' ', Qt::SkipEmptyParts);

        if (tokens.contains("--no-write-thumbnail")) {
            saveThumbnail = false;
        } else if (tokens.contains("--write-thumbnail")) {
            saveThumbnail = true;
        }

        if (tokens.contains("--no-write-subs")) {
            saveSubtitles = false;
        } else if (tokens.contains("--write-subs")) {
            saveSubtitles = true;
        }

        if (tokens.contains("--no-embed-metadata")) {
            embedMetadata = false;
        } else if (tokens.contains("--embed-metadata")) {
            embedMetadata = true;
        }

        if (tokens.contains("--force-ipv6") || tokens.contains("-6")) {
            forceIPv4 = false;
        } else if (tokens.contains("--force-ipv4") || tokens.contains("-4")) {
            forceIPv4 = true;
        }
    }
};