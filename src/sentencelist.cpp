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
  
  QPushButton *loadSentencesButton = new QPushButton(QIcon(":remove.png"), tr("Load sentences..."), this);
  loadSentencesButton->setIconSize(QSize(16, 16));
  connect(loadSentencesButton, &QPushButton::clicked, this, &SentenceList::loadSentences);

  QPushButton *deleteSentenceButton = new QPushButton(QIcon(":remove.png"), tr("Delete selected sentence"), this);
  deleteSentenceButton->setIconSize(QSize(16, 16));
  connect(deleteSentenceButton, &QPushButton::clicked, this, &SentenceList::removeSentence);

  QVBoxLayout *buttonLayout = new QVBoxLayout();
  buttonLayout->addWidget(loadSentencesButton);
  buttonLayout->addWidget(deleteSentenceButton);
  buttonLayout->addStretch(1);

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
  QVector<QVector<QString> > sentences;
  if(sentenceFile.open(QIODevice::ReadOnly)) {
    QString backupFileString = sentenceFile.fileName() + ".bu" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    if(QFile::copy(sentenceFile.fileName(), backupFileString)) {
      qInfo("Created sentence backup file '%s'...", qPrintable(backupFileString));
    } else {
      qCritical("Couldn't create backup file '%s', sentence loading cancelled!", qPrintable(backupFileString));
      return;
    }

    qInfo("Loading sentences, please wait...");
    qint64 lineIdx = 1;
    while(!sentenceFile.atEnd()) {
      QString line = QString::fromUtf8(sentenceFile.readLine().trimmed());
      QVector<QString> cells = line.split('|');
      if(cells.count() != 2) {
	qWarning("Format error!\nLine: %llu, Data: '%s'\nExpected 2 columns / cells but got %llu, loading cancelled!", lineIdx, qPrintable(line), cells.count());
	return;
      }
      sentences.append(cells);
      lineIdx++;
    }
    sentenceFile.close();
    setSentences(sentences);
  } else {
    qWarning("Couldn't open sentence file '%s' for reading, can't load sentences...", qPrintable(sentenceFile.fileName()));
  }
}

void SentenceList::setSentences(const QVector<QVector<QString> > &data)
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
    if(!wavIds.contains(sentenceModel->getAllData()[i][0])) {
      qDebug("Selecting id %s", qPrintable(sentenceModel->getAllData()[i][0]));
      QModelIndex idx = sentenceModel->index(i, 0);
      sentenceView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      sentenceView->scrollTo(idx);
      break;
    }
  }
}

void SentenceList::saveSentences()
{
  if(!settings.sentenceFileInfo.isFile()) {
    return;
  }

  qInfo("Saving all sentences back to '%s'...", qPrintable(settings.sentenceFileInfo.absoluteFilePath()));
  const QVector<QVector<QString> > &tableData = sentenceModel->getAllData();
  QFile sentenceFile(settings.sentenceFileInfo.absoluteFilePath());
  if(sentenceFile.open(QIODevice::WriteOnly)) {
    for(const auto &row: tableData) {
      if(row.size() != 2) {
	QMessageBox::critical(this, tr("Number of rows differ from 2!"),
			      tr("The number of rows in the internal data structure differs from 2! Something is very wrong!"),
			      QMessageBox::Ok);
	qCritical("Number of rows != 2!!! Something is very wrong!");
	return;
      }
      QString line = row[0] + "|" + row[1] + "\n";
      sentenceFile.write(line.toUtf8());
    }
    sentenceFile.close();
    qInfo("Success!");
    mainWindow->setWindowModified(false);
  }
}

void SentenceList::removeSentence()
{
  QList<QModelIndex> selectedRow = sentenceView->selectionModel()->selectedRows();
  
  if(selectedRow.isEmpty()) {
    qInfo("No sentences selected!");
    return;
  }
  
  int row = selectedRow.first().row();
  qInfo("Removing row %d", row);
  // This automatically calls removeRows (plural) in the selectionModel
  sentenceModel->removeRow(row);

  saveSentences();
}

void SentenceList::clearSentenceList()
{
  QVector<QVector<QString> > tempList;
  setSentences(tempList);
}

void SentenceList::selectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
  if(previous.isValid()) {
    QString oldId = sentenceModel->getRowIdString(previous.row());
    qDebug("Leaving row %d with id '%s'.", previous.row(), qPrintable(oldId));
    emit leavingSentence(oldId);
  }

  QString newId = sentenceModel->getRowIdString(current.row());
  qDebug("Entering row %d with id '%s'.", current.row(), qPrintable(newId));
  emit enteringSentence(newId);
}
