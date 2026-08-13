#include "ServiceMaintainer.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

ServiceMaintainer::ServiceMaintainer(QObject *parent) : QObject(parent) {
  networkManager = new QNetworkAccessManager(this);
}

void ServiceMaintainer::getService(bool nightly) {
  QDir directory;
  if (!directory.exists(serviceDirectory)) {
    if (!directory.mkdir(serviceDirectory)) {
      emit finished(2);
      return;
    }
  }

  if (currentReply) {
      return; // Already downloading
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
#elif defined(Q_OS_MAC)
  urlString += "yt-dlp_macos";
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
}

void ServiceMaintainer::onDownloadFinished() {
  if (currentReply->error() == QNetworkReply::NoError) {
      if (downloadFile) {
          downloadFile->close();
          // Make it executable on Unix systems
#ifndef Q_OS_WIN
          downloadFile->setPermissions(downloadFile->permissions() | QFileDevice::ExeOwner | QFileDevice::ExeUser | QFileDevice::ExeGroup | QFileDevice::ExeOther);
#endif
      }
      qDebug() << "[ServiceMaintainer] yt-dlp descargado.";
      emit finished(0);
  } else {
      qDebug() << "[ServiceMaintainer] Fallo en la descarga. Error:" << currentReply->errorString();
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
  QString appPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN
  return appPath + "/bin/yt-dlp.exe";
#else
  return appPath + "/bin/yt-dlp";
#endif
}