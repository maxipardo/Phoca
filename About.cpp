#include "About.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

About::About(QWidget *parent) : QDialog(parent) {
    this->setWindowTitle(tr("About Phoca") + " " + PROJECT_VERSION);
    this->resize(300, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *sourceLayout = new QHBoxLayout(); 
    layout->setAlignment(Qt::AlignCenter);

    QLabel *icon = new QLabel(this);
    QPixmap imagen(":/phoca.png");
    icon->setPixmap(imagen.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("<h2>Phoca</h2>", this);
    title->setAlignment(Qt::AlignCenter);

    QLabel *description = new QLabel(tr("Download any video/audio through yt-dlp."), this);
    description->setAlignment(Qt::AlignCenter);

    QLabel *author = new QLabel("<br><b>Máximo Pardo</b>", this);
    author->setAlignment(Qt::AlignCenter);

    QPushButton *repoButton = new QPushButton(tr("Source on GitHub"), this);
    QPushButton *licensesButton = new QPushButton(tr("Licenses"), this);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(description);
    layout->addStretch();
    layout->addWidget(author);
    layout->addStretch();
    sourceLayout->addWidget(repoButton);
    sourceLayout->addWidget(licensesButton);
    layout->addLayout(sourceLayout);

    connect(repoButton, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/maxipardo/Phoca"));
    });
    
    connect(licensesButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::about(this, "Licenses", 
            "© 2026 Máximo Pardo <br><br>"
            "This application comes with absolutely no warranty. See the <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU General Public License, version 3 or later</a> for details. <br><br>"
            "<b><a href='https://github.com/yt-dlp/yt-dlp/blob/master/LICENSE'>yt-dlp</a>: </b>Unlicense<br>"
            "<b><a href='https://ffmpeg.org/legal.html'>FFmpeg & FFprobe</a>: </b> GPL 2, LGPL 2.1 and more<br>"
            "<b><a href='https://github.com/denoland/deno/blob/main/LICENSE.md'>Deno</a>: </b>MIT License<br><br>"
            "<i>Source code and license details are available on their official repositories.</i>");
    });
}