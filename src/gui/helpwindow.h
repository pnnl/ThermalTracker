#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class HelpWindow;
}

class HelpWindow : public QDialog
{
  Q_OBJECT

public:
  explicit HelpWindow(QWidget *parent = 0);
  ~HelpWindow();
  void setSettings(const QSettings &settings);

private:
  QSettings settings;
  Ui::HelpWindow *ui;
};

#endif // HELPWINDOW_H
