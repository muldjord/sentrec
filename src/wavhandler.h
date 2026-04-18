#pragma once

#include <QVector>
#include <QString>

QVector<float> loadWav(const QString& path, int* sampleRateOut = nullptr);

bool saveWav(const QString& path, const QVector<float>& buffer, int sampleRate);
