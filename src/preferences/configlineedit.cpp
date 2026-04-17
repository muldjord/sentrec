#include <stdio.h>
#include "configlineedit.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ConfigLineEdit::ConfigLineEdit(QSettings &settings,
                               const QString &group, const QString &name, const QString &defaultValue,
                               const QString &caption, const QString &unit, const bool &vertical)
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

  QLabel *unitLabel = nullptr;
  if(!unit.isEmpty()) {
    unitLabel = new QLabel(unit);
  }

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
  if(unitLabel != nullptr) {
    layout->addWidget(unitLabel);
  }
  if(vLayout != nullptr) {
    setLayout(vLayout);
  } else {
    setLayout(layout);
  }
  
  connect(lineEdit, &QLineEdit::textChanged, this, &ConfigLineEdit::saveToConfig);
}

ConfigLineEdit::~ConfigLineEdit()
{
}

void ConfigLineEdit::setValidator(const QValidator *validator)
{
  lineEdit->setValidator(validator);
}

void ConfigLineEdit::resetToDefault()
{
  lineEdit->setText(defaultValue);
}

void ConfigLineEdit::saveToConfig()
{
  settings.setValue(key, lineEdit->text());
  qDebug("Key '%s' saved to config with value '%s'\n", qPrintable(key), qPrintable(lineEdit->text()));
}
