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

extern QSettings *iniSettings;
extern Settings settings;

MainWindow::MainWindow()
{
  restoreGeometry(iniSettings->value("main/windowState", "").toByteArray());
  setWindowTitle("SentRec v" +
                 QString("%1.%2.%3")
                 .arg(PROJECT_VERSION_MAJOR)
                 .arg(PROJECT_VERSION_MINOR)
                 .arg(PROJECT_VERSION_PATCH)
                 + "[*]");

  createStatusBar();
  createActions();
  createMenus();
  createToolBar();
  createMainLayout();
  
  //QTimer::singleShot(500, this, &MainWindow::init);
}

MainWindow::~MainWindow()
{
  iniSettings->setValue("main/windowState", saveGeometry());
}

void MainWindow::init()
{
}

void MainWindow::createActions()
{
  quitAct = new QAction(QIcon(":quit.png"), tr("&Quit"), this);
  connect(quitAct, &QAction::triggered, this, &MainWindow::close);

  preferencesAct = new QAction(QIcon(":preferences.png"), tr("&Preferences..."), this);
  connect(preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);

  aboutAct = new QAction(QIcon(":about.png"), tr("&About..."), this);
  connect(aboutAct, &QAction::triggered, this, &MainWindow::showAbout);

  qInfo("Created actions...\n");
}

void MainWindow::createMenus()
{
  fileMenu = new QMenu(tr("&File"), this);
  fileMenu->addSeparator();
  fileMenu->addAction(quitAct);

  optionsMenu = new QMenu(tr("&Options"), this);
  optionsMenu->addAction(preferencesAct);

  helpMenu = new QMenu(tr("&Help"), this);
  //helpMenu->addAction(helpAct);
  helpMenu->addAction(aboutAct);

  menuBar = new QMenuBar();
  menuBar->addMenu(fileMenu);
  menuBar->addMenu(optionsMenu);
  menuBar->addMenu(helpMenu);
  
  setMenuBar(menuBar);

  qInfo("Created menu...\n");
}

void MainWindow::createToolBar()
{
  QToolBar *mainFunctions = new QToolBar(tr("Main functions"));
  mainFunctions->setMovable(false);

	//QWidget *spacerWidget = new QWidget(this);
	//spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	//spacerWidget->setVisible(true);

  //mainFunctions->addWidget(findPatient);
  //mainFunctions->addSeparator();

  addToolBar(Qt::TopToolBarArea, mainFunctions);

  qInfo("Created toolbar...\n");
}

void MainWindow::createMainLayout()
{
  setCentralWidget(new QWidget(this));

  sentenceList = new SentenceList(this);
  /*
  connect(diabetesInfo, &DiabetesInfo::updateDiabetesInfo,
          connStatus, &ConnStatus::updateDiabetesInfo);
  */
  audioRecorder = new AudioRecorder(this);

  //addDockWidget(Qt::RightDockWidgetArea, queue);

  /*
  QHBoxLayout *diabetesInfoLayout = new QHBoxLayout();
  diabetesInfoLayout->addWidget(diabetesInfo);
  QGroupBox *diabetesInfoBox = new QGroupBox(tr("Patient information"));
  diabetesInfoBox->setLayout(diabetesInfoLayout);
  diabetesInfoBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  */
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(sentenceList);
  layout->addWidget(audioRecorder);

  centralWidget()->setLayout(layout);

  qInfo("Created main layout...\n");
}

void MainWindow::createStatusBar()
{
  setStatusBar(new QStatusBar(this));
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
    QMessageBox::critical(this, tr("Unsaved data"), tr("There's unsaved data, please save it before quitting."), QMessageBox::Ok);
    event->ignore();
  }
}
