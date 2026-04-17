#pragma once

#include <QCheckBox>
#include <QSettings>

class ConfigCheckBox : public QCheckBox
{
  Q_OBJECT
    
public:
  ConfigCheckBox(QSettings &settings, QString caption, QString group, QString name, bool defaultValue, QList<QWidget *> childWidgets = QList<QWidget *>());
  ~ConfigCheckBox();
  void refreshState();

public slots:
  void resetToDefault();

private slots:
  void saveToConfig(int);

private:
  QSettings &settings;
  QString key;
  bool defaultValue;
  QList<QWidget*> childWidgets;

};
