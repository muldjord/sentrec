#include "configcombobox.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ConfigComboBox::ConfigComboBox(QSettings &settings,
                               const QString &group, const QString &name, const QString &defaultValue,
                               const QString &caption, const QString &unit, const bool &vertical)
  : settings(settings), defaultValue(defaultValue)
{
  key = (group != "General"?group + "/":"") + name;

  if(!settings.contains(key)) {
    settings.setValue(key, defaultValue);
  }

  QLabel *captionLabel = new QLabel(caption);

  comboBox = new QComboBox(this);
  comboBox->setToolTip(tr("Default value: ") + defaultValue);

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
  layout->addWidget(comboBox, 1);
  if(unitLabel != nullptr) {
    layout->addWidget(unitLabel);
  }
  if(vLayout != nullptr) {
    setLayout(vLayout);
  } else {
    setLayout(layout);
  }

  connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ConfigComboBox::saveToConfig);
}

ConfigComboBox::~ConfigComboBox()
{
}

void ConfigComboBox::addConfigItem(QString text, QString value)
{
  disconnect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ConfigComboBox::saveToConfig);
  comboBox->addItem(text, value);
  for(int i = 0; i < comboBox->count(); ++i) {
    if(comboBox->itemData(i).toString() == settings.value(key).toString()) {
      comboBox->setCurrentIndex(i);
    }
  }
  connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ConfigComboBox::saveToConfig);
}

void ConfigComboBox::resetToDefault()
{
  for(int i = 0; i < comboBox->count(); ++i) {
    if(comboBox->itemData(i).toString() == defaultValue) {
      comboBox->setCurrentIndex(i);
    }
  }
}

void ConfigComboBox::saveToConfig()
{
  settings.setValue(key, comboBox->currentData().toString());
  qDebug("Key '%s' saved to config with value '%s'\n", qPrintable(key), qPrintable(comboBox->currentData().toString()));
}
