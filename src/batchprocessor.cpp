#include "batchprocessor.h"
#include "audioprocessor.h"
#include "settings.h"
#include "wavhandler.h"

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>

extern QSettings *iniSettings;
extern Settings settings;

BatchProcessor::BatchProcessor(QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Audio batch processing"));
  setMinimumWidth(700);
  setWindowIcon(QIcon(":batchprocess.png"));

  QGridLayout *pathLayout = new QGridLayout;

  QLabel *inputPathLabel = new QLabel(tr("Input path:"));
  inputPathLineEdit = new QLineEdit(this);
  inputPathLineEdit->setReadOnly(true);
  QPushButton *inputPathButton = new QPushButton(QIcon(":load.png"), "", this);
  connect(inputPathButton, &QPushButton::clicked, this, &BatchProcessor::chooseInputPath);

  pathLayout->addWidget(inputPathLabel, 0, 0);
  pathLayout->addWidget(inputPathLineEdit, 0, 1);
  pathLayout->addWidget(inputPathButton, 0, 2);

  QLabel *outputPathLabel = new QLabel(tr("Output path:"));
  outputPathLineEdit = new QLineEdit(this);
  outputPathLineEdit->setReadOnly(true);
  QPushButton *outputPathButton = new QPushButton(QIcon(":load.png"), "", this);
  connect(outputPathButton, &QPushButton::clicked, this, &BatchProcessor::chooseOutputPath);

  pathLayout->addWidget(outputPathLabel, 1, 0);
  pathLayout->addWidget(outputPathLineEdit,1, 1);
  pathLayout->addWidget(outputPathButton, 1, 2);

  trimCheckBox = new QCheckBox(tr("Apply audio trimming filter"), this);
  trimCheckBox->setChecked(true);
  normalizeCheckBox = new QCheckBox(tr("Apply normalization filter"), this);
  normalizeCheckBox->setChecked(true);
  fadeCheckBox = new QCheckBox(tr("Apply fade-in and fade-out filter"), this);
  fadeCheckBox->setChecked(true);

  QPushButton *processButton = new QPushButton(QIcon(":batchprocess.png"), tr("Start processing!"), this);
  connect(processButton, &QPushButton::clicked, this, &BatchProcessor::processAll);

  QPushButton *closeButton = new QPushButton("Close", this);
  connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(processButton);
  buttonLayout->addWidget(closeButton);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(pathLayout);
  layout->addWidget(trimCheckBox);
  layout->addWidget(normalizeCheckBox);
  layout->addWidget(fadeCheckBox);
  layout->addLayout(buttonLayout);
  
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
    outputPathLineEdit->setText("");
  }
}

void BatchProcessor::processAll()
{
  if(inputPathLineEdit->text().isEmpty() ||
     outputPathLineEdit->text().isEmpty()) {
    QMessageBox::warning(this, tr("Missing path(s)"), tr("Input and / or output path has not been set!\nPlease set both of them before initiating audio processing."));
    return;
  }

  if(!trimCheckBox->isChecked() &&
     !normalizeCheckBox->isChecked() &&
     !fadeCheckBox->isChecked()) {
    QMessageBox::warning(this, tr("No filters selected"), tr("Please select one or more filters to apply to the audio input files."));
    return;
  }

  QDir inputDir(inputPathLineEdit->text(), "*.wav", QDir::Name, QDir::Files | QDir::NoDotAndDotDot);
  QDir outputDir(outputPathLineEdit->text(), "*.wav", QDir::Name, QDir::Files | QDir::NoDotAndDotDot);
  QString outputPath = outputPathLineEdit->text();
  
  QList<QFileInfo> inInfoList = inputDir.entryInfoList();
  QList<QFileInfo> outInfoList = outputDir.entryInfoList();

  if(inInfoList.isEmpty()) {
    QMessageBox::information(this, tr("No input files"), tr("The input folder doesn't contain any wav files."));
    return;
  }
  
  bool breakOnThroughToTheOtherSide = false;
  for(const auto &outFileInfo: outInfoList) {
    for(const auto &inFileInfo: inInfoList) {
      if(outFileInfo.fileName() == inFileInfo.fileName()) {
	if(QMessageBox::question(this, tr("Matching files already exist!"), tr("Matching files already exist in the output path. Would you like to overwrite them?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
	  return;
	} else {
	  breakOnThroughToTheOtherSide = true;
	  break;
	}
      }
    }
    if(breakOnThroughToTheOtherSide) {
      break;
    }
  }

  QProgressDialog progressDialog(tr("Processing audio files..."), tr("Cancel"), 0, inInfoList.length(), this);
  progressDialog.setWindowModality(Qt::WindowModal);

  int progress = 0;
  bool cancelled = false;
  for(const auto &fileInfo: inInfoList) {
    progressDialog.setValue(progress);
    progress++;
    progressDialog.setLabelText(tr("Processing: ") + fileInfo.fileName());
    qApp->processEvents();
    
    if(progressDialog.wasCanceled()) {
      cancelled = true;
      break;
    }

    QString inFile = fileInfo.absoluteFilePath();
    QString outFile = outputPath + "/" + fileInfo.fileName();
    qDebug("\nProcessing file: %s", qPrintable(inFile));

    QVector<float> audioData;
    int wavSamplerate = 0;
    if(QFileInfo::exists(inFile)) {
      qInfo("Loading wav: %s", qPrintable(inFile));
      audioData = loadWav(inFile, &wavSamplerate);
    }

    qInfo("Processing wav...");
    if(trimCheckBox->isChecked()) {
      audioData = AudioProcessor::cutSilence(audioData, wavSamplerate);
    }
    if(normalizeCheckBox->isChecked()) {
      audioData = AudioProcessor::normalize(audioData);
    }
    if(fadeCheckBox->isChecked()) {
      audioData = AudioProcessor::fadeEnds(audioData, wavSamplerate);
    }

    if(audioData.isEmpty()) {
      qDebug("AudioData is empty! We're saving an empty wav file.");
    }
    qInfo("Saving wav to disk: %s", qPrintable(outFile));
    if(!saveWav(outFile, audioData, wavSamplerate)) {
      QMessageBox::warning(this, tr("Could not save to disk"), tr("Could not save file to disk, please check output folder permissions.\n\nOutput file: ") + outFile + "\n" + tr("Output path:") + outputPath);
      qDebug("Could not save to disk!");
    }
  }
  progressDialog.setValue(progressDialog.maximum());
  if(!cancelled) {
    QMessageBox::information(this, tr("Completed!"), tr("All audio files have been processed!"));
  }
}
