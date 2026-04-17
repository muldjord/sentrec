#pragma once

#include <QPlainTextEdit>
#include <QSettings>

class ConfigTextEdit : public QPlainTextEdit
{
  Q_OBJECT
    
public:
  ConfigTextEdit(QSettings &settings, QString group, QString name, QString defaultValue);
  ~ConfigTextEdit();
  
public slots:
  void resetToDefault();

protected:
  
private slots:
  void saveToConfig();

private:
  QSettings &settings;
  QString key;
  QString defaultValue;
};
