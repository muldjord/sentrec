#pragma once

#include <QFileInfo>
#include <QAudioDevice>

struct Settings {
  QFileInfo sentenceFileInfo;
  QString currentSentenceId = "";

  // Audio configs
  QAudioDevice inputDevice;
  QAudioDevice outputDevice;
  int samplerate = 44100;
  bool autoTrim = true;
  bool autoNormalize = true;
  bool autoFade = true;
};
