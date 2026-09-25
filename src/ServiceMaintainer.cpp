/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ServiceMaintainer.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QStandardPaths>

ServiceMaintainer::ServiceMaintainer(QObject *parent) : QObject(parent) {
  networkManager = new QNetworkAccessManager(this);

#ifdef FLATPAK_BUILD
  // Flatpak static directory given in manifest
  serviceFile = "/app/bin/yt-dlp";
#else
  QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Phoca";
  serviceDirectory = basePath + "/bin";

  #ifdef Q_OS_WIN
    serviceFile = serviceDirectory + "/yt-dlp.exe";
  #else
    serviceFile = serviceDirectory + "/yt-dlp";
  #endif
#endif
}

void ServiceMaintainer::getService(bool nightly) {
#ifdef FLATPAK_BUILD
Q_UNUSED(nightly);
  emit finished(0);
  return;
#else
  QDir directory;
  if (!directory.exists(serviceDirectory)) {
    if (!directory.mkpath(serviceDirectory)) {
      emit finished(2);
      return;
    }
  }

  if (currentReply) {
      return; // Already downloading
  }

  if (downloadFile) {
      downloadFile->deleteLater();
      downloadFile = nullptr;
  }

  downloadFile = new QFile(serviceFile, this);
  if (!downloadFile->open(QIODevice::WriteOnly)) {
      delete downloadFile;
      downloadFile = nullptr;
      emit finished(2);
      return;
  }

  QString urlString;
  if (nightly) {
    urlString = "https://github.com/yt-dlp/yt-dlp-nightly-builds/releases/latest/download/";
  } else {
    urlString = "https://github.com/yt-dlp/yt-dlp/releases/latest/download/";
  }

#ifdef Q_OS_WIN
  urlString += "yt-dlp.exe";
#else
  urlString += "yt-dlp";
#endif

  QNetworkRequest request((QUrl(urlString)));
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  currentReply = networkManager->get(request);
  
  emit started();

  connect(currentReply, &QNetworkReply::readyRead, this, [this]() {
      if (downloadFile) {
          downloadFile->write(currentReply->readAll());
      }
  });

  connect(currentReply, &QNetworkReply::finished, this, &ServiceMaintainer::onDownloadFinished);
#endif
}

void ServiceMaintainer::onDownloadFinished() {
  if (currentReply->error() == QNetworkReply::NoError) {
      if (downloadFile) {
          downloadFile->close();
#ifndef Q_OS_WIN
          downloadFile->setPermissions(downloadFile->permissions() | QFileDevice::ExeOwner | QFileDevice::ExeUser | QFileDevice::ExeGroup | QFileDevice::ExeOther);
#endif
      }
      qDebug() << "[ServiceMaintainer] yt-dlp downloaded at:" << serviceFile;
      emit finished(0);
  } else {
      qDebug() << "[ServiceMaintainer] yt-dlp Download failed. Error:" << currentReply->errorString();
      if (downloadFile) {
          downloadFile->close();
          downloadFile->remove();
      }
      emit finished(1);
  }

  if (downloadFile) {
      downloadFile->deleteLater();
      downloadFile = nullptr;
  }
  
  currentReply->deleteLater();
  currentReply = nullptr;
}

bool ServiceMaintainer::exists() {
  return QFile::exists(serviceFile);
}

QString ServiceMaintainer::getServiceLocation() {
#ifdef FLATPAK_BUILD
  return QStringLiteral("/app/bin/yt-dlp");
#else
  QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Phoca";
  QString dir = basePath + "/bin";
#ifdef Q_OS_WIN
  return dir + "/yt-dlp.exe";
#else
  return dir + "/yt-dlp";
#endif
#endif
}