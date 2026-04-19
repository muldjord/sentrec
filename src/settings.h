#pragma once

#include <QFileInfo>
#include <QAudioDevice>

struct Settings {
  QFileInfo sentenceFileInfo;

  // Audio configs
  QAudioDevice inputDevice;
  int samplerate = 44100;
  bool autoTrim = true;
  bool autoNormalize = true;
  bool autoFade = true;
};
