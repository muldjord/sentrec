#pragma once

#include <QWidget>
#include <QVector>

class WaveformWidget : public QWidget
{
  Q_OBJECT
public:
  WaveformWidget(QVector<float> &samples, QWidget *parent = nullptr);
  void setSamples(QVector<float> &samples);
  void setPlayheadPos(const qint64 &pos);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  qint64 playheadPos = 0;
  QVector<float> &samples;
};
