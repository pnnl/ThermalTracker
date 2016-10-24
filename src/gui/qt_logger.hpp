#ifndef QT_LOGGER_HPP
#define QT_LOGGER_HPP

#include <string>
#include <QEvent>
#include <boost/iostreams/concepts.hpp>
#include "ui_mainwindow.h"

class ProcessingMessageEvent : public QEvent {
 public:
  ProcessingMessageEvent(std::string message, bool error);
  std::string msg() const;
  bool error() const;
  static const QEvent::Type eventType;
 
 private:
  std::string msg_;
  bool error_;
};


class QtOutputSink : public boost::iostreams::sink {
 public:
  QtOutputSink(QObject *receiver, bool error);
  std::streamsize write(const char *s, std::streamsize n);

 private:
  QObject *receiver_;
  bool error_;
};

#endif
