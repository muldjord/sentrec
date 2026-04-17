#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

class ConfigPathEdit : public QWidget
{
  Q_OBJECT
    
public:
  ConfigPathEdit(QSettings &settings,
                 const QString &group, const QString &name, const QString &defaultValue,
                 const QString &caption, const bool &vertical = false);
  ~ConfigPathEdit();
  
public slots:
  void resetToDefault();

protected:
  
private slots:
  void saveToConfig();
  void setPath();

private:
  QSettings &settings;
  QString key;
  QString defaultValue;
  QLineEdit *lineEdit = nullptr;
};
