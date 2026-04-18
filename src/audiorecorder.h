#pragma once

#include "waveformwidget.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMediaCaptureSession>
#include <QAudioSource>
#include <QAudioSink>

class AudioRecorder : public QWidget
{
  Q_OBJECT

public:
  AudioRecorder(QWidget *parent = nullptr);
  ~AudioRecorder();

public slots:
  void loadFromDisk(const QString &id);
  bool saveToDisk(const QString &id);
		  
private slots:
  void startRecording();
  void stopRecording(); 
  void playRecording();
  void nextRecording();

private:
  WaveformWidget *waveformWidget = nullptr;
  QPushButton *recordButton = nullptr;
  QPushButton *stopButton = nullptr;
  QPushButton *playButton = nullptr;
  QPushButton *nextButton = nullptr;
  
  QAudioSource *audioSource = nullptr;
  QAudioSink *audioSink = nullptr;
  
  QIODevice *audioIn = nullptr;
  QIODevice *audioOut = nullptr;

  QVector<float> buffer;
};
