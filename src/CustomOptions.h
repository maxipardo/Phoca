/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSettings>

class CustomOptions : public QDialog {
Q_OBJECT
public:
    CustomOptions(QWidget *parent = nullptr);
private:
    QVBoxLayout *m_layout;
    QHBoxLayout *m_bottomLayout;
    QPlainTextEdit *m_textBox;

    QLabel *m_referenceLabel;
    QPushButton *m_saveButton;

private slots:
    void saveOptionsSlot();
};