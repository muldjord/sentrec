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
  bool autoPlay = false;
  bool samplerateWarning = true;
  bool autoTrim = true;
  bool autoNormalize = true;
  bool autoFade = true;
  int paddingMs = 150;
  int avgWindowMs = 100;
  int fadeLengthMs = 20;
};
