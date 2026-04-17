#pragma once

#include "configpages.h"

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QSettings>

class ConfigDialog : public QDialog
{
  Q_OBJECT

public:
  ConfigDialog(QSettings &settings);

public slots:
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);
  
private:
  void createIcons();
  QListWidget *contentsWidget;
  QStackedWidget *pagesWidget;
  MainPage *mainPage;
  
};
