#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QThread>
#include "qt_logger.hpp"
#include "logger.hpp"
#include "process_params.hpp"

class ProcessThread : public QThread {

  Q_OBJECT

 public:
  explicit ProcessThread(QObject *parent = 0);
  ~ProcessThread();
  void process(Parameters params);
  void abortProcess();

 protected:
  void run() Q_DECL_OVERRIDE;

 signals:
  void processComplete();

 public slots:

 private:
  Parameters params;
  QtOutputSink stdoutSink;
  QtOutputSink stderrSink;
  bool abort;
};

#endif // PROCESSTHREAD_H
