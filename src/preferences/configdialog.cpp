#include "configdialog.h"

#include <QtWidgets>

ConfigDialog::ConfigDialog(QSettings &settings)
{
  setFixedSize(700, 600);
  contentsWidget = new QListWidget;
  contentsWidget->setViewMode(QListView::IconMode);
  contentsWidget->setIconSize(QSize(64, 64));
  contentsWidget->setMovement(QListView::Static);
  contentsWidget->setMaximumWidth(128);
  contentsWidget->setSpacing(12);

  mainPage = new MainPage(settings);

  pagesWidget = new QStackedWidget;
  pagesWidget->addWidget(mainPage);

  QPushButton *okButton = new QPushButton(tr("Ok"));
  okButton->setDefault(true);
  connect(okButton, &QPushButton::clicked, this, &ConfigDialog::accept);
  
  createIcons();
  contentsWidget->setCurrentRow(0);

  QHBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(contentsWidget);
  horizontalLayout->addWidget(pagesWidget, 1);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(okButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Preferences"));
  okButton->setFocus();
}

void ConfigDialog::createIcons()
{
  QListWidgetItem *mainButton = new QListWidgetItem(contentsWidget);
  mainButton->setIcon(QIcon(":generalconfig.png"));
  mainButton->setText(tr("General"));
  mainButton->setTextAlignment(Qt::AlignHCenter);
  mainButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  connect(contentsWidget, &QListWidget::currentItemChanged, this, &ConfigDialog::changePage);
}

void ConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
  if(!current) {
    current = previous;
  }

  pagesWidget->setCurrentIndex(contentsWidget->row(current));
}
