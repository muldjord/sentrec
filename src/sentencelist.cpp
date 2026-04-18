#include "sentencelist.h"
#include "sentencemodel.h"
#include "mainwindow.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QHeaderView>

extern MainWindow *mainWindow;

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

  QPushButton *loadSentencesButton = new QPushButton(QIcon(":remove.png"), tr("Load sentences..."), this);
  loadSentencesButton->setIconSize(QSize(16, 16));
  connect(loadSentencesButton, &QPushButton::clicked, this, &SentenceList::loadSentences);

  QPushButton *deleteSentenceButton = new QPushButton(QIcon(":remove.png"), tr("Delete selected sentence"), this);
  deleteSentenceButton->setIconSize(QSize(16, 16));
  connect(deleteSentenceButton, &QPushButton::clicked, this, &SentenceList::deleteSentence);

  QVBoxLayout *buttonLayout = new QVBoxLayout();
  buttonLayout->addWidget(loadSentencesButton);
  buttonLayout->addWidget(deleteSentenceButton);
  buttonLayout->addStretch(1);

  QHBoxLayout *layout = new QHBoxLayout();
  layout->addWidget(sentenceView);
  layout->addLayout(buttonLayout);

  setLayout(layout);
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
  sentenceFileString = QFileDialog::getOpenFileName(this, tr("Select CSV file with 'ID|Sentence' format"), ".", tr("CSV files (*.csv)"));
  if(sentenceFileString.isNull()) {
    qInfo("Loading cancelled!");
    return;
  }
  QFile sentenceFile(sentenceFileString);
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

void SentenceList::saveSentences()
{
  if(sentenceFileString.isEmpty()) {
    return;
  }

  qInfo("Saving all sentences back to '%s'...", qPrintable(sentenceFileString));
  const QVector<QVector<QString> > &tableData = sentenceModel->getAllData();
  QFile sentenceFile(sentenceFileString);
  if(sentenceFile.open(QIODevice::WriteOnly)) {
    for(const auto &row: tableData) {
      if(row.size() != 2) {
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

void SentenceList::deleteSentence()
{
  if(sentenceView->selectionModel() == nullptr) {
    return;
  }
  /*
  if(currentSentence.getSentenceUid() != MMK::NONE) {
    QModelIndexList selected = sentenceView->selectionModel()->selectedRows();
    if(!selected.isEmpty()) {
      SentenceModel *sentenceModel = (SentenceModel *)sentenceView->model();
      QList<SentenceData> sentences = sentenceModel->getRows(selected);
      if(!sentences.isEmpty() && sentences.count() == 1) {
        if(QMessageBox::question(this, tr("Delete sentence?"), tr("Are you sure you wish to delete the selected sentence?")) == QMessageBox::Yes) {
          emit requestDeleteSentence(sentences.at(0).getSentenceUid());
        }
      }
    }
  }
  */
}

void SentenceList::clearSentenceList()
{
  QVector<QVector<QString> > tempList;
  setSentences(tempList);
}
