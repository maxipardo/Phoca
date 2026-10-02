#include "SponsorBlock.h"

const QList<SponsorBlock::CategoryInfo> SponsorBlock::s_categories = {
    {"sponsor", QT_TR_NOOP("Sponsor")},
    {"intro", QT_TR_NOOP("Intro")},
    {"outro", QT_TR_NOOP("Outro")},
    {"selfpromo", QT_TR_NOOP("Self-promotion")},
    {"interaction", QT_TR_NOOP("Interaction reminder")},
    {"preview", QT_TR_NOOP("Preview / Recap")},
    {"music_offtopic", QT_TR_NOOP("Non-music section")},
    {"filler", QT_TR_NOOP("Filler")}};

SponsorBlock::SponsorBlock(QWidget *parent) : QDialog(parent) {

  this->setWindowTitle(tr("SponsorBlock options"));
  this->resize(300, 320);
  m_layout = new QVBoxLayout(this);
  m_topLayout = new QHBoxLayout();
  m_bottomLayout = new QHBoxLayout();
  m_layout->addLayout(m_topLayout);

  m_label = new QLabel(tr("Categories to"), this);
  m_actionBox = new QComboBox(this);
  m_actionBox->addItem(tr("Skip"));
  m_actionBox->addItem(tr("Mark"));
  m_topLayout->setSpacing(4);
  m_topLayout->setContentsMargins(0, 0, 0, 0);
  m_topLayout->addWidget(m_label);
  m_topLayout->addWidget(m_actionBox);
  m_topLayout->addStretch();

  m_listWidget = new QListWidget(this);

  QSettings settings;
  QStringList saved = settings.value("sponsorblock/categories").toStringList();
  QString action = settings.value("sponsorblock/action", "skip").toString();
  m_actionBox->setCurrentIndex(action == "mark" ? 1 : 0);

  for (const auto &cat : s_categories) {
    auto *item = new QListWidgetItem(tr(cat.displayName), m_listWidget);
    item->setData(Qt::UserRole, cat.key);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(saved.contains(cat.key) ? Qt::Checked : Qt::Unchecked);
  }

  m_layout->addWidget(m_listWidget);

  m_layout->addLayout(m_bottomLayout);

  m_sponsorBlockReference = new QLabel(this);
  m_sponsorBlockReference->setText(
      "<a href=\"https://sponsor.ajay.app/\">About Sponsorblock...</a>");
  m_sponsorBlockReference->setTextFormat(Qt::RichText);
  m_sponsorBlockReference->setTextInteractionFlags(Qt::TextBrowserInteraction);
  m_sponsorBlockReference->setOpenExternalLinks(true);
  m_bottomLayout->addWidget(m_sponsorBlockReference, 1);

  m_saveButton = new QPushButton(tr("Save"), this);
  m_bottomLayout->addWidget(m_saveButton);

  connect(m_saveButton, &QPushButton::clicked, this,
          &SponsorBlock::saveSettings);
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
  settings.setValue("sponsorblock/action",
                    m_actionBox->currentIndex() == 1 ? "mark" : "skip");

  accept();
}

QStringList SponsorBlock::selectedCategories() {
  QSettings settings;
  return settings.value("sponsorblock/categories").toStringList();
}

QString SponsorBlock::selectedAction() {
  QSettings settings;
  return settings.value("sponsorblock/action", "skip").toString();
}