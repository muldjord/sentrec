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

SentenceModel::SentenceModel(QObject *parent)
  : QAbstractTableModel(parent)
{
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
  return 2;
}

QVariant SentenceModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid()) {
    return QVariant();
  }

  int column = index.column();

  if(role == Qt::DisplayRole) {
    if(column == SR::ID_COL) {
      return QVariant(tableData[index.row()].id);
    } else if(column == SR::SENTENCE_COL) {
      return QVariant(tableData[index.row()].sentence);
    }
  } else if(role == Qt::EditRole) {
    // Only allow edit if this is the sentence column
    if(column == SR::SENTENCE_COL) {
      return QVariant(tableData[index.row()].sentence);
    }
  } else if(role == Qt::ForegroundRole) {
    if(column == SR::SENTENCE_COL) {
      return QVariant(QBrush(QColor(255, 255, 255)));
    }
  } else if(role == Qt::BackgroundRole) {
    if(column == SR::SENTENCE_COL) {
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
        return QString::number(section + 1);
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

  int column = index.column();

  if(column == SR::SENTENCE_COL) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool SentenceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid() ||
     role != Qt::EditRole ||
     index.column() != SR::SENTENCE_COL) {
    return false;
  }

  tableData[index.row()].sentence = value.toString();
  mainWindow->setWindowModified(true);
  emit dataChanged(index, index); // Inform the model that this index has changed
  return true;
}

bool SentenceModel::removeRows(int row, int count, const QModelIndex &parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  tableData.removeAt(row);
  endRemoveRows();

  return true;
}

void SentenceModel::setAllData(const QVector<CellData> &data)
{
  beginResetModel();
  tableData = data;
  endResetModel();
}

const QVector<CellData> &SentenceModel::getAllData() const
{
  return tableData;
}

const QString &SentenceModel::getRowIdString(const qint64 &row) const
{
  return tableData[row].id;
}
