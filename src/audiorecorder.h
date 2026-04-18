#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QAudioInput>
#include <QAudioOutput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>

class AudioRecorder : public QWidget
{
  Q_OBJECT

public:
  AudioRecorder(QWidget *parent = nullptr);
  ~AudioRecorder();

public slots:
  void loadWav(const QString &id);
		  
private slots:
  void startRecording();
  void stopRecording();
  void playRecording();

private:
  QPushButton *recordButton;
  QPushButton *stopButton;
  QPushButton *playButton;
  
  QAudioInput *inputDevice = nullptr;
  QAudioOutput *outputDevice = nullptr;

  QMediaCaptureSession *captureSession = nullptr;
  QMediaRecorder *recorder = nullptr;
};
