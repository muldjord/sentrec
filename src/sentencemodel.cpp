#include "sentencemodel.h"
#include "globaldefs.h"
#include "mainwindow.h"
#include "settings.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpression>
#include <QApplication>

extern MainWindow *mainWindow;
extern Settings settings;

SentenceModel::SentenceModel(const QVector<QVector<QString> > &data, QObject *parent)
  : QAbstractTableModel(parent)
{
  this->tableData = data;
  headers.append("ID");
  headers.append("Sentence");
}

int SentenceModel::rowCount(const QModelIndex &) const
{
  return tableData.size();
}

int SentenceModel::columnCount(const QModelIndex &) const
{
  if(tableData.isEmpty()) {
    return 0;
  }
  return tableData[0].size();
}

QVariant SentenceModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid()) {
    return QVariant();
  }

  if(role == Qt::DisplayRole) {
    return QVariant(tableData[index.row()][index.column()]);
  } else if(role == Qt::EditRole) {
    // Only allow edit if this is the sentence column
    if(index.column() == SR::SENT_COL) {
      return QVariant(tableData[index.row()][index.column()]);
    }
  } else if(role == Qt::ForegroundRole) {
    if(index.column() == SR::SENT_COL) {
      return QVariant(QBrush(QColor(255, 255, 255)));
    }
  } else if(role == Qt::BackgroundRole) {
    if(index.column() == SR::SENT_COL) {
      return QVariant(QBrush(QColor(0, 0, 0)));
    }
  }
  return QVariant();
}

QVariant SentenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(!tableData.isEmpty()) {
    if(role == Qt::DisplayRole) {
      if(orientation == Qt::Horizontal) {
        return headers[section];
      } else {
        return QString::number(tableData.size());
      }
    }
  }
  return QVariant();
}

Qt::ItemFlags SentenceModel::flags(const QModelIndex &index) const
{
  if(!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  if(index.column() == SR::SENT_COL) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool SentenceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid() ||
     role != Qt::EditRole ||
     index.column() != SR::SENT_COL) {
    return false;
  }

  tableData[index.row()][index.column()] = value.toString();
  emit dataChanged(index, index); // Inform the model that this index has changed
  return true;
}
