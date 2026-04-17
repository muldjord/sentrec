#include "audiorecorder.h"

#include <QDebug>
#include <QMediaDevices>
#include <QAudioDevice>

AudioRecorder::AudioRecorder(QWidget *parent)
  : QWidget(parent)
{
  recordButton = new QPushButton("Record");
  stopButton = new QPushButton("Stop");
  playButton = new QPushButton("Play");

  auto layout = new QVBoxLayout(this);
  layout->addWidget(recordButton);
  layout->addWidget(stopButton);
  layout->addWidget(playButton);

  connect(recordButton, &QPushButton::clicked, this, &AudioRecorder::startRecording);
  connect(stopButton, &QPushButton::clicked, this, &AudioRecorder::stopRecording);
  connect(playButton, &QPushButton::clicked, this, &AudioRecorder::playRecording);

  inputDevice = new QAudioInput;
  inputDevice->setDevice(QMediaDevices::defaultAudioInput());
  outputDevice = new QAudioOutput;
  outputDevice->setDevice(QMediaDevices::defaultAudioOutput());

  captureSession = new QMediaCaptureSession(this);
  captureSession->setAudioInput(inputDevice);
  captureSession->setAudioOutput(outputDevice);
  recorder = new QMediaRecorder;
  recorder->setAudioBitRate(32);
  recorder->setAudioChannelCount(1);
  recorder->setAudioSampleRate(48000);
  captureSession->setRecorder(recorder);

  setLayout(layout);
}

AudioRecorder::~AudioRecorder()
{
}

void AudioRecorder::startRecording()
{
  qDebug() << "Recording started";
}

void AudioRecorder::stopRecording()
{
  qDebug() << "Recording stopped.";
}

void AudioRecorder::playRecording()
{
  qDebug() << "Playback started";
}
