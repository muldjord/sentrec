#include "audiorecorder.h"
#include <QDebug>

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

  // Define audio format (32-bit float)
  format.setSampleRate(44100);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Float);

  playbackBuffer.setBuffer(&audioData);

  setLayout(layout);
}

AudioRecorder::~AudioRecorder()
{
  delete audioSource;
  delete audioSink;
}

void AudioRecorder::startRecording()
{
  audioData.clear();

  auto device = QMediaDevices::defaultAudioInput();

  audioSource = new QAudioSource(device, format, this);

  inputDevice = audioSource->start();

  connect(inputDevice, &QIODevice::readyRead, this, [this]() {
    audioData.append(inputDevice->readAll());
  });

  qDebug() << "Recording started";
}

void AudioRecorder::stopRecording()
{
  if(audioSource) {
    audioSource->stop();
    audioSource->deleteLater();
    audioSource = nullptr;
  }

  qDebug() << "Recording stopped. Bytes:" << audioData.size();
}

void AudioRecorder::playRecording()
{
  if(audioData.isEmpty()) {
    qDebug() << "No audio recorded";
    return;
  }

  playbackBuffer.close();
  playbackBuffer.setData(audioData);
  playbackBuffer.open(QIODevice::ReadOnly);

  audioSink = new QAudioSink(format, this);
  audioSink->start(&playbackBuffer);

  qDebug() << "Playback started";
}
