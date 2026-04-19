#pragma once

#include "sentencemodel.h"

#include <QWidget>
#include <QTableView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>
#include <QFileInfo>

class SentenceList : public QWidget
{
Q_OBJECT

public:
  SentenceList(QWidget *parent);

public slots:
  void setSentences(const QVector<CellData> &data);
  void clearSentenceList();
  void selectPreviousSentence();
  void selectNextSentence();

signals:
  void sentencesLoaded();
  void enteringSentence(const QString &sentenceId);

private slots:
  void loadSentences();
  void saveSentences();
  void deleteSentence();
  void selectionChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  void sentenceAdvance(const int &delta);
  SentenceModel *sentenceModel = nullptr;
  QTableView *sentenceView;

};
