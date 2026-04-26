#include "sentencelist.h"
#include "sentencemodel.h"
#include "mainwindow.h"
#include "settings.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QHeaderView>

extern MainWindow *mainWindow;
extern Settings settings;

SentenceList::SentenceList(QWidget *parent)
  : QWidget(parent)
{
  sentenceView = new QTableView();
  sentenceView->setSelectionBehavior(QTableView::SelectRows);
  sentenceView->setSelectionMode(QTableView::SingleSelection);
  sentenceView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  sentenceModel = new SentenceModel(sentenceView);
  sentenceView->setModel(sentenceModel);
  connect(sentenceModel, &SentenceModel::dataChanged, this, &SentenceList::saveSentences);

  // When the sentence selection changes
  connect(sentenceView->selectionModel(), &QItemSelectionModel::currentChanged, this, &SentenceList::selectionChanged);
  
  QPushButton *loadSentencesButton = new QPushButton(QIcon(":load.png"), tr("Load sentences..."), this);
  loadSentencesButton->setIconSize(QSize(32, 32));
  connect(loadSentencesButton, &QPushButton::clicked, this, &SentenceList::loadSentences);

  QPushButton *deleteSentenceButton = new QPushButton(QIcon(":delete.png"), tr("Delete sentence"), this);
  deleteSentenceButton->setIconSize(QSize(32, 32));
  connect(deleteSentenceButton, &QPushButton::clicked, this, &SentenceList::deleteSentence);

  progressLabel = new QLabel("0 / 0");
  
  QVBoxLayout *buttonLayout = new QVBoxLayout();
  buttonLayout->addWidget(loadSentencesButton);
  buttonLayout->addWidget(deleteSentenceButton);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(progressLabel);

  QHBoxLayout *layout = new QHBoxLayout();
  layout->addWidget(sentenceView);
  layout->addLayout(buttonLayout);

  setLayout(layout);
}

void SentenceList::loadSentences()
{
  if(mainWindow->isWindowModified()) {
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Unsaved sentence edits!"),
							       tr("You have unsaved sentence edits!\nAre you sure you want to load a new sentence file?"),
							       QMessageBox::Yes | QMessageBox::No,
							       QMessageBox::No);
    if(button == QMessageBox::No) {
      return;
    }
  }
  QString sentenceFileString = QFileDialog::getOpenFileName(this, tr("Select CSV file with 'ID|Sentence' format"), ".", tr("CSV files (*.csv)"));
  if(sentenceFileString.isNull()) {
    qInfo("Loading cancelled!");
    return;
  }

  settings.sentenceFileInfo = QFileInfo(sentenceFileString);

  QFile sentenceFile(settings.sentenceFileInfo.absoluteFilePath());
  QVector<CellData> sentences;
  if(sentenceFile.open(QIODevice::ReadOnly)) {
    if(settings.csvBackup) {
      QString backupFileString = sentenceFile.fileName() + ".bu" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
      if(QFile::copy(sentenceFile.fileName(), backupFileString)) {
	qInfo("Created sentence backup file '%s'...", qPrintable(backupFileString));
      } else {
	qCritical("Couldn't create CSV backup file '%s', sentence loading cancelled!", qPrintable(backupFileString));
	QMessageBox::critical(this, tr("Couldn't create backup!"),
			      tr("CSV backup is enabled but backup file '") + backupFileString + tr("' could not be created!\n\nSentence loading cancelled!"),
			      QMessageBox::Ok,
			      QMessageBox::Ok);
	return;
      }
    }

    qInfo("Loading sentences, please wait...");
    qint64 lineIdx = 1;
    while(!sentenceFile.atEnd()) {
      QString line = QString::fromUtf8(sentenceFile.readLine().trimmed());
      QVector<QString> cells = line.split('|');
      if(cells.count() != 2) {
	QMessageBox::critical(this, tr("Format error!"),
			      tr("One or more lines from the CSV file does not contain exactly 2 pipe-separated columns. Please check that your input CSV has the format of 'ID|Sentence'."),
			      QMessageBox::Ok,
			      QMessageBox::Ok);
	qWarning("Format error!\nLine: %llu, Data: '%s'\nExpected 2 columns / cells but got %llu, loading cancelled!", lineIdx, qPrintable(line), cells.count());
	return;
      }
      CellData cellData;
      cellData.id = cells[0];
      cellData.sentence = cells[1];
      sentences.append(cellData);
      lineIdx++;
    }
    sentenceFile.close();
    qDebug("Loaded %llu sentences, now setting data...", sentences.size());
    setSentences(sentences);
  } else {
    qWarning("Couldn't open sentence file '%s' for reading, can't load sentences...", qPrintable(sentenceFile.fileName()));
  }
}

void SentenceList::setSentences(const QVector<CellData> &data)
{
  if(data.isEmpty()) {
    qInfo("No sentences found in data...");
  }

  sentenceModel->setAllData(data);
  sentenceView->resizeColumnsToContents();
  sentenceView->horizontalHeader()->setStretchLastSection(true);

  mainWindow->setWindowModified(false);

  // Find and select the first sentence that does not have a corresponding wav file
  QString wavPath = settings.sentenceFileInfo.absolutePath() + "/wav";
  QDir wavDir(wavPath, "*.wav", QDir::Name, QDir::Files);
  if(!wavDir.exists() && !wavDir.mkpath(wavPath)) {
    QMessageBox::critical(this, tr("Output wav folder could not be created"),
			  tr("The wav output folder '") + wavPath + tr("' could not be created. Please check permissions!"),
			  QMessageBox::Ok);
  }
  
  QList<QString> wavIds;
  for(const auto &wavInfo: wavDir.entryInfoList()) {
    wavIds.append(wavInfo.baseName());
  }
  for(int i = 0; i < sentenceModel->getAllData().size(); ++i) {
    if(!wavIds.contains(sentenceModel->getAllData()[i].id)) {
      qDebug("Selecting id %s", qPrintable(sentenceModel->getAllData()[i].id));
      QModelIndex idx = sentenceModel->index(i, 0);
      sentenceView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      sentenceView->scrollTo(idx);
      break;
    }
  }
  progressLabel->setText("0 / " + QString::number(sentenceModel->rowCount()));
  emit sentencesLoaded();
}

void SentenceList::saveSentences()
{
  if(!settings.sentenceFileInfo.isFile()) {
    return;
  }

  qInfo("Saving all sentences back to '%s'...", qPrintable(settings.sentenceFileInfo.absoluteFilePath()));
  const QVector<CellData> &tableData = sentenceModel->getAllData();
  QFile sentenceFile(settings.sentenceFileInfo.absoluteFilePath());
  if(sentenceFile.open(QIODevice::WriteOnly)) {
    for(const auto &row: tableData) {
      QString line = row.id + "|" + row.sentence + "\n";
      sentenceFile.write(line.toUtf8());
    }
    sentenceFile.close();
    qInfo("Success!");
    mainWindow->setWindowModified(false);
  }
}

void SentenceList::deleteSentence()
{
  QList<QModelIndex> selectedRow = sentenceView->selectionModel()->selectedRows();
  
  if(selectedRow.isEmpty()) {
    qInfo("No sentences selected!");
    return;
  }
  
  int row = selectedRow.first().row();
  QString sentenceId = sentenceModel->getRowIdString(row);

  bool doDelete = true;
  if(settings.askDelete) {
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete sentence?"),
							       tr("Are you sure you want to delete sentence with id '") + sentenceId + tr("'? The corresponding wav file will also be deleted!"),
							       QMessageBox::Yes | QMessageBox::No,
							       QMessageBox::No);
    if(button == QMessageBox::No) {
      doDelete = false;
    }
  }

  if(!doDelete) {
    return;
  }

  emit deleteFromDisk(sentenceId);
  qInfo("Removing row %d", row);
  // This automatically calls removeRows (plural) in the selectionModel
  sentenceModel->removeRow(row);

  saveSentences();
}

void SentenceList::clearSentenceList()
{
  QVector<CellData> tempList;
  setSentences(tempList);
  progressLabel->setText("0 / 0");
}

void SentenceList::selectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
  /*
  if(previous.isValid() && sentenceModel->isDirty(previous.row())) {
    QString oldId = sentenceModel->getRowIdString(previous.row());
    qDebug("Leaving row %d with id '%s'.", previous.row(), qPrintable(oldId));
    emit leavingSentence(oldId);
    sentenceModel->setDirty(previous.row(), false);
  }
  */

  QString newId = sentenceModel->getRowIdString(current.row());
  qDebug("Entering row %d with id '%s'.", current.row(), qPrintable(newId));
  progressLabel->setText(QString::number(current.row() + 1) + " / " + QString::number(sentenceModel->rowCount()));
  emit enteringSentence(newId);
}

void SentenceList::selectPreviousSentence()
{
  sentenceAdvance(-1);
}

void SentenceList::selectNextSentence()
{
  sentenceAdvance(1);
}

void SentenceList::sentenceAdvance(const int &delta)
{
  QList<QModelIndex> selectedRow = sentenceView->selectionModel()->selectedRows();

  if(selectedRow.isEmpty()) {
    qInfo("No sentences selected, can't select next sentence!");
    return;
  }
  int currentRow = selectedRow[0].row();
  
  // Check if out of bounds
  if(currentRow + delta > sentenceModel->rowCount() - 1 ||
     currentRow + delta < 0) {
    qDebug("Sentence advance out of bounds, ignoring sentence change!");
    return;
  }

  QModelIndex idx = sentenceModel->index(currentRow + delta, 0);
  sentenceView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  sentenceView->scrollTo(idx);

}

void SentenceList::disableSentenceList()
{
  setEnabled(false);
}

void SentenceList::enableSentenceList()
{
  setEnabled(true);
}
