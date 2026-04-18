#include "audioprocessor.h"
#include "settings.h"

#include <vector>
#include <cmath>

extern Settings settings;

QVector<float> AudioProcessor::cutSilence(const QVector<float> &samples)
{
  int smpsPerMs = round(settings.sampleRate / 1000.0);
  int padding = 100 * smpsPerMs; // 100 ms padding at beginning and end after removing silence

  // First non-silence detection from left BEGIN
  int avgWindow = 20 * smpsPerMs; // 20 ms of audio
  float avg = 0.0;

  // Initial average calculation
  for(int i = 0; i < avgWindow; ++i) {
    avg += std::fabs(samples[i]);
  }
  avg /= avgWindow;
  
  float silThres = avg * 8.0; // Calculated silence threshold based on average of beginning of audio
  printf("Silence threshold = %f\n", silThres);

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
  int smpsPerMs = round(settings.sampleRate / 1000.0);
  int fadeLen = 20 * smpsPerMs; // 20 ms fade applied at beginning and end after adding padding
  
  QVector<float> withFades = samples;

  for(int i = 0; i < fadeLen; ++i) {
    float t = float(i) / fadeLen;
    
    withFades[i] *= t;
    withFades[withFades.size() - 1 - i] *= t;
  }
  return withFades;
}
