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

  ConfigCheckBox *showQueuePanelCheckBox = new ConfigCheckBox(settings, tr("Show queue panel on startup"), "main", "showQueuePanel", true);
  connect(resetButton, &QPushButton::clicked, showQueuePanelCheckBox, &ConfigCheckBox::resetToDefault);

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

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(resetButton);
  layout->addWidget(showQueuePanelCheckBox);
  layout->addWidget(diskLoggingCheckBox);
  layout->addWidget(logLevelComboBox);
  layout->addWidget(logFolderPathEdit);
  layout->addWidget(userConfigsCheckBox);
  layout->addWidget(inactivitySlider);
  layout->addStretch();

  setLayout(layout);
}
