#pragma once
#include "ServiceMaintainer.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class MainWindow : public QWidget {
Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    QVBoxLayout *layout;
    QLineEdit *linkBox;
    QPushButton *downloadButton;
    QPushButton *getEngineButton;
    QLabel *statusLabel; // General: descarga y servicio
    ServiceMaintainer *maintainer;
    
private slots:
    void setDownloadReadiness(); // Updates downloadButton status
    void engineDownloading();
    void engineDownloaded(int exit);

};