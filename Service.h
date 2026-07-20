/* #pragma once
#include <QObject>
#include <QProcess>
#include <QDir>
#include <QProcess>
#include <qobject.h>
#include <qtmetamacros.h>

class Service : public QObject {
Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    void startDownload(QString link, QString location);
    void locationChanged(QString location);
private:
    
signals:
    void downloadFinished();
};
*/