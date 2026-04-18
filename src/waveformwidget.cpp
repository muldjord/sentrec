#include "waveformwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

WaveformWidget::WaveformWidget(QWidget *parent)
  : QWidget(parent)
{
  setMinimumHeight(200);
}

void WaveformWidget::setSamples(const QVector<float>& samples)
{
  buffer = samples;
  update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);
  painter.setRenderHint(QPainter::Antialiasing, false);

  if(buffer.isEmpty()) {
    return;
  }

  painter.setPen(QPen(Qt::green, 1));

  int w = width();
  int h = height();
  int midY = h / 2;

  int sampleCount = buffer.size();

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
      float s = buffer[i];
      minVal = std::min(minVal, s);
      maxVal = std::max(maxVal, s);
    }
    
    int y1 = midY - static_cast<int>(maxVal * midY);
    int y2 = midY - static_cast<int>(minVal * midY);
    
    painter.drawLine(x, y1, x, y2);
  }
}
