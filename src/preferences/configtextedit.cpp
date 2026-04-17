#include <stdio.h>
#include "configtextedit.h"

ConfigTextEdit::ConfigTextEdit(QSettings &settings, QString group, QString name, QString defaultValue)
  : settings(settings), defaultValue(defaultValue)
{
  key = (group != "General"?group + "/":"") + name;

  if(!settings.contains(key)) {
    settings.setValue(key, defaultValue);
  }
  setPlainText(settings.value(key, defaultValue).toString());
  setToolTip(tr("Default value: ") + defaultValue);

  connect(this, &QPlainTextEdit::textChanged, this, &ConfigTextEdit::saveToConfig);
}

ConfigTextEdit::~ConfigTextEdit()
{
}

void ConfigTextEdit::resetToDefault()
{
  setPlainText(defaultValue);
}

void ConfigTextEdit::saveToConfig()
{
  settings.setValue(key, toPlainText());
  qDebug("Key '%s' saved to config with value '%s'\n", qPrintable(key), qPrintable(toPlainText()));
}
