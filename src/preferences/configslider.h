#pragma once

#include <QWidget>
#include <QSettings>
#include <QSlider>
#include <QLineEdit>

class ConfigSlider : public QWidget
{
  Q_OBJECT
    
public:
  ConfigSlider(QSettings &settings,
               const QString &group, const QString &name, const int &defaultValue,
               const int &minValue, const int &maxValue,
               const QString &caption, const QString &unit = QString(), const bool &vertical = false);
  ~ConfigSlider();
  
public slots:
  void resetToDefault();

protected:
  
private slots:
  void saveToConfig();
  void setSlider();

private:
  QSettings &settings;
  QSlider *slider;
  QLineEdit *lineEdit;
  
  QString key;

  int defaultValue;
};
