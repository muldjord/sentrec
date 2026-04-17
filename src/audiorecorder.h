#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QBuffer>

class AudioRecorder : public QWidget
{
  Q_OBJECT

 public:
  AudioRecorder(QWidget *parent = nullptr);
  ~AudioRecorder();

private slots:
  void startRecording();
  void stopRecording();
  void playRecording();

private:
  QPushButton *recordButton;
  QPushButton *stopButton;
  QPushButton *playButton;
  
  QAudioSource *audioSource = nullptr;
  QAudioSink *audioSink = nullptr;
  
  QIODevice *inputDevice = nullptr;
  
  QByteArray audioData;  // raw float32 buffer
  QBuffer playbackBuffer;
  
  QAudioFormat format;
};
