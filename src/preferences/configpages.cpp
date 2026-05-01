#include <QtWidgets>

#include "configpages.h"
#include "configlineedit.h"
#include "configcheckbox.h"
#include "configtextedit.h"
#include "configpathedit.h"
#include "configcombobox.h"
#include "configslider.h"

MainPage::MainPage(QSettings &settings, QWidget *parent) : QWidget(parent), settings(settings)
{
  QPushButton *resetButton = new QPushButton(tr("Reset all to defaults"));

  ConfigCheckBox *csvBackupCheckBox = new ConfigCheckBox(settings, tr("Create CSV backup on load"), "main", "csvBackup", true);
  connect(resetButton, &QPushButton::clicked, csvBackupCheckBox, &ConfigCheckBox::resetToDefault);

  ConfigCheckBox *askDeleteCheckBox = new ConfigCheckBox(settings, tr("Ask before deleting sentences"), "main", "askDelete", true);
  connect(resetButton, &QPushButton::clicked, askDeleteCheckBox, &ConfigCheckBox::resetToDefault);
  /*

  ConfigCheckBox *diskLoggingCheckBox = new ConfigCheckBox(settings, tr("Enable disk logging in addition to GUI logging"), "main", "logToFile", false);
  connect(resetButton, &QPushButton::clicked, diskLoggingCheckBox, &ConfigCheckBox::resetToDefault);
  ConfigComboBox *logLevelComboBox = new ConfigComboBox(settings, "main", "logLevel", "0", tr("Log level:"));
  logLevelComboBox->addConfigItem("0 (disabled)", "0");
  logLevelComboBox->addConfigItem("1 (<= fatal and critical)", "1");
  logLevelComboBox->addConfigItem("2 (<= warnings)", "2");
  logLevelComboBox->addConfigItem("3 (<= info)", "3");
  logLevelComboBox->addConfigItem("4 (<= debug)", "4");
  connect(resetButton, &QPushButton::clicked, logLevelComboBox, &ConfigComboBox::resetToDefault);

  ConfigPathEdit *logFolderPathEdit = new ConfigPathEdit(settings, "main", "logFolder", "logs", tr("Log folder location:"));
  connect(resetButton, &QPushButton::clicked, logFolderPathEdit, &ConfigPathEdit::resetToDefault);

  ConfigCheckBox *userConfigsCheckBox = new ConfigCheckBox(settings, tr("Enable user-specific configuration files"), "main", "userConfigs", true);
  connect(resetButton, &QPushButton::clicked, userConfigsCheckBox, &ConfigCheckBox::resetToDefault);

  ConfigSlider *inactivitySlider = new ConfigSlider(settings, "main", "inactivityTimeoutMinutes", 15,
                                                    5, 120,
                                                    tr("Minutes of inactivity before user logout (restart required):"), tr("minutes"), true);
  connect(resetButton, &QPushButton::clicked, inactivitySlider, &ConfigSlider::resetToDefault);
  */
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(resetButton);
  layout->addWidget(csvBackupCheckBox);
  layout->addWidget(askDeleteCheckBox);
  /*
  layout->addWidget(diskLoggingCheckBox);
  layout->addWidget(logLevelComboBox);
  layout->addWidget(logFolderPathEdit);
  layout->addWidget(userConfigsCheckBox);
  layout->addWidget(inactivitySlider);
  */
  layout->addStretch();

  setLayout(layout);
}

AudioPage::AudioPage(QSettings &settings, QWidget *parent) : QWidget(parent), settings(settings)
{
  QPushButton *resetButton = new QPushButton(tr("Reset all to defaults"));

  ConfigCheckBox *autoPlayCheckBox = new ConfigCheckBox(settings, tr("Autoplay when selecting sentence that already has audio"), "audio", "autoPlay", false);
  connect(resetButton, &QPushButton::clicked, autoPlayCheckBox, &ConfigCheckBox::resetToDefault);

  ConfigCheckBox *autoNormalizeCheckBox = new ConfigCheckBox(settings, tr("Normalize audio after recording"), "audio", "autoNormalize", true);
  connect(resetButton, &QPushButton::clicked, autoNormalizeCheckBox, &ConfigCheckBox::resetToDefault);

  ConfigCheckBox *autoTrimCheckBox = new ConfigCheckBox(settings, tr("Trim silence from beginning and end after recording"), "audio", "autoTrim", true);
  connect(resetButton, &QPushButton::clicked, autoTrimCheckBox, &ConfigCheckBox::resetToDefault);

  ConfigSlider *autoTrimPaddingSlider = new ConfigSlider(settings, "audio", "autoTrimPaddingMs", 150,
							 0, 2000,
							 tr("Trim padding:"), tr("milliseconds"), true);
  connect(resetButton, &QPushButton::clicked, autoTrimPaddingSlider, &ConfigSlider::resetToDefault);
  ConfigSlider *autoTrimAvgWindowSlider = new ConfigSlider(settings, "audio", "autoTrimAvgWindowMs", 100,
							   0, 200,
							   tr("Trim threshold average window size:"), tr("milliseconds"), true);
  connect(resetButton, &QPushButton::clicked, autoTrimAvgWindowSlider, &ConfigSlider::resetToDefault);

  ConfigCheckBox *autoFadeCheckBox = new ConfigCheckBox(settings, tr("Apply fade-in/out after recording"), "audio", "autoFade", true);
  connect(resetButton, &QPushButton::clicked, autoFadeCheckBox, &ConfigCheckBox::resetToDefault);
  ConfigSlider *autoFadeLengthSlider = new ConfigSlider(settings, "audio", "autoFadeLengthMs", 20,
							0, 500,
							tr("Fade-in/out length:"), tr("milliseconds"), true);
  connect(resetButton, &QPushButton::clicked, autoFadeLengthSlider, &ConfigSlider::resetToDefault);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(resetButton);
  layout->addWidget(autoPlayCheckBox);
  layout->addWidget(autoNormalizeCheckBox);
  layout->addWidget(autoTrimCheckBox);
  layout->addWidget(autoTrimPaddingSlider);
  layout->addWidget(autoTrimAvgWindowSlider);
  layout->addWidget(autoFadeCheckBox);
  layout->addWidget(autoFadeLengthSlider);
  layout->addStretch();

  setLayout(layout);
}
