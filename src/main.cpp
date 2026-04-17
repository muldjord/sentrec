#include "mainwindow.h"
#include "settings.h"

#include <QStyleFactory>
#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QCommandLineParser>
#include <QSettings>

QSettings *iniSettings = nullptr;
Settings settings;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QDir::setCurrent(QApplication::applicationDirPath());

  QSettings s("config.ini", QSettings::IniFormat);
  iniSettings = &s;

  /*
  if(!iniSettings->contains("main/logLevel")) {
    iniSettings->setValue("main/logLevel", 0);
  }
  settings.logLevel = iniSettings->value("main/logLevel", 0).toInt();
  */

  app.setStyle(QStyleFactory::create("Fusion"));

  QTranslator translator;
  if(translator.load("sentrec_" + iniSettings->value("main/locale", QLocale::system().name()).toString())) {
    app.installTranslator(&translator);
  }

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
