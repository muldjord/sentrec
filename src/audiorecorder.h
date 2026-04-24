#pragma once

#include "waveformwidget.h"

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QMediaCaptureSession>
#include <QAudioSource>
#include <QAudioSink>
#include <QBuffer>
#include <QTimer>

class AudioRecorder : public QWidget
{
  Q_OBJECT

public:
  AudioRecorder(QWidget *parent = nullptr);
  ~AudioRecorder();

public slots:
  void loadFromDisk(const QString &id);
  void deleteFromDisk(const QString &id);
		  
signals:
  void enableSentenceList();
  void disableSentenceList();
  void selectNextSentence();
  void selectPreviousSentence();

private slots:
  void refreshInputDevices();
  void inputDeviceChanged(int index);
  void samplerateChanged(int index);
  void audioSinkStateChanged(QAudio::State state);
  void toggleRecording();
  void startRecording();
  void stopRecording(); 
  void playRecording();
  void updatePlayhead();
  
private:
  bool saveToDisk(const QString &id);

  void setInputDevice();
  void setOutputDevice();

  QTimer playheadTimer;

  QPushButton *refreshDevicesButton = nullptr;
  QComboBox *devicesCombo = nullptr;
  QComboBox *samplerateCombo = nullptr;

  WaveformWidget *waveformWidget = nullptr;

  QPushButton *recordButton = nullptr;
  //QPushButton *stopButton = nullptr;
  QPushButton *playButton = nullptr;
  QPushButton *prevButton = nullptr;
  QPushButton *nextButton = nullptr;
  
  QAudioDevice inputDevice;
  QAudioDevice outputDevice;

  QAudioSource *audioSource = nullptr;
  QAudioSink *audioSink = nullptr;
  
  QIODevice *audioIn = nullptr;
  QBuffer *outBuffer = nullptr;

  QVector<float> audioData;
};
