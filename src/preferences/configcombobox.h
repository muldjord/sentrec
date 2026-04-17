#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSettings>

class ConfigComboBox : public QWidget
{
  Q_OBJECT
    
public:
  ConfigComboBox(QSettings &settings,
                 const QString &group, const QString &name, const QString &defaultValue,
                 const QString &caption, const QString &unit = QString(), const bool &vertical = false);
  ~ConfigComboBox();
  void addConfigItem(QString text, QString value);

public slots:
  void resetToDefault();

private slots:
  void saveToConfig();

private:
  QComboBox *comboBox = nullptr;
  QSettings &settings;
  QString key;
  QString defaultValue;
};
