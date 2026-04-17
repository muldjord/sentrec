#include "sentencelist.h"
#include "sentencemodel.h"
#include "mainwindow.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>

extern MainWindow *mainWindow;

SentenceList::SentenceList(QWidget *parent)
  : QWidget(parent)
{
  sentenceView = new QTableView();
  // Set row selection color and cell selection highlight color. Default highlight color is very hard to distinguish.
  // Read more here: https://doc.qt.io/qt-5/stylesheet-reference.html
  sentenceView->setStyleSheet("QTableView::item {"
                          "selection-background-color: #458ccb;"
                          "}"
                          "QTableView::item:focus {"
                          "selection-background-color: #57b0ff;"
                          "},"
                          );
  sentenceView->setSelectionBehavior(QTableView::SelectRows);

  QPushButton *deleteSentenceButton = new QPushButton(QIcon(":remove.png"),
                                                       tr("Delete selected sentence"), this);
  deleteSentenceButton->setIconSize(QSize(16, 16));
  connect(deleteSentenceButton, &QPushButton::clicked, this, &SentenceList::deleteSentence);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(deleteSentenceButton);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(sentenceView);
  layout->addLayout(buttonLayout);

  setLayout(layout);
}

void SentenceList::setSentences(const QVector<QVector<QString> > &data)
{
  if(data.isEmpty()) {
    qInfo("No sentences found on sentence...\n");
  }
  //sentenceView->model()->deleteLater();
  QAbstractItemModel *oldModel = sentenceView->model();
  sentenceModel = new SentenceModel(data, sentenceView);
  sentenceView->setModel(sentenceModel);
  delete oldModel;
  sentenceView->resizeColumnsToContents();
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
