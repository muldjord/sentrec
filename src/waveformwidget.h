#pragma once

#include <QWidget>
#include <QVector>

class WaveformWidget : public QWidget
{
  Q_OBJECT
public:
  WaveformWidget(QWidget *parent = nullptr);
  void setSamples(const QVector<float>& samples);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QVector<float> buffer;
};
