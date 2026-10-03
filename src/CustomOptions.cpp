#include "CustomOptions.h"
#include <qpushbutton.h>

CustomOptions::CustomOptions(QWidget *parent) : QDialog(parent) {

    this->resize(380, 200);
    this->setWindowTitle(tr("Custom options"));
    
    m_layout = new QVBoxLayout(this);
    m_bottomLayout = new QHBoxLayout();
    m_textBox = new QPlainTextEdit(this);
    m_textBox->setPlaceholderText(tr("Insert custom yt-dlp options..."));

    m_referenceLabel = new QLabel(this);
    m_referenceLabel->setText("<a href=\"https://github.com/yt-dlp/yt-dlp#usage-and-options\">yt-dlp usage reference</a>");
    m_referenceLabel->setTextFormat(Qt::RichText);
    m_referenceLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_referenceLabel->setOpenExternalLinks(true);

    m_saveButton = new QPushButton(this);  
    m_saveButton->setText(tr("Save"));

    m_bottomLayout->addWidget(m_referenceLabel, 1);
    m_bottomLayout->addWidget(m_saveButton);

    m_layout->addWidget(m_textBox);
    m_layout->addLayout(m_bottomLayout);

    QSettings settings;
    QString text = settings.value("customOptions", "").toString();

    m_textBox->setPlainText(text);

    connect(m_saveButton, &QPushButton::clicked, this, &CustomOptions::saveOptionsSlot);
}

void CustomOptions::saveOptionsSlot() {
    QSettings settings;
    settings.setValue("customOptions", m_textBox->toPlainText());
    this->accept();
}