#pragma once

#include <QDialog>
#include <QLineEdit>

class BatchProcessor : public QDialog
{
  Q_OBJECT

public:
  BatchProcessor(QWidget *parent = nullptr);
  ~BatchProcessor();

private slots:
  void chooseInputPath();
  void chooseOutputPath();
  void processAll();
  
private:
  QLineEdit *inputPathLineEdit = nullptr;
  QLineEdit *outputPathLineEdit = nullptr;
};
