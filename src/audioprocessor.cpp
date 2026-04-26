#include "audioprocessor.h"
#include "settings.h"

#include <vector>
#include <cmath>

#include <QObject>
#include <QMessageBox>

extern Settings settings;

QVector<float> AudioProcessor::cutSilence(const QVector<float> &samples)
{
  int smpsPerMs = round(settings.samplerate / 1000.0);
  int padding = smpsPerMs * settings.paddingMs; // ms audio padding at beginning and end after removing silence

  // First non-silence detection from left BEGIN
  int avgWindow = smpsPerMs * settings.avgWindowMs; // ms of audio
  float avg = 0.0;

  if(samples.size() < padding * 2 ||
     samples.size() < avgWindow) {
    qDebug("Not enough samples to apply padding, padding not applied!");
    return samples;
  }
  
  // Initial average calculation
  for(int i = 0; i < avgWindow; ++i) {
    avg += std::fabs(samples[i]);
  }
  avg /= avgWindow;
  
  float silThres = avg * 3.0; // Calculated silence threshold based on first avgWindow
  qDebug("Silence threshold = %f\n", silThres);
  if(silThres >= 0.1) {
    QMessageBox::warning(nullptr, QObject::tr("Very high noise floor"),
			 QObject::tr("Your input signal has a very high average noise floor (") + QString::number(silThres) + QObject::tr(" to 1.0)\n\nPlease remember to be silent when starting a recording. The first part of the recording is used to determine a noise threshold for trimming.\n\nIf you are already silent when starting recording your input device might be very sensitive to room noise. Consider switching to a microphone with less room sensitivity such as a dynamic microphone."),
			 QMessageBox::Ok,
			 QMessageBox::Ok);
  }

  int begin = avgWindow;

  while(begin < samples.size() && avg < silThres) {
    // Running average
    avg *= avgWindow;
    avg -= std::fabs(samples[begin - avgWindow]);
    avg += std::fabs(samples[begin]);
    avg /= avgWindow;
    begin++;
  }
  // First non-silence detection from left END

  // First non-silence detection from right BEGIN
  avg = 0.0;
  // Initial backwards average calculation
  for(int i = 0; i < avgWindow; ++i) {
    avg += std::fabs(samples[i + samples.size() - avgWindow]);
  }
  avg /= avgWindow;

  int end = samples.size() - 1 - avgWindow;
  while(end > begin && avg < silThres) {
    // Running average
    avg *= avgWindow;
    avg -= std::fabs(samples[end + avgWindow]);
    avg += std::fabs(samples[end]);
    avg /= avgWindow;
    end--;
  }
  // First non-silence detection from right END

  // Remove silence but add padding
  begin -= padding;
  end += padding;
  if(begin < 0) {
    begin = 0;
  }
  if(end >= samples.size()) {
    end = samples.size() - 1;
  }

  QVector<float> trimmed;

  for(int i = begin; i <= end; ++i) {
    trimmed.push_back(samples[i]);
  }
  // Remove silence but add padding END

  return trimmed;
}

QVector<float> AudioProcessor::normalize(const QVector<float> &samples)
{
  float maxAmp = 0.0;

  for(const auto &sample : samples) {
    maxAmp = std::max(maxAmp, std::fabs(sample));
  }

  float gain = 0.9 / maxAmp;

  QVector<float> normalized = samples;

  for(int i = 0; i < samples.size(); ++i) {
    normalized[i] *= gain;
  }

  return normalized;
}

QVector<float> AudioProcessor::fadeEnds(const QVector<float> &samples)
{
  int smpsPerMs = round(settings.samplerate / 1000.0);
  int fadeLen = smpsPerMs * settings.fadeLengthMs; // 20 ms fade applied at beginning and end after adding padding

  if(samples.size() < fadeLen) {
    qDebug("Not enough samples for applying fade-in/out, fade not applied!");
    return samples;
  }
  
  QVector<float> withFades = samples;

  for(int i = 0; i < fadeLen; ++i) {
    float t = float(i) / fadeLen;
    
    withFades[i] *= t;
    withFades[withFades.size() - 1 - i] *= t;
  }
  return withFades;
}
