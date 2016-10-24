#include <algorithm>
#include <iostream>
#include <string>
#include <QEvent>
#include "qt_logger.hpp"

const QEvent::Type ProcessingMessageEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType(1000));

ProcessingMessageEvent::ProcessingMessageEvent(std::string message, bool error)
    : QEvent(eventType), msg_(message), error_(error) {
}

bool ProcessingMessageEvent::error() const {
  return error_;
}

std::string ProcessingMessageEvent::msg() const {
  return msg_;
}

QtOutputSink::QtOutputSink(QObject *receiver, bool error)
    : receiver_(receiver), error_(error) {
}

std::streamsize QtOutputSink::write(const char *s, std::streamsize n) {
  std::string msg(s, n);
  ProcessingMessageEvent *event = new ProcessingMessageEvent(msg, error_);

  QCoreApplication::postEvent(receiver_, event);

  return n;
}
