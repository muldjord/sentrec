#include <stdio.h>
#include "configpathedit.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

ConfigPathEdit::ConfigPathEdit(QSettings &settings,
                               const QString &group, const QString &name, const QString &defaultValue,
                               const QString &caption, const bool &vertical)
  : settings(settings), defaultValue(defaultValue)
{
  key = (group != "General"?group + "/":"") + name;

  if(!settings.contains(key)) {
    settings.setValue(key, defaultValue);
  }

  QLabel *captionLabel = new QLabel(caption);
  
  lineEdit = new QLineEdit(this);
  lineEdit->setToolTip(tr("Default value: ") + defaultValue);
  lineEdit->setText(settings.value(key, defaultValue).toString());

  QPushButton *pathButton = new QPushButton(QIcon(":choose_path.png"), "");
  pathButton->setToolTip(tr("Choose path"));
  connect(pathButton, &QPushButton::clicked, this, &ConfigPathEdit::setPath);

  QVBoxLayout *vLayout = nullptr;
  if(vertical) {
    vLayout = new QVBoxLayout;
  }

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  if(vLayout != nullptr) {
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(captionLabel);
    vLayout->addLayout(layout);
  } else {
    layout->addWidget(captionLabel);
  }
  layout->addWidget(lineEdit);
  layout->addWidget(pathButton);
  if(vLayout != nullptr) {
    setLayout(vLayout);
  } else {
    setLayout(layout);
  }
  
  connect(lineEdit, &QLineEdit::textChanged, this, &ConfigPathEdit::saveToConfig);
}

ConfigPathEdit::~ConfigPathEdit()
{
}

void ConfigPathEdit::resetToDefault()
{
  lineEdit->setText(defaultValue);
}

void ConfigPathEdit::saveToConfig()
{
  QString pathString = lineEdit->text().replace("\\", "/");
  settings.setValue(key, pathString);
  qDebug("Key '%s' saved to config with value '%s'\n", qPrintable(key),
         qPrintable(pathString));
}

void ConfigPathEdit::setPath()
{
  QFileDialog fileDialog(this, tr("Choose path"), lineEdit->text());
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if(fileDialog.exec() == QDialog::Accepted) {
    QString chosenPath = fileDialog.directory().absolutePath();
    if(chosenPath.contains(QDir::currentPath())) {
      chosenPath.replace(QDir::currentPath() + "/", "");
    }
    lineEdit->setText(chosenPath);
  }
}
