#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class ServiceMaintainer : public QObject {
  Q_OBJECT

public:
  ServiceMaintainer(QObject *parent = nullptr);
  bool exists();
  void getService(bool nightly);
  static QString getServiceLocation();

private:
  QNetworkAccessManager *networkManager;
  QNetworkReply *currentReply = nullptr;
  QFile *downloadFile = nullptr;
  QString programLocation{QCoreApplication::applicationDirPath()};
  QString serviceDirectory{programLocation + "/bin"};
#ifdef Q_OS_WIN
  QString serviceFile{serviceDirectory + "/yt-dlp.exe"};
#else
  QString serviceFile{serviceDirectory + "/yt-dlp"};
#endif
private slots:
  void onDownloadFinished();
signals:
  void started();
  void finished(int exit);
};
