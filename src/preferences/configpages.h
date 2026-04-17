#pragma once

#include <QWidget>
#include <QSettings>

class MainPage : public QWidget
{
  Q_OBJECT

public:
  MainPage(QSettings &settings, QWidget *parent = 0);

private:
  QSettings &settings;
};
