#include "SponsorBlock.h"

const QList<SponsorBlock::CategoryInfo> SponsorBlock::s_categories = {
    {"sponsor",         tr("Sponsor")},
    {"intro",           tr("Intro")},
    {"outro",           tr("Outro")},
    {"selfpromo",       tr("Self-promotion")},
    {"interaction",     tr("Interaction reminder")},
    {"preview",         tr("Preview / Recap")},
    {"music_offtopic",  tr("Non-music section")},
    {"filler",          tr("Filler")}
};

SponsorBlock::SponsorBlock(QWidget *parent) : QDialog(parent) {

    this->setWindowTitle(tr("SponsorBlock options"));
    this->resize(300, 320);
    layout = new QVBoxLayout(this);

    m_label = new QLabel(tr("Categories to skip"), this);
    layout->addWidget(m_label);

    m_listWidget = new QListWidget(this);

    QSettings settings;
    QStringList saved = settings.value("sponsorblock/categories").toStringList();

    for (const auto &cat : s_categories) {
        auto *item = new QListWidgetItem(cat.displayName, m_listWidget);
        item->setData(Qt::UserRole, cat.key);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(saved.contains(cat.key) ? Qt::Checked : Qt::Unchecked);
    }

    layout->addWidget(m_listWidget);

    m_sponsorBlockReference = new QLabel(this);
    m_sponsorBlockReference->setText("<a href=\"https://sponsor.ajay.app//\">About Sponsorblock...</a>");
    m_sponsorBlockReference->setTextFormat(Qt::RichText);
    m_sponsorBlockReference->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_sponsorBlockReference->setOpenExternalLinks(true);
    layout->addWidget(m_sponsorBlockReference);

    m_saveButton = new QPushButton(tr("Save"), this);
    layout->addWidget(m_saveButton);

    connect(m_saveButton, &QPushButton::clicked, this, &SponsorBlock::saveSettings);
}

void SponsorBlock::saveSettings() {
    QStringList selected;

    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selected << item->data(Qt::UserRole).toString();
        }
    }

    QSettings settings;
    settings.setValue("sponsorblock/categories", selected);

    accept();
}

QStringList SponsorBlock::selectedCategories() {
    QSettings settings;
    return settings.value("sponsorblock/categories").toStringList();
}