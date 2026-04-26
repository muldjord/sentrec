#pragma once

#include <QFileInfo>
#include <QAudioDevice>

struct Settings {
  QFileInfo sentenceFileInfo;
  QString currentSentenceId = "";
  bool csvBackup = true;
  bool askDelete = true;
  
  // Audio configs
  int samplerate = 44100;
  bool autoTrim = true;
  bool autoNormalize = true;
  bool autoFade = true;
  int endPaddingMs = 150;
  int avgWindowMs = 100;
  int fadeLengthMs = 20;
};
