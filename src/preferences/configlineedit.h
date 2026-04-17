#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QSettings>
#include <QValidator>

class ConfigLineEdit : public QWidget
{
  Q_OBJECT
    
public:
  ConfigLineEdit(QSettings &settings,
                 const QString &group, const QString &name, const QString &defaultValue,
                 const QString &caption, const QString &unit = QString(), const bool &vertical = false);
  ~ConfigLineEdit();
  void setValidator(const QValidator *validator);
  
public slots:
  void resetToDefault();

protected:
  
private slots:
  void saveToConfig();

private:
  QLineEdit *lineEdit = nullptr;
  QSettings &settings;
  QString key;
  QString defaultValue;
};
