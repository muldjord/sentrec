#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariant>
#include <QEventLoop>

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
  void setAllData(const QVector<QVector<QString> > &data);
  const QVector<QVector<QString> > &getAllData() const;
  
  //bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
  //bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
  //bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
  //bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());

private:
  QVector<QString> headers;
  QVector<QVector<QString> > tableData;
};
