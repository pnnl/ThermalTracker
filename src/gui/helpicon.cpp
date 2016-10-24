#include "helpicon.hpp"
#include <QMouseEvent>
#include <QToolTip>

HelpIcon::HelpIcon(QWidget *parent) : QLabel(parent){
  setMouseTracking(true);
}

bool HelpIcon::event(QEvent *e) {
  if(e->type() == QEvent::ToolTip) {
    return false;
  }

  QLabel::event(e);
}

void HelpIcon::mouseMoveEvent(QMouseEvent *event) {
  QToolTip::showText(event->globalPos(), this->toolTip());
}
