#pragma once
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QMessageBox>

class About : public QDialog {
Q_OBJECT
public:
    About(QWidget *parent = nullptr);
private:
    QVBoxLayout *layout;
    QHBoxLayout *sourceLayout;
};