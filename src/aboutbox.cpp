#include "aboutbox.h"
#include "version.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QTimer>
#include <QScrollBar>
#include <QScrollArea>
#include <QPushButton>
#include <QApplication>
#include <QDateTime>

AboutBox::AboutBox(QWidget *parent): QDialog(parent)
{
  setWindowTitle(tr("About SentRec"));
  setFixedSize(590,500);

  // Read AUTHORS data from file
  QFile file(":AUTHORS");
  QByteArray authorsText;
  if(file.open(QIODevice::ReadOnly)) {
    authorsText = file.readAll();
    file.close();
  } else {
    qWarning("WARNING: Couldn't find AUTHORS file at the designated location.\n");
    authorsText = "ERROR: File not found...";
  }

  // Read LICENSE data from file
  file.setFileName(":LICENSE");
  QByteArray gplText;
  if(file.open(QIODevice::ReadOnly)) {
    gplText = file.readAll();
    file.close();
  } else {
    qWarning("WARNING: Couldn't find LICENSE file at the designated location.\n");
    gplText = "ERROR: File not found...";
  }

  QVBoxLayout *layout = new QVBoxLayout();

  QHBoxLayout *topLayout = new QHBoxLayout();
  QLabel *logo = new QLabel();
  logo->setScaledContents(true);
  logo->setPixmap(QPixmap(":icon.png"));
  logo->setFixedSize(128, 128);
  QLabel *title = new QLabel;
  title->setWordWrap(true);
  title->setText(tr("<h1>SentRec</h1>"
                    "<h2>Version ")
                 + QString("%1.%2.%3")
                 .arg(PROJECT_VERSION_MAJOR)
                 .arg(PROJECT_VERSION_MINOR)
                 .arg(PROJECT_VERSION_PATCH)
                 + tr("</h2>"
                      "SentRec Copyright (c) ") + QDateTime::currentDateTime().toString("yyyy") +
                 tr(" Lars Muldjord<br />"
                    "SentRec is free software released under the terms "
                    "of the GNU General Public License. Click the license "
                    "tab for full license information."));
  topLayout->addWidget(logo);
  topLayout->addWidget(title);
  topLayout->setStretch(1, 1);
  topLayout->setStretch(2, 2);

  QLabel *releaseInfo = new QLabel(authorsText);
  releaseInfo->setStyleSheet("QLabel { background-color : white; }");

  QScrollArea *releaseInfoScroll = new QScrollArea();
  releaseInfoScroll->setWidget(releaseInfo);
  releaseInfoScroll->setStyleSheet("QScrollArea { background-color : white; }");

  QLabel *license = new QLabel(gplText);
  license->setStyleSheet("QLabel { font-family: monospace; "
                     "background-color : white; }");
  licenseScroll = new QScrollArea();
  licenseScroll->setStyleSheet("QScrollArea { background-color : white; }");
  licenseScroll->setWidget(license);

  connect(licenseScroll->verticalScrollBar(), &QScrollBar::sliderPressed,
          this, &AboutBox::noMoreScroll);
  tabs = new QTabWidget();
  connect(tabs, &QTabWidget::currentChanged, this, &AboutBox::checkTab);
  tabs->addTab(releaseInfoScroll, "Release Info");
  tabs->addTab(licenseScroll, "License");

  layout->addLayout(topLayout);
  layout->addWidget(tabs);
  setLayout(layout);

  scrollState = 0; // 0 = initial state
                   // 1 = initiate scroll
                   // 2 = scroll 1 step
                   // 3 = no more scroll

  scrollTimer = new QTimer();
  scrollTimer->setSingleShot(true);
  connect(scrollTimer, &QTimer::timeout, this, &AboutBox::scroll);
}

void AboutBox::scroll()
{
  switch(scrollState) {
  case 0 : // 0 = initial state
    scrollState = 1;
    scrollTimer->setInterval(5000);
    scrollTimer->start();
    break;
  case 1 : // 1 = initiate scroll
    scrollState = 2;
    licenseScroll->verticalScrollBar()->setValue(0);
    scrollTimer->setInterval(200);
    scrollTimer->start();
    break;
  case 2 : // 2 = scroll 1 step
    licenseScroll->verticalScrollBar()->
      setValue(licenseScroll->verticalScrollBar()->value() + 2);
    if(licenseScroll->verticalScrollBar()->value() >=
       licenseScroll->verticalScrollBar()->maximum()) {
      scrollState = 0;
    }
    scrollTimer->start();
    break;
  case 3 : // 3 = no more scroll
    scrollTimer->stop();
    break;
  }
}

void AboutBox::noMoreScroll()
{
  scrollState = 3;
}

void AboutBox::checkTab(int tabIndex)
{
  if(tabIndex == 1) {
    scrollState = 0;
    scrollTimer->stop();
    scroll();
  }
}
