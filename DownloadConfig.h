#pragma once
#include <QString>

struct DownloadConfig {
    QString link;
    QString downloadLocation;
    int format = 0;
    QString quality;
    QString conversion;
    bool playlist = false;
    bool savePlaylistInFolder = true;
    bool saveThumbnail = false;
};