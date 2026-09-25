/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once
#include <QString>

enum class DownloadError {
    None,
    CookiesNotFound,     // Browser cookies database not found
    AgeVerification,
    Forbidden,           // 403 / IP ban
    NetworkTimeout,
    ProcessCrashed,
    ProcessNotFound,     // yt-dlp not found
    GenericYtdlp,        // Other yt-dlp error
    Unknown
};
