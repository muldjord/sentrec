#include "waveformwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

WaveformWidget::WaveformWidget(QVector<float> &samples, QWidget *parent)
  : samples(samples), QWidget(parent)
{
  setMinimumHeight(200);
}

void WaveformWidget::reset()
{
  // Reset playhead since new audio is coming in
  playheadPos = 0;
  update();
  state = SR::DATA;
}

void WaveformWidget::setState(const int &state)
{
  this->state = state;
  qDebug("State is: %d", state);
  update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);
  painter.setRenderHint(QPainter::Antialiasing, true);

  switch(state) {
  case SR::INIT:
    qDebug("Waveformwidget at SR::INIT state");
    painter.drawImage(10, 10, QImage(":record_init.png"));
    break;
  case SR::DATA:
    painter.setPen(QPen(Qt::green, 1));

    int w = width();
    int h = height();
    int midY = h / 2;

    int sampleCount = samples.size();

    // Downsampling factor: how many samples per pixel column
    float samplesPerPixel = static_cast<float>(sampleCount) / w;

    for(int x = 0; x < w; ++x) {
      int start = static_cast<int>(x * samplesPerPixel);
      int end = static_cast<int>((x + 1) * samplesPerPixel);
    
      start = std::clamp(start, 0, sampleCount - 1);
      end = std::clamp(end, 0, sampleCount);
    
      float minVal = 1.0f;
      float maxVal = -1.0f;
    
      for(int i = start; i < end; ++i) {
	float s = samples[i];
	minVal = std::min(minVal, s);
	maxVal = std::max(maxVal, s);
      }
    
      int y1 = midY - static_cast<int>(maxVal * midY);
      int y2 = midY - static_cast<int>(minVal * midY);
    
      painter.drawLine(x, y1, x, y2);
    }
    // Draw playhead
    painter.setPen(QPen(Qt::blue, 2));
    float sampleToPixelFactor = (float)w / (samples.size() * sizeof(float));
    int playheadPixelPos = sampleToPixelFactor * playheadPos;
    painter.drawLine(playheadPixelPos, 0, playheadPixelPos, h);
    break;
  }
}

void WaveformWidget::setPlayheadPos(const qint64 &pos)
{
  playheadPos = pos;
  update();
}
