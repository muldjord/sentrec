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
  sentenceView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
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
    qInfo("No sentences found in data...\n");
  }
  if(sentenceModel == nullptr) {
    sentenceModel = new SentenceModel(sentenceView);
    sentenceView->setModel(sentenceModel);
  }
  sentenceModel->setAllData(data);
  sentenceView->resizeColumnsToContents();
  sentenceView->horizontalHeader()->setStretchLastSection(true);
}

void SentenceList::loadSentences()
{
  qInfo("Loading sentences...\n");
  QString sentenceFileString = QFileDialog::getOpenFileName(this, tr("Select CSV file with 'ID|Sentence' format"), ".", tr("CSV files (*.csv)"));
  if(sentenceFileString.isNull()) {
    qInfo("Loading cancelled!\n");
    return;
  }
  QFile sentenceFile(sentenceFileString);
  QVector<QVector<QString> > sentences;
  if(sentenceFile.open(QIODevice::ReadOnly)) {
    qInfo("Loading sentences, please wait...\n");
    qint64 lineIdx = 1;
    while(!sentenceFile.atEnd()) {
      QString line = QString::fromUtf8(sentenceFile.readLine().trimmed());
      QVector<QString> cells = line.split('|');
      if(cells.count() != 2) {
	qWarning("Format error!\nLine: %llu, Data: '%s'\nExpected 2 columns / cells but got %llu, loading cancelled!\n", lineIdx, qPrintable(line), cells.count());
	return;
      }
      sentences.append(cells);
      lineIdx++;
    }
    sentenceFile.close();
    setSentences(sentences);
  } else {
    qWarning("Couldn't open sentence file '%s' for reading, can't load sentences...\n", qPrintable(sentenceFile.fileName()));
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
