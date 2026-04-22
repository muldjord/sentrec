#include "audiorecorder.h"
#include "audioprocessor.h"
#include "wavhandler.h"
#include "settings.h"

#include <QDebug>
#include <QSettings>
#include <QMediaDevices>
#include <QMessageBox>
#include <QAudioDevice>
#include <QLabel>

extern QSettings *iniSettings;
extern Settings settings;

AudioRecorder::AudioRecorder(QWidget *parent)
  : QWidget(parent)
{
  outputDevice = QMediaDevices::defaultAudioOutput();
 
  QLabel *samplerateLabel = new QLabel(tr("Samplerate:"));
  samplerateCombo = new QComboBox;
  connect(samplerateCombo, &QComboBox::activated, this, &AudioRecorder::samplerateChanged);

  QLabel *deviceLabel = new QLabel(tr("Input device:"));
  deviceCombo = new QComboBox;
  for(const auto &device: QMediaDevices::audioInputs()) {
    deviceCombo->addItem(device.description(), device.id());
  }

  QByteArray inputDeviceId = iniSettings->value("audio/inputDeviceId", "").toByteArray();
  int inputDeviceIdx = deviceCombo->findData(inputDeviceId);
  if(inputDeviceIdx != -1) {
    deviceCombo->setCurrentIndex(deviceCombo->findData(inputDeviceId));
  } else {
    deviceCombo->setCurrentIndex(deviceCombo->findData(QMediaDevices::defaultAudioInput().id()));
  }
  inputDeviceChanged(deviceCombo->currentIndex());
  connect(deviceCombo, &QComboBox::currentIndexChanged, this, &AudioRecorder::inputDeviceChanged);

  waveformWidget = new WaveformWidget;

  recordButton = new QPushButton(tr("Record"));
  recordButton->setIcon(QIcon(":record.png"));
  recordButton->setIconSize(QSize(32, 32));
  recordButton->setCheckable(true);

  playButton = new QPushButton(tr("Play"));
  playButton->setIcon(QIcon(":play.png"));
  playButton->setIconSize(QSize(32, 32));

  prevButton = new QPushButton(tr("Previous"));
  prevButton->setIcon(QIcon(":previous.png"));
  prevButton->setIconSize(QSize(32, 32));

  nextButton = new QPushButton(tr("Next"));
  nextButton->setIcon(QIcon(":next.png"));
  nextButton->setIconSize(QSize(32, 32));

  auto deviceLayout = new QHBoxLayout;
  deviceLayout->addWidget(deviceLabel);
  deviceLayout->addWidget(deviceCombo);
  deviceLayout->addWidget(samplerateLabel);
  deviceLayout->addWidget(samplerateCombo);
  deviceLayout->addStretch(1);

  auto buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(prevButton);
  buttonLayout->addWidget(recordButton);
  //buttonLayout->addWidget(stopButton);
  buttonLayout->addWidget(playButton);
  buttonLayout->addWidget(nextButton);
  
  auto vLayout = new QVBoxLayout;
  vLayout->addLayout(deviceLayout);
  vLayout->addWidget(waveformWidget);
  vLayout->addLayout(buttonLayout);

  connect(recordButton, &QPushButton::released, this, &AudioRecorder::toggleRecording);
  //connect(stopButton, &QPushButton::clicked, this, &AudioRecorder::stopRecording);
  connect(playButton, &QPushButton::clicked, this, &AudioRecorder::playRecording);
  connect(prevButton, &QPushButton::clicked, this, [this]() { emit selectPreviousSentence(); });
  connect(nextButton, &QPushButton::clicked, this, [this]() { emit selectNextSentence(); });

  setLayout(vLayout);
}

AudioRecorder::~AudioRecorder()
{
}

void AudioRecorder::loadFromDisk(const QString &id)
{
  buffer.clear();
  QString wavFileString = settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav";
  if(QFileInfo::exists(wavFileString)) {
    qInfo("Loading wav with id '%s' into audio recorder", qPrintable(id));
    int wavSamplerate = 0;
    buffer = loadWav(wavFileString, &wavSamplerate);
    if(settings.samplerate != wavSamplerate) {
      QMessageBox::information(this, tr("Mismatched samplerate"),
			       tr("Wav file: ") + wavFileString + "\n" +
			       tr("Wav file samplerate: ") + QString::number(wavSamplerate) + "\n" +
			       tr("SentRec samplerate: ") + QString::number(settings.samplerate) + "\n\n" +
			       tr("The loaded wav file has a mismatched samplerate to what is currently configured int SentRec. The output will sound either sped up or slowed down. Consider either converting your wav files or configure the SentRec samplerate to match your wav files."),
			       QMessageBox::Ok,
			       QMessageBox::Ok);
    }
  }
  waveformWidget->setSamples(buffer);

  settings.currentSentenceId = id;
}

bool AudioRecorder::saveToDisk(const QString &id)
{
  if(buffer.isEmpty()) {
    qDebug("Buffer is empty, not saving to disk.");
    return true;
  }
  qInfo("Saving wav to disk with id '%s'", qPrintable(id));
  if(saveWav(settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav", buffer, settings.samplerate)) {
    return true;
  }
  return false;
}

void AudioRecorder::deleteFromDisk(const QString &id)
{
  QString wavFileString = settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav";
  if(QFileInfo::exists(wavFileString)) {
    qInfo("Deleting wav with id '%s' from disk", qPrintable(id));
    QFile::remove(wavFileString);
  }
}

void AudioRecorder::toggleRecording()
{
  if(recordButton->isChecked()) {
    if(settings.currentSentenceId.isEmpty()) {
      recordButton->setChecked(false);
      return;
    }
    startRecording();
    recordButton->setText(tr("Stop"));
    recordButton->setIcon(QIcon(":stop.png"));
    playButton->setEnabled(false);
    nextButton->setEnabled(false);
    prevButton->setEnabled(false);
  } else {
    stopRecording();
    recordButton->setText(tr("Record"));
    recordButton->setIcon(QIcon(":record.png"));
    playButton->setEnabled(true);
    nextButton->setEnabled(true);
    prevButton->setEnabled(true);
  }
}

void AudioRecorder::startRecording()
{
  if(!audioSource) {
    return;
  }

  qInfo("Starting recording!");
  buffer.clear();
  audioIn = audioSource->start();
  
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

/*
The range of a 16-bit sample integer in a WAV file is from -32,768 to 32,767. This range allows for a total of 65,536 possible amplitude values for each sample.

The 24-bit WAV audio format can represent values between -8,388,608 and 8,388,607 for signed integers, allowing for a total of 16,777,216 discrete values.
*/  

void AudioRecorder::stopRecording()
{
  if(audioIn == nullptr) {
    return;
  }
  qDebug("Stopping recording!");

  if(audioIn) {
    audioIn->disconnect(this);
  }
  audioIn = nullptr;

  if(settings.autoTrim) {
    buffer = AudioProcessor::cutSilence(buffer);
  }
  if(settings.autoNormalize) {
    buffer = AudioProcessor::normalize(buffer);
  }
  if(settings.autoFade) {
    buffer = AudioProcessor::fadeEnds(buffer);
  }
  
  waveformWidget->setSamples(buffer);

  saveToDisk(settings.currentSentenceId);
}

void AudioRecorder::playRecording()
{
  qDebug("Starting playback! State: %d", audioSink->state());

  // If currently playing, don't play again until it's done!
  if(audioSink) {
    if(audioSink->state() == QAudio::ActiveState) {
      return;
    } else if(audioSink->state() == QAudio::IdleState) {
      audioSink->stop();
    }
  }
  audioOut = audioSink->start();
  if(audioOut) {
    const char* dataPtr = reinterpret_cast<const char*>(buffer.constData());
    qsizetype byteCount = buffer.size() * sizeof(float);
    audioOut->write(dataPtr, byteCount);
  }
}

void AudioRecorder::inputDeviceChanged(int index)
{
  QByteArray deviceId = deviceCombo->currentData().toByteArray();
  for(const auto &device: QMediaDevices::audioInputs()) {
    if(device.id() == deviceId) {
      inputDevice = device;
      iniSettings->setValue("audio/inputDeviceId", deviceId);
      break;
    }
  }

  samplerateCombo->clear();

  QList<int> samplerates;
  samplerates.append(22050);
  samplerates.append(44100);
  samplerates.append(48000);

  int minSamplerate = inputDevice.minimumSampleRate();
  int maxSamplerate = inputDevice.maximumSampleRate();

  for(const auto &samplerate: samplerates) {
    if(samplerate >= minSamplerate && samplerate <= maxSamplerate) {
      samplerateCombo->addItem(QString::number(samplerate), samplerate);
      if(samplerate == settings.samplerate) {
	samplerateCombo->setCurrentIndex(samplerateCombo->count() - 1);
      }
    }
  }
  samplerateChanged(samplerateCombo->currentIndex());
}

void AudioRecorder::samplerateChanged(int index)
{
  int samplerate = samplerateCombo->itemData(index).toInt();
  iniSettings->setValue("audio/samplerate", samplerate);
  settings.samplerate = samplerate;

  setInputDevice();
  setOutputDevice();
}

void AudioRecorder::setInputDevice()
{
  QAudioFormat format;
  format.setSampleRate(settings.samplerate);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Unknown);
  for(const auto &sampleFormat: inputDevice.supportedSampleFormats()) {
    if(sampleFormat > format.sampleFormat()) {
      format.setSampleFormat(sampleFormat);
    }
  }
  qDebug("Best sample format supported is: %d", format.sampleFormat());
  
  if(audioSource) {
    if(audioSource->state() != QAudio::StoppedState) {
      audioSource->stop();
    }
    delete audioSource;
  }
  audioSource = new QAudioSource(inputDevice, format, this);

  qInfo("Set input device to: '%s'", qPrintable(inputDevice.id()));
}

void AudioRecorder::setOutputDevice()
{
  QAudioFormat format;
  format.setSampleRate(settings.samplerate);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Float);
  
  if(audioSink) {
    audioSink->stop();
    delete audioSink;
  }
  audioSink = new QAudioSink(outputDevice, format, this);

  qInfo("Set output device to: '%s'", qPrintable(outputDevice.id()));
}
