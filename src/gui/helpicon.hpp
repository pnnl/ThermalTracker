#ifndef HELP_ICON_H
#define HELP_ICON_H

#include <QLabel>

class HelpIcon : public QLabel {
  Q_OBJECT
 public:
  explicit HelpIcon(QWidget *parent = 0);
  virtual bool event(QEvent *e);
 protected:
  virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif
