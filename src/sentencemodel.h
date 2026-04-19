#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariant>
#include <QEventLoop>

struct CellData {
  QString id = "";
  QString sentence = "";
  bool dirty = false;
};

class SentenceModel : public QAbstractTableModel
{
Q_OBJECT
  
public:
  SentenceModel(QObject *parent = 0);
  
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  void setAllData(const QVector<CellData> &data);
  const QVector<CellData> &getAllData() const;
  const QString &getRowIdString(const qint64 &row) const;
  void setDirty(const qint64 &row, const bool &dirty = true);
  const bool &isDirty(const qint64 &row) const;

  //bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
  //bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
  //bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
  //bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());

private:
  QVector<QString> headers;
  QVector<CellData> tableData;
};
