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
#include <QMediaDevices>

QMainWindow *mainWindow = nullptr;

extern QSettings *iniSettings;
extern Settings settings;

MainWindow::MainWindow()
{
  mainWindow = this;
  if(iniSettings->contains("main/windowState")) {
    restoreGeometry(iniSettings->value("main/windowState", "").toByteArray());
  } else {
    resize(1027, 768);
  }
  setWindowIcon(QIcon(":icon.png"));
  setWindowTitle("SentRec v" +
                 QString("%1.%2.%3")
                 .arg(PROJECT_VERSION_MAJOR)
                 .arg(PROJECT_VERSION_MINOR)
                 .arg(PROJECT_VERSION_PATCH)
                 + "[*]");

  updateFromConfig();

  createActions();
  createMenus();
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

  loadAct = new QAction(QIcon(":load.png"), tr("&Load sentences..."), this);

  preferencesAct = new QAction(QIcon(":preferences.png"), tr("&Preferences..."), this);
  connect(preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);

  aboutAct = new QAction(QIcon(":about.png"), tr("&About..."), this);
  connect(aboutAct, &QAction::triggered, this, &MainWindow::showAbout);

  qInfo("Created actions...");
}

void MainWindow::createMenus()
{
  fileMenu = new QMenu(tr("&File"), this);
  fileMenu->addAction(loadAct);
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

void MainWindow::createMainLayout()
{
  setCentralWidget(new QWidget(this));

  sentenceList = new SentenceList(this);
  
  audioRecorder = new AudioRecorder(this);
  audioRecorder->setEnabled(false);
  connect(audioRecorder, &AudioRecorder::disableSentenceList, sentenceList, &SentenceList::disableSentenceList);
  connect(audioRecorder, &AudioRecorder::enableSentenceList, sentenceList, &SentenceList::enableSentenceList);
  connect(audioRecorder, &AudioRecorder::selectPreviousSentence, sentenceList, &SentenceList::selectPreviousSentence);
  connect(audioRecorder, &AudioRecorder::selectNextSentence, sentenceList, &SentenceList::selectNextSentence);

  connect(loadAct, &QAction::triggered, sentenceList, &SentenceList::loadSentences);
  connect(sentenceList, &SentenceList::sentencesLoaded, this, [this]() { audioRecorder->setEnabled(true); });
  connect(sentenceList, &SentenceList::enteringSentence, audioRecorder, &AudioRecorder::loadFromDisk);
  connect(sentenceList, &SentenceList::deleteFromDisk, audioRecorder, &AudioRecorder::deleteFromDisk);

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

  updateFromConfig();
}

void MainWindow::updateFromConfig()
{
  // Update internal config to match ini settings
  settings.csvBackup = iniSettings->value("main/csvBackup", true).toBool();
  settings.askDelete = iniSettings->value("main/askDelete", true).toBool();

  settings.samplerate = iniSettings->value("audio/samplerate", 44100).toInt();

  settings.autoPlay = iniSettings->value("audio/autoPlay", false).toBool();
  settings.samplerateWarning = iniSettings->value("audio/samplerateWarning", true).toBool();

  settings.autoTrim = iniSettings->value("audio/autoTrim", true).toBool();
  settings.paddingMs = iniSettings->value("audio/autoTrimPaddingMs", 150).toInt();
  settings.avgWindowMs = iniSettings->value("audio/autoTrimAvgWindowMs", 100).toInt();

  settings.autoNormalize = iniSettings->value("audio/autoNormalize", true).toBool();

  settings.autoFade = iniSettings->value("audio/autoFade", true).toBool();
  settings.fadeLengthMs = iniSettings->value("audio/autoFadeLengthMs", 20).toInt();
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
