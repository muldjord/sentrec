#pragma once

#include "sentencemodel.h"

#include <QWidget>
#include <QTableView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>
#include <QFileInfo>
#include <QLabel>

class SentenceList : public QWidget
{
Q_OBJECT

public:
  SentenceList(QWidget *parent);

public slots:
  void loadSentences();
  void setSentences(const QVector<CellData> &data);
  void clearSentenceList();
  void selectPreviousSentence();
  void selectNextSentence();
  void disableSentenceList();
  void enableSentenceList();
  
signals:
  void sentencesLoaded();
  void enteringSentence(const QString &sentenceId);
  void deleteFromDisk(const QString &id);

private slots:
  void deleteSentence();
  void saveSentences();
  void selectionChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  void sentenceAdvance(const int &delta);
  SentenceModel *sentenceModel = nullptr;
  QTableView *sentenceView;

  QLabel *progressLabel = nullptr;
};
