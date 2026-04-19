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
  settings.outputDevice = QMediaDevices::defaultAudioOutput();
 
  QLabel *samplerateLabel = new QLabel(tr("Samplerate:"));
  samplerateCombo = new QComboBox;
  connect(samplerateCombo, &QComboBox::activated, this, &AudioRecorder::samplerateChanged);

  QLabel *deviceLabel = new QLabel(tr("Input device:"));
  deviceCombo = new QComboBox;
  connect(deviceCombo, &QComboBox::activated, this, &AudioRecorder::inputDeviceChanged);
  for(const auto &device: QMediaDevices::audioInputs()) {
    deviceCombo->addItem(device.description(), device.id());
    if(settings.inputDevice.id() == device.id()) {
      deviceCombo->setCurrentIndex(deviceCombo->count() - 1);
    }
  }
  inputDeviceChanged(deviceCombo->currentIndex());
  
  waveformWidget = new WaveformWidget;
  recordButton = new QPushButton(tr("Record"));
  recordButton->setCheckable(true);
  //stopButton = new QPushButton(tr("Stop"));
  playButton = new QPushButton(tr("Play"));
  prevButton = new QPushButton(tr("<< Previous"));
  nextButton = new QPushButton(tr("Next >>"));

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

void AudioRecorder::toggleRecording()
{
  if(recordButton->isChecked()) {
    if(settings.currentSentenceId.isEmpty()) {
      recordButton->setChecked(false);
      return;
    }
    startRecording();
    recordButton->setText(tr("Stop"));
    playButton->setEnabled(false);
    nextButton->setEnabled(false);
  } else {
    stopRecording();
    recordButton->setText(tr("Record"));
    playButton->setEnabled(true);
    nextButton->setEnabled(true);
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

void AudioRecorder::inputDeviceChanged(int index)
{
  QByteArray deviceId = deviceCombo->itemData(index).toByteArray();
  iniSettings->setValue("audio/inputDeviceId", deviceId);

  for(const auto &device: QMediaDevices::audioInputs()) {
    if(deviceId == device.id()) {
      settings.inputDevice = device;
      break;
    }
  }

  samplerateCombo->clear();

  QList<int> samplerates;
  samplerates.append(22050);
  samplerates.append(44100);
  samplerates.append(48000);

  int minSamplerate = settings.inputDevice.minimumSampleRate();
  int maxSamplerate = settings.inputDevice.maximumSampleRate();

  for(const auto &samplerate: samplerates) {
    if(samplerate >= minSamplerate && samplerate <= maxSamplerate) {
      samplerateCombo->addItem(QString::number(samplerate), samplerate);
      if(samplerate == settings.samplerate) {
	// This also runs samplerateChanged which in turn initiates the new input device
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
  format.setSampleFormat(QAudioFormat::Float);

  if(audioSource) {
    if(audioSource->state() != QAudio::StoppedState) {
      audioSource->stop();
    }
    delete audioSource;
  }
  audioSource = new QAudioSource(settings.inputDevice, format, this);

  qInfo("Set input device to: '%s'", qPrintable(settings.inputDevice.id()));
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
  audioSink = new QAudioSink(settings.outputDevice, format, this);

  qInfo("Set output device to: '%s'", qPrintable(settings.outputDevice.id()));
}
