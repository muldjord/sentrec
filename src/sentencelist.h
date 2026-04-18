#pragma once

#include "sentencemodel.h"

#include <QWidget>
#include <QTableView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>

class SentenceList : public QWidget
{
Q_OBJECT

public:
  SentenceList(QWidget *parent);

public slots:
  void setSentences(const QVector<QVector<QString> > &data);
  void clearSentenceList();

private slots:
  void loadSentences();
  void saveSentences();
  void removeSentence();

private:
  QString sentenceFileString = "";
  SentenceModel *sentenceModel = nullptr;
  QTableView *sentenceView;

};
