#include <sndfile.h>
#include <QVector>
#include <QString>

QVector<float> loadWav(const QString &path, int *sampleRateOut)
{
  SF_INFO info{};
  SNDFILE* file = sf_open(path.toStdString().c_str(), SFM_READ, &info);
  if(!file) {
    return {};
  }

  if(sampleRateOut) {
    *sampleRateOut = info.samplerate;
  }

  // Read all frames interleaved
  QVector<float> interleaved(info.frames * info.channels);
  sf_readf_float(file, interleaved.data(), info.frames);

  sf_close(file);

  // Extract left channel
  QVector<float> buffer;
  buffer.reserve(info.frames);

  for(sf_count_t i = 0; i < info.frames; ++i) {
    buffer.append(interleaved[i * info.channels]); // channel 0 = left
  }

  return buffer;
}

bool saveWav(const QString &path, const QVector<float> &buffer, int sampleRate)
{
  SF_INFO info{};
  info.channels = 1;
  info.samplerate = sampleRate;

  info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  SNDFILE* file = sf_open(path.toStdString().c_str(), SFM_WRITE, &info);
  if(!file) {
    return false;
  }

  sf_writef_float(file, buffer.data(), buffer.size());

  sf_close(file);
  return true;
}
