#pragma once
#include <QString>

struct DownloadConfig {
    QString link;
    QString downloadLocation;
    int format;
    QString quality;
    QString conversion;
    bool playlist;
    bool savePlaylistInFolder;
    bool saveThumbnail;
};