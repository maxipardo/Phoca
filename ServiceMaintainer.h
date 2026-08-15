#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStandardPaths>

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
  
  QString serviceDirectory;
  QString serviceFile;

private slots:
  void onDownloadFinished();
signals:
  void started();
  void finished(int exit);
};