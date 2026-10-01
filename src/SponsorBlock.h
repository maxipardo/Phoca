#pragma once
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QStringList>

class SponsorBlock : public QDialog {
Q_OBJECT

public:
    SponsorBlock(QWidget *parent = nullptr);

    static QStringList selectedCategories();

private:
    QVBoxLayout *layout;
    QLabel *m_label;
    QListWidget *m_listWidget;
    QLabel *m_sponsorBlockReference;
    QPushButton *m_saveButton;

    struct CategoryInfo {
        QString key;
        QString displayName;
    };

    static const QList<CategoryInfo> s_categories;

private slots:
    void saveSettings();
};