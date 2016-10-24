#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <fstream>
#include <memory>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMainWindow>
#include "helpwindow.h"
#include "process_params.hpp"
#include "processthread.hpp"

namespace Ui {

class MainWindow;

}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  bool event(QEvent *e);

  static const QColor STDERR_TEXT_COLOR;
  static const QColor STDOUT_TEXT_COLOR;

private:
  Parameters getProcessParams() const;
  void handleMsgEvent(const QEvent *e);
  void setFormItemsEnabled(bool enabled);
  void setValidators();
  void setFormValues(QString filepath);
  void setLogTextColor(QBrush brush);

  std::unique_ptr<QDoubleValidator> doubleValidator;
  std::unique_ptr<QDoubleValidator> doubleFracValidator;
  std::unique_ptr<QIntValidator> intValidator;
  std::unique_ptr<HelpWindow> helpWindow;
  std::ofstream logFile;
  Parameters params;
  std::unique_ptr<ProcessThread> processThread;
  QSettings settings;
  Ui::MainWindow *ui;

private slots:
  void browseInput();
  void browseOutput();
  void start();
  void stop();
  void on_actionHelp_triggered();
  void on_actionAbout_triggered();
  void on_actionSave_triggered();
  void on_actionLoad_triggered();
  void on_actionRestore_defaults_triggered();
  void processing_completed();
};

#endif // MAINWINDOW_H
