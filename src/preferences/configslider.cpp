#include <stdio.h>
#include "configslider.h"
#include <QHBoxLayout>
#include <QLabel>

ConfigSlider::ConfigSlider(QSettings &settings,
                           const QString &group, const QString &name, const int &defaultValue,
                           const int &minValue, const int &maxValue,
                           const QString &caption, const QString &unit, const bool &vertical)
  : settings(settings), defaultValue(defaultValue)
{
  key = (group != "General"?group + "/":"") + name;
  
  if(!settings.contains(key)) {
    settings.setValue(key, QString::number(defaultValue));
  }

  QLabel *captionLabel = new QLabel(caption);

  lineEdit = new QLineEdit();
  lineEdit->setMaximumWidth(50);
  slider = new QSlider(Qt::Horizontal);
  slider->setMinimum(minValue);
  slider->setMaximum(maxValue);
  slider->setTickInterval(1);
  slider->setSingleStep(1);
  slider->setPageStep(1);
  slider->setToolTip(tr("Default value: ") + QString::number(defaultValue));

  slider->setValue(settings.value(key).toInt());
  lineEdit->setText(QString::number(slider->value()));
  lineEdit->setToolTip(tr("Default value: ") + QString::number(defaultValue));
  
  QLabel *unitLabel = nullptr;
  if(!unit.isEmpty()) {
    unitLabel = new QLabel(unit);
  }

  QVBoxLayout *vLayout = nullptr;
  if(vertical) {
    vLayout = new QVBoxLayout;
  }

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  if(vLayout != nullptr) {
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(captionLabel);
    vLayout->addLayout(layout);
  } else {
    layout->addWidget(captionLabel);
  }
  layout->addWidget(slider);
  layout->addWidget(lineEdit);
  if(unitLabel != nullptr) {
    layout->addWidget(unitLabel);
  }

  if(vLayout != nullptr) {
    setLayout(vLayout);
  } else {
    setLayout(layout);
  }

  connect(slider, &QSlider::valueChanged, this, &ConfigSlider::saveToConfig);
  connect(lineEdit, &QLineEdit::editingFinished, this, &ConfigSlider::setSlider);
}

ConfigSlider::~ConfigSlider()
{
}

void ConfigSlider::resetToDefault()
{
  slider->setValue(defaultValue);
  lineEdit->setText(QString::number(defaultValue));
}

void ConfigSlider::saveToConfig()
{
  lineEdit->setText(QString::number(slider->value()));

  settings.setValue(key, lineEdit->text());
  qDebug("Key '%s' saved to config with value '%s'\n", qPrintable(key), qPrintable(lineEdit->text()));
}

void ConfigSlider::setSlider()
{
  slider->setValue(lineEdit->text().toInt());
}
