#include "configcheckbox.h"
#include <stdio.h>

ConfigCheckBox::ConfigCheckBox(QSettings &settings, QString caption, QString group, QString name, bool defaultValue, QList<QWidget *> childWidgets)
  : QCheckBox(caption), settings(settings), defaultValue(defaultValue), childWidgets(childWidgets)
{
  key = (group != "General"?group + "/":"") + name;

  if(!settings.contains(key)) {
    settings.setValue(key, defaultValue);
  }
  setChecked(settings.value(key, defaultValue).toBool());
  setToolTip(tr("Default value: ") + QString(defaultValue?tr("true"):tr("false")));
  
  connect(this, &ConfigCheckBox::checkStateChanged, this, &ConfigCheckBox::saveToConfig);
  refreshState();
}

ConfigCheckBox::~ConfigCheckBox()
{
}

void ConfigCheckBox::refreshState()
{
  // Hack to make the checkbox send changed signals
  toggle();
  toggle();
}

void ConfigCheckBox::resetToDefault()
{
  setChecked(defaultValue);
}

void ConfigCheckBox::saveToConfig(int)
{
  if(isChecked()) {
    for(auto *childWidget: childWidgets) {
      childWidget->setEnabled(true);
    }
    settings.setValue(key, true);
    qDebug("Key '%s' saved to config with value 'true'\n", qPrintable(key));
  } else {
    for(auto *childWidget: childWidgets) {
      childWidget->setEnabled(false);
    }
    settings.setValue(key, false);
    qDebug("Key '%s' saved to config with value 'false'\n", qPrintable(key));
  }
}
