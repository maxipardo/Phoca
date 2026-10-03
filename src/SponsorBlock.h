#pragma once
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QStringList>
#include <QComboBox>

class SponsorBlock : public QDialog {
Q_OBJECT

public:
    SponsorBlock(QWidget *parent = nullptr);

    static QStringList selectedCategories();
    static QString selectedAction();

private:
    QHBoxLayout *m_topLayout;
    QVBoxLayout *m_layout;
    QHBoxLayout *m_bottomLayout;
    QLabel *m_label;
    QComboBox *m_actionBox;
    
    QListWidget *m_listWidget;
    QLabel *m_sponsorBlockReference;
    QPushButton *m_selectAllButton;
    QPushButton *m_deselectAllButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;

    struct CategoryInfo {
        QString key;
        const char *displayName;
    };

    static const QList<CategoryInfo> s_categories;

private slots:
    void saveSettings();
    void selectAll();
    void deselectAll();
};