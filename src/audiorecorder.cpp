#include "audiorecorder.h"

#include <QDebug>
#include <QMediaDevices>
#include <QAudioDevice>

AudioRecorder::AudioRecorder(QWidget *parent)
  : QWidget(parent)
{
  recordButton = new QPushButton(tr("Record"));
  stopButton = new QPushButton(tr("Stop"));
  playButton = new QPushButton(tr("Play"));
  nextButton = new QPushButton(tr("Next"));

  auto layout = new QVBoxLayout(this);
  layout->addWidget(recordButton);
  layout->addWidget(stopButton);
  layout->addWidget(playButton);
  layout->addWidget(nextButton);

  connect(recordButton, &QPushButton::clicked, this, &AudioRecorder::startRecording);
  connect(stopButton, &QPushButton::clicked, this, &AudioRecorder::stopRecording);
  connect(playButton, &QPushButton::clicked, this, &AudioRecorder::playRecording);
  connect(nextButton, &QPushButton::clicked, this, &AudioRecorder::nextRecording);

  setLayout(layout);

  QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
  QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
  
  QAudioFormat format;
  format.setSampleRate(44100);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Float);
  
  audioSource = new QAudioSource(inputDevice, format, this);
  audioSink = new QAudioSink(outputDevice, format, this);
}

AudioRecorder::~AudioRecorder()
{
}

void AudioRecorder::loadWav(const QString &id)
{
  qInfo("Loading wav with id '%s' into audio recorder", qPrintable(id));
}

void AudioRecorder::startRecording()
{
  if(!audioSource) {
    return;
  }
  qInfo("Starting recording!");
  audioIn = audioSource->start();

  buffer.clear();
  
  connect(audioIn, &QIODevice::readyRead, this, [this]() {
    QByteArray data = audioIn->readAll();

    const float* samples = reinterpret_cast<const float*>(data.constData());
    qint64 sampleCount = data.size() / sizeof(float);

    buffer.reserve(sampleCount);
    
    for(int i = 0; i < sampleCount; ++i) {
      buffer.append(samples[i]);
    }
  });
}

void AudioRecorder::stopRecording()
{
  qDebug("Stopping recording!");
  if(audioIn) {
    audioIn->disconnect(this);
  }
}

void AudioRecorder::playRecording()
{
  qDebug("Starting playback!");

  // If currently playing, don't play again until it's done!
  if(audioSink && audioSink->state() == QAudio::ActiveState) {
    return;
  }

  audioOut = audioSink->start();
  if(audioOut) {
    const char* dataPtr = reinterpret_cast<const char*>(buffer.constData());
    qsizetype byteCount = buffer.size() * sizeof(float);
    audioOut->write(dataPtr, byteCount);
  }
}

void AudioRecorder::nextRecording()
{
  qDebug("Moving to next recording!");
  /*
  if(audioIn) {
    audioIn->disconnect(this);
  }
  */
}
