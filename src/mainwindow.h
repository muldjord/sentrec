#pragma once

#include "aboutbox.h"
#include "sentencelist.h"
#include "audiorecorder.h"

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QTabWidget>
#include <QSettings>
#include <QToolBar>

class MainWindow : public QMainWindow
{
  Q_OBJECT
    
public:
  MainWindow();
  ~MainWindow();

protected slots:
  void closeEvent(QCloseEvent *event);

private slots:
  void showAbout();
  void showPreferences();
  
private:
  void createMenus();
  void createActions();
  void createToolBar();
  void createMainLayout();
  void updateFromConfig();

  QMenuBar *menuBar;
  QMenu *fileMenu;
  QMenu *optionsMenu;
  QMenu *helpMenu;
  QAction *quitAct;
  QAction *aboutAct;
  QAction *preferencesAct;
  QToolBar *mainFunctions;

  SentenceList *sentenceList;
  AudioRecorder *audioRecorder;
};
