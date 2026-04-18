#pragma once

#include <QVector>

class AudioProcessor
{
public:
  static QVector<float> cutSilence(const QVector<float> &samples);
  static QVector<float> normalize(const QVector<float> &samples);
  static QVector<float> fadeEnds(const QVector<float> &samples);
};
