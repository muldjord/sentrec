#include "mainwindow.h"
#include "preferences/configdialog.h"
#include "settings.h"
#include "version.h"

#include <stdio.h>

#include <QApplication>
#include <QSettings>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>

QMainWindow *mainWindow = nullptr;

extern QSettings *iniSettings;
extern Settings settings;

MainWindow::MainWindow()
{
  mainWindow = this;
  restoreGeometry(iniSettings->value("main/windowState", "").toByteArray());
  setWindowTitle("SentRec v" +
                 QString("%1.%2.%3")
                 .arg(PROJECT_VERSION_MAJOR)
                 .arg(PROJECT_VERSION_MINOR)
                 .arg(PROJECT_VERSION_PATCH)
                 + "[*]");

  createActions();
  createMenus();
  //createToolBar();
  createMainLayout();
}

MainWindow::~MainWindow()
{
  iniSettings->setValue("main/windowState", saveGeometry());
}

void MainWindow::createActions()
{
  quitAct = new QAction(QIcon(":quit.png"), tr("&Quit"), this);
  connect(quitAct, &QAction::triggered, this, &MainWindow::close);

  preferencesAct = new QAction(QIcon(":preferences.png"), tr("&Preferences..."), this);
  connect(preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);

  aboutAct = new QAction(QIcon(":about.png"), tr("&About..."), this);
  connect(aboutAct, &QAction::triggered, this, &MainWindow::showAbout);

  qInfo("Created actions...");
}

void MainWindow::createMenus()
{
  fileMenu = new QMenu(tr("&File"), this);
  fileMenu->addSeparator();
  fileMenu->addAction(quitAct);

  optionsMenu = new QMenu(tr("&Options"), this);
  optionsMenu->addAction(preferencesAct);

  helpMenu = new QMenu(tr("&Help"), this);
  helpMenu->addAction(aboutAct);

  menuBar = new QMenuBar();
  menuBar->addMenu(fileMenu);
  menuBar->addMenu(optionsMenu);
  menuBar->addMenu(helpMenu);
  
  setMenuBar(menuBar);

  qInfo("Created menu...");
}

void MainWindow::createToolBar()
{
  QToolBar *mainFunctions = new QToolBar(tr("Main functions"));
  mainFunctions->setMovable(false);

  addToolBar(Qt::TopToolBarArea, mainFunctions);

  qInfo("Created toolbar...");
}

void MainWindow::createMainLayout()
{
  setCentralWidget(new QWidget(this));

  sentenceList = new SentenceList(this);
  audioRecorder = new AudioRecorder(this);

  connect(sentenceList, &SentenceList::sentenceChanged, audioRecorder, &AudioRecorder::loadWav);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(sentenceList);
  layout->addWidget(audioRecorder);

  centralWidget()->setLayout(layout);

  qInfo("Created main layout...");
}

void MainWindow::showAbout()
{
  AboutBox aboutBox(this);
  aboutBox.exec();
}

void MainWindow::showPreferences()
{
  ConfigDialog preferences(*iniSettings);
  preferences.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if(isWindowModified()) {
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Unsaved sentence edits!"),
							       tr("You have unsaved sentence edits!\nAre you sure you want to quit?"),
							       QMessageBox::Yes | QMessageBox::No,
							       QMessageBox::No);
    if(button == QMessageBox::Yes) {
      event->accept();
    } else {
      event->ignore();
    }
  }
}
