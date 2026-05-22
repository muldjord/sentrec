#include "batchprocessor.h"
#include "audioprocessor.h"
#include "settings.h"
#include "wavhandler.h"

#include <QDebug>
#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

extern QSettings *iniSettings;
extern Settings settings;

BatchProcessor::BatchProcessor(QWidget *parent)
  : QDialog(parent)
{
  setMinimumWidth(1024);
  QHBoxLayout *inputPathLayout = new QHBoxLayout;
  QLabel *inputPathLabel = new QLabel(tr("Input path:"));
  inputPathLineEdit = new QLineEdit(this);
  inputPathLineEdit->setReadOnly(true);
  QPushButton *inputPathButton = new QPushButton("...", this);
  connect(inputPathButton, &QPushButton::clicked, this, &BatchProcessor::chooseInputPath);
  inputPathLayout->addWidget(inputPathLabel);
  inputPathLayout->addWidget(inputPathLineEdit);
  inputPathLayout->addWidget(inputPathButton);

  QHBoxLayout *outputPathLayout = new QHBoxLayout;
  QLabel *outputPathLabel = new QLabel(tr("Output path:"));
  outputPathLineEdit = new QLineEdit(this);
  outputPathLineEdit->setReadOnly(true);
  QPushButton *outputPathButton = new QPushButton("...", this);
  connect(outputPathButton, &QPushButton::clicked, this, &BatchProcessor::chooseOutputPath);
  outputPathLayout->addWidget(outputPathLabel);
  outputPathLayout->addWidget(outputPathLineEdit);
  outputPathLayout->addWidget(outputPathButton);

  QPushButton *processButton = new QPushButton("Process all!", this);
  connect(processButton, &QPushButton::clicked, this, &BatchProcessor::processAll);

  QPushButton *cancelButton = new QPushButton("Cancel", this);
  connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(inputPathLayout);
  layout->addLayout(outputPathLayout);
  layout->addWidget(processButton);
  layout->addWidget(cancelButton);

  setLayout(layout);
}

BatchProcessor::~BatchProcessor()
{
}

void BatchProcessor::chooseInputPath()
{
  inputPathLineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose wav audio file input path")));
}

void BatchProcessor::chooseOutputPath()
{
  QString outputPath = QFileDialog::getExistingDirectory(this, tr("Choose wav audio file output path"));
  if(outputPath != inputPathLineEdit->text()) {
    outputPathLineEdit->setText(outputPath);
  } else {
    QMessageBox::warning(this, tr("Error"), tr("Input and output path can't be the same. The processed audio files will have the same names as the audio files from the input path. Please choose a different output path."));
  }
}

void BatchProcessor::processAll()
{
  QDir inputDir(inputPathLineEdit->text(), "*.wav", QDir::Name, QDir::Files | QDir::NoDotAndDotDot);
  QString outputPath = outputPathLineEdit->text();

  for(const auto &fileInfo: inputDir.entryInfoList()) {
    QString inFile = fileInfo.absoluteFilePath();
    QString outFile = outputPath + "/" + fileInfo.fileName();
    qDebug("Processing file: %s", qPrintable(inFile));

    QVector<float> audioData;
    int wavSamplerate = 0;
    if(QFileInfo::exists(inFile)) {
      qInfo("Loading wav: %s", qPrintable(inFile));
      audioData = loadWav(inFile, &wavSamplerate);
    }

    qInfo("Processing wav...");
    audioData = AudioProcessor::cutSilence(audioData);
    audioData = AudioProcessor::normalize(audioData);
    audioData = AudioProcessor::fadeEnds(audioData);

    if(audioData.isEmpty()) {
      qDebug("audioData is empty, not saving to disk.");
      continue;
    }
    qInfo("Saving wav to disk: %s", qPrintable(outFile));
    if(!saveWav(outFile, audioData, wavSamplerate)) {
      qDebug("Could not save to disk!");
    }
  }
}
/*
void BatchProcessor::processFile(const QString &id)
{
  QString wavFileString = settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav";
  if(QFileInfo::exists(wavFileString)) {
    qInfo("Loading wav with id '%s' into audio recorder", qPrintable(id));
    int wavSamplerate = 0;
    audioData = loadWav(wavFileString, &wavSamplerate);
    if(settings.samplerateWarning &&
       settings.samplerate != wavSamplerate) {
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

  if(audioSink == nullptr && settings.autoPlay) {
    startPlaying();
  }
}

bool BatchProcessor::saveToDisk(const QString &id)
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

void BatchProcessor::deleteFromDisk(const QString &id)
{
  QString wavFileString = settings.sentenceFileInfo.absolutePath() + "/wav/" + id + ".wav";
  if(QFileInfo::exists(wavFileString)) {
    qInfo("Deleting wav with id '%s' from disk", qPrintable(id));
    QFile::remove(wavFileString);
  }
}

void BatchProcessor::toggleRecording()
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

void BatchProcessor::startRecording()
{
  if(audioSource == nullptr) {
    return;
  }

  qInfo("Starting recording!");
  audioData.clear();

  waveformWidget->setState(SR::INIT);
  audioIn = audioSource->start();

  connect(audioIn, &QIODevice::readyRead, this, &BatchProcessor::appendAudioData);
}

void BatchProcessor::appendAudioData()
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

void BatchProcessor::stopRecording()
{
  if(audioSource == nullptr) {
    return;
  }
  qDebug("Stopping recording!");

  waveUpdateTimer.stop();

  if(audioIn != nullptr) {
    disconnect(audioIn, &QIODevice::readyRead, this, &BatchProcessor::appendAudioData);
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

void BatchProcessor::startPlaying()
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
  connect(audioSink, &QAudioSink::stateChanged, this, &BatchProcessor::audioSinkStateChanged);
  audioSink->start(&outBuffer);
}

void BatchProcessor::stopPlaying()
{
  if(audioSink != nullptr) {
    audioSink->stop();
  }
}

void BatchProcessor::waveUpdate()
{
  if(outBuffer.isOpen()) {
    waveformWidget->setPlayheadPos(((audioSink->elapsedUSecs() / 1000) * (settings.samplerate / 1000)) * sizeof(float));
  } else {
    waveformWidget->update();
  }
}

void BatchProcessor::audioSinkStateChanged(QAudio::State state)
{
  if(state == QAudio::ActiveState) {
    waveUpdateTimer.start();
  } else if(state == QAudio::IdleState) {
    qDebug("Entered idle state, stopping playback!");
    audioSink->stop();
  } else if(state == QAudio::StoppedState) {
    qDebug("Entered stopped state, resetting output!");
    waveUpdateTimer.stop();
    waveformWidget->setPlayheadPos(0);
    outBuffer.close();
    delete audioSink;
    audioSink = nullptr;
  }
}

void BatchProcessor::refreshInputDevices()
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

void BatchProcessor::inputDeviceChanged(int index)
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

void BatchProcessor::samplerateChanged(int index)
{
  int samplerate = samplerateCombo->itemData(index).toInt();
  iniSettings->setValue("audio/samplerate", samplerate);
  settings.samplerate = samplerate;

  setInputDevice();
}

void BatchProcessor::setInputDevice()
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
*/
