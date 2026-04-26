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
  QLabel *samplerateLabel = new QLabel(tr("Samplerate:"));
  samplerateCombo = new QComboBox;
  connect(samplerateCombo, &QComboBox::activated, this, &AudioRecorder::samplerateChanged);

  QLabel *deviceLabel = new QLabel(tr("Input device:"));
  refreshDevicesButton = new QPushButton(this);
  refreshDevicesButton->setIcon(QIcon(":refresh.png"));
  refreshDevicesButton->setIconSize(QSize(20, 20));
  connect(refreshDevicesButton, &QPushButton::clicked, this, &AudioRecorder::refreshInputDevices);

  devicesCombo = new QComboBox;
  connect(devicesCombo, &QComboBox::activated, this, &AudioRecorder::inputDeviceChanged);
  refreshInputDevices();

  waveformWidget = new WaveformWidget(audioData, this);

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
  deviceLayout->addWidget(refreshDevicesButton);
  deviceLayout->addWidget(devicesCombo);
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
  connect(playButton, &QPushButton::clicked, this, &AudioRecorder::startPlaying);
  connect(prevButton, &QPushButton::clicked, this, [this]() { emit selectPreviousSentence(); });
  connect(nextButton, &QPushButton::clicked, this, [this]() { emit selectNextSentence(); });

  setLayout(vLayout);

  waveUpdateTimer.setInterval(25);
  waveUpdateTimer.setSingleShot(false);
  connect(&waveUpdateTimer, &QTimer::timeout, this, &AudioRecorder::waveUpdate);
}

AudioRecorder::~AudioRecorder()
{
  if(audioSink != nullptr) {
    audioSink->stop();
  }
  if(audioSource != nullptr) {
    audioSource->stop();
  }
}

void AudioRecorder::loadFromDisk(const QString &id)
{
  stopPlaying();

  audioData.clear();
  QString wavFileString = settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav";
  if(QFileInfo::exists(wavFileString)) {
    qInfo("Loading wav with id '%s' into audio recorder", qPrintable(id));
    int wavSamplerate = 0;
    audioData = loadWav(wavFileString, &wavSamplerate);
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
  waveformWidget->reset();

  settings.currentSentenceId = id;
}

bool AudioRecorder::saveToDisk(const QString &id)
{
  if(audioData.isEmpty()) {
    qDebug("audioData is empty, not saving to disk.");
    return true;
  }
  qInfo("Saving wav to disk with id '%s'", qPrintable(id));
  if(saveWav(settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav", audioData, settings.samplerate)) {
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
    stopPlaying();
    if(settings.currentSentenceId.isEmpty()) {
      recordButton->setChecked(false);
      return;
    }
    recordButton->setText(tr("Stop"));
    recordButton->setIcon(QIcon(":stop.png"));

    emit disableSentenceList();
    refreshDevicesButton->setEnabled(false);
    devicesCombo->setEnabled(false);
    samplerateCombo->setEnabled(false);
    recordButton->setEnabled(false);
    playButton->setEnabled(false);
    nextButton->setEnabled(false);
    prevButton->setEnabled(false);

    startRecording();
  } else {
    stopRecording();
    recordButton->setText(tr("Record"));
    recordButton->setIcon(QIcon(":record.png"));

    emit enableSentenceList();
    refreshDevicesButton->setEnabled(true);
    devicesCombo->setEnabled(true);
    samplerateCombo->setEnabled(true);
    playButton->setEnabled(true);
    nextButton->setEnabled(true);
    prevButton->setEnabled(true);
  }
}

void AudioRecorder::startRecording()
{
  if(audioSource == nullptr) {
    return;
  }

  qInfo("Starting recording!");
  audioData.clear();

  waveformWidget->setState(SR::INIT);
  audioIn = audioSource->start();

  connect(audioIn, &QIODevice::readyRead, this, &AudioRecorder::appendAudioData);
}

void AudioRecorder::appendAudioData()
{
  recordButton->setEnabled(true);
  QAudioFormat::SampleFormat sampleFormat = audioSource->format().sampleFormat();
  QByteArray data = audioIn->readAll();

  QVector<float> incomingAudioData;

  if(sampleFormat == QAudioFormat::UInt8) {
    const quint8* samples = reinterpret_cast<const quint8*>(data.constData());
    qint64 sampleCount = data.size() / sizeof(quint8);

    incomingAudioData.reserve(sampleCount);
      
    for(int i = 0; i < sampleCount; ++i) {
      float sample = (samples[i] / 128.0) - 1.0; // Convert to float format
      incomingAudioData.append(sample);
    }
  } else if(sampleFormat == QAudioFormat::Int16) {
    const qint16* samples = reinterpret_cast<const qint16*>(data.constData());
    qint64 sampleCount = data.size() / sizeof(qint16);

    incomingAudioData.reserve(sampleCount);
      
    for(int i = 0; i < sampleCount; ++i) {
      float sample = (samples[i] / 32768.0);  // Convert to float format
      incomingAudioData.append(sample);
    }
  } else if(sampleFormat == QAudioFormat::Int32) {
    const qint32* samples = reinterpret_cast<const qint32*>(data.constData());
    qint64 sampleCount = data.size() / sizeof(qint32);

    incomingAudioData.reserve(sampleCount);
      
    for(int i = 0; i < sampleCount; ++i) {
      float sample = (samples[i] / 2147483648.0); // Convert to float format
      incomingAudioData.append(sample);
    }
  } else if(sampleFormat == QAudioFormat::Float) {
    const float* samples = reinterpret_cast<const float*>(data.constData());
    qint64 sampleCount = data.size() / sizeof(float);

    incomingAudioData.reserve(sampleCount);
      
    for(int i = 0; i < sampleCount; ++i) {
      float sample = (samples[i]);
      incomingAudioData.append(sample);
    }
  }
  // Check if audio has settled around 0.0 at least once. This probably means interface has settled into meaningful audio
  if(audioData.isEmpty()) {
    bool foundAbove = false;
    for(const auto &sample: incomingAudioData) {
      if(sample > 0.0) {
	foundAbove = true;
	break;
      }
    }
    bool foundBelow = false;
    for(const auto &sample: incomingAudioData) {
      if(sample < 0.0) {
	foundBelow = true;
	break;
      }
    }
    if(foundAbove && foundBelow) {
      audioData.append(incomingAudioData);
      waveformWidget->setState(SR::DATA);
    }
  } else {
    audioData.append(incomingAudioData);
  }

  waveUpdateTimer.start();
}

void AudioRecorder::stopRecording()
{
  if(audioSource == nullptr) {
    return;
  }
  qDebug("Stopping recording!");

  waveUpdateTimer.stop();

  if(audioIn != nullptr) {
    disconnect(audioIn, &QIODevice::readyRead, this, &AudioRecorder::appendAudioData);
    // Never delete audioIn as it points to the QIODevice inside of audioSource
    // But we can 'reset' it by setting our pointer to nullptr
    audioIn = nullptr;
  }

  audioSource->stop();

  // Make sure internal audioData are cleaned out so we are ready for the next recording
  audioSource->reset();
  
  if(settings.autoTrim) {
    audioData = AudioProcessor::cutSilence(audioData);
  }
  if(settings.autoNormalize) {
    audioData = AudioProcessor::normalize(audioData);
  }
  if(settings.autoFade) {
    audioData = AudioProcessor::fadeEnds(audioData);
  }

  waveformWidget->update();
  saveToDisk(settings.currentSentenceId);
}

void AudioRecorder::startPlaying()
{
  stopPlaying();
    
  qDebug("Starting playback!");
  
  QAudioFormat format;
  format.setSampleRate(settings.samplerate);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Float);

  const char* dataPtr = reinterpret_cast<const char*>(audioData.constData());
  qsizetype byteCount = audioData.size() * sizeof(float);
  
  outBuffer.setData(dataPtr, byteCount);
  outBuffer.open(QIODevice::ReadOnly);

  QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
  
  audioSink = new QAudioSink(outputDevice, format, this);
  connect(audioSink, &QAudioSink::stateChanged, this, &AudioRecorder::audioSinkStateChanged);
  audioSink->start(&outBuffer);
}

void AudioRecorder::stopPlaying()
{
  if(audioSink != nullptr) {
    audioSink->stop();
  }
}

void AudioRecorder::waveUpdate()
{
  if(outBuffer.isOpen()) {
    waveformWidget->setPlayheadPos(((audioSink->elapsedUSecs() / 1000) * (settings.samplerate / 1000)) * sizeof(float));
  } else {
    waveformWidget->update();
  }
}

void AudioRecorder::audioSinkStateChanged(QAudio::State state)
{
  if(state == QAudio::ActiveState) {
    waveUpdateTimer.start();
  } else {
    waveUpdateTimer.stop();
    waveformWidget->setPlayheadPos(0);
    outBuffer.close();
    delete audioSink;
    audioSink = nullptr;
  }
}

void AudioRecorder::refreshInputDevices()
{
  devicesCombo->clear();
  for(const auto &device: QMediaDevices::audioInputs()) {
    devicesCombo->addItem(device.description(), device.id());
  }

  QByteArray inputDeviceId = iniSettings->value("audio/inputDeviceId", "").toByteArray();
  int inputDeviceIdx = devicesCombo->findData(inputDeviceId);
  if(inputDeviceIdx != -1) {
    devicesCombo->setCurrentIndex(devicesCombo->findData(inputDeviceId));
  } else {
    devicesCombo->setCurrentIndex(devicesCombo->findData(QMediaDevices::defaultAudioInput().id()));
  }

  inputDeviceChanged(devicesCombo->currentIndex());
}

void AudioRecorder::inputDeviceChanged(int index)
{
  QByteArray deviceId = devicesCombo->currentData().toByteArray();
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
