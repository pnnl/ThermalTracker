#include <QFileDialog>
#include <QAbstractButton>
#include <QFuture>
#include <QMessageBox>
#include <QCoreApplication>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "process_params.hpp"

const QColor MainWindow::STDERR_TEXT_COLOR = "darkRed";
// Darker gray
const QColor MainWindow::STDOUT_TEXT_COLOR(68, 68, 68, 255);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  processThread.reset(new ProcessThread(this));
  connect(processThread.get(), &ProcessThread::processComplete, this, &MainWindow::processing_completed);
  connect(ui->processButton, &QAbstractButton::clicked, this, &MainWindow::start);

  setValidators();
  on_actionRestore_defaults_triggered();
}

MainWindow::~MainWindow() {
  if(logFile.is_open()) {
    logFile.close();
  }

  delete ui;
}

bool MainWindow::event(QEvent *e) {

  if(e->type() == ProcessingMessageEvent::eventType) {

    handleMsgEvent(e);
    return true;
  }

  return QWidget::event(e);
}

Parameters MainWindow::getProcessParams() const {
  Parameters ret;

  ret.outputDir(ui->outputText->toPlainText().toStdString());
  ret.inputFile(ui->inputText->toPlainText().toStdString());
  ret.windowSize = ui->winSize->text().toFloat();
  ret.framesPerSecond = ui->fpsCheckbox->checkState() ? ui->fps->text().toFloat() : -1;
  ret.fracImageX = ui->fractionWidth->text().toFloat();
  ret.fracImageY = ui->fractionHeight->text().toFloat();
  ret.minTrackObjects = ui->minObjects->text().toUInt();
  ret.minObjectPix = ui->minPixels->text().toUInt();
  ret.maxFrameDiff = ui->maxFrames->text().toUInt();
  //ret.view = ui->processViewCheck->checkState();
  ret.view = false;
  ret.trackPositions = ui->trackPosCheck->checkState();
  ret.trackBlobs = ui->trackBlobsCheck->checkState();

  return ret;
}

void MainWindow::handleMsgEvent(const QEvent *e) {
  const ProcessingMessageEvent *msgEvent = static_cast<const ProcessingMessageEvent*>(e);


  QString msg = QString::fromStdString(msgEvent->msg());

  // The append puts messages on a new line
  msg.replace("\n", "");

  if(msg.length() > 0) {
    if(msgEvent->error()) {
      ui->logText->setTextColor(STDERR_TEXT_COLOR);
    } else {
      ui->logText->setTextColor(STDOUT_TEXT_COLOR);
    }

    ui->logText->append(msg);
    logFile << msg.toStdString() << std::endl;
  }
}

void MainWindow::setFormItemsEnabled(bool enabled) {
  ui->inputText->setEnabled(enabled);
  ui->inputBrowseButton->setEnabled(enabled);
  ui->outputText->setEnabled(enabled);
  ui->outputBrowseButton->setEnabled(enabled);
  ui->winSize->setEnabled(enabled);
  if (ui->fpsCheckbox->isChecked()) {
      ui->fps->setEnabled(enabled);
  }
  ui->fractionWidth->setEnabled(enabled);
  ui->fractionHeight->setEnabled(enabled);
  ui->minPixels->setEnabled(enabled);
  ui->minObjects->setEnabled(enabled);
  ui->maxFrames->setEnabled(enabled);
  //ui->processViewCheck->setEnabled(enabled);
  ui->trackBlobsCheck->setEnabled(enabled);
  ui->trackPosCheck->setEnabled(enabled);
  ui->fpsCheckbox->setEnabled(enabled);
}

void MainWindow::setValidators() {
  if(!doubleFracValidator) {
    doubleFracValidator.reset(new QDoubleValidator());
    doubleFracValidator.get()->setBottom(0.0);
  }

  if(!doubleValidator) {
    doubleValidator.reset(new QDoubleValidator());
    doubleValidator.get()->setBottom(0.0);
  }

  if(!intValidator) {
    intValidator.reset(new QIntValidator());
    intValidator.get()->setBottom(0);
  }

  ui->winSize->setValidator(doubleValidator.get());
  ui->fps->setValidator(doubleValidator.get());
  ui->fractionWidth->setValidator(doubleValidator.get());
  ui->fractionHeight->setValidator(doubleValidator.get());
  ui->minPixels->setValidator(intValidator.get());
  ui->minObjects->setValidator(intValidator.get());
  ui->maxFrames->setValidator(intValidator.get());
}

void MainWindow::browseInput() {
  QString input = ui->inputText->toPlainText();
  input = QFileDialog::getOpenFileName(this, tr("Open File"), input);

  if (input!="") {
    ui->inputText->setPlainText(input);

    //Worst case: escape all the invalid characters.
    //Assumes the video file has an extension after the final period.
    QString output = input.remove(QRegExp("\\.[a-zA-Z0-9]*$"))
                        .append("_out");

    ui->outputText->setPlainText(output+"/");
  }
}

void MainWindow::browseOutput() {
  QString output = ui->outputText->toPlainText();
  output = QFileDialog::getExistingDirectory(this, tr("Open Output Directory"), output);
  if (output.length() > 0) {
    ui->outputText->setPlainText(output+"/");
  }
}

void MainWindow::start() {
  ui->processButton->setText("CANCEL");
  ui->logText->clear();
  setFormItemsEnabled(false);

  params = getProcessParams();
  logFile.open(params.outputDir() + "/log.txt");
  
  processThread->process(params);

  disconnect(ui->processButton, &QAbstractButton::clicked, this, &MainWindow::start);
  connect(ui->processButton, &QAbstractButton::clicked, this, &MainWindow::stop);
}

void MainWindow::stop() {
  ProcessingMessageEvent *event = new ProcessingMessageEvent("User requested stop. Aborting after current step. Please wait.", true);
  QCoreApplication::postEvent(this, event);

  if (!processThread->isFinished()) {

    //We can't kill the current process, so we have to wait until the next abort check.
    //Disable the proccess button in the mean time.
    processThread->abortProcess();
    ui->processButton->setEnabled(false);
  }
}


void MainWindow::setFormValues(QString settingsFilePath) {
    //If the user clicked cancel we don't want to do anything.
    if (!settingsFilePath.isNull())
    {
      QSettings settings(settingsFilePath, QSettings::IniFormat);
      settings.beginGroup("ThermalTrackerSettings");
      ui->outputText->setPlainText(settings.value("outputDir", "").toString());
      ui->inputText->setPlainText(settings.value("inputFile","").toString());
      ui->winSize->setText(settings.value("windowSize","").toString());
        if (settings.value("framesPerSecond","").toInt() > -1) {
          ui->fpsCheckbox->setChecked(true);
          ui->fps->setText(settings.value("framesPerSecond","").toString());
        } else {
          ui->fpsCheckbox->setChecked(false);
          ui->fps->setText(QString::number(0.0));
        }

      ui->fractionWidth->setText(settings.value("fracImageX", "").toString());
      ui->fractionHeight->setText(settings.value("fracImageY", "").toString());
      ui->minObjects->setText(settings.value("minTrackObjects", "").toString());
      ui->minPixels->setText(settings.value("minObjectPix", "").toString());
      ui->maxFrames->setText(settings.value("maxFrameDiff", "").toString());
      //ui->processViewCheck->setChecked(settings.value("view", false).toBool());
      ui->trackPosCheck->setChecked(settings.value("trackPositions", false).toBool());
      ui->trackBlobsCheck->setChecked(settings.value("trackBlobs", false).toBool());

      settings.endGroup();
    }
}

void MainWindow::on_actionHelp_triggered()
{
  if(!helpWindow) {
    helpWindow.reset(new HelpWindow());
    helpWindow->show();
  } else {
    helpWindow->show();
  }
}

void MainWindow::on_actionAbout_triggered()
{
  QString msg = "ThermalTracker\nVersion 1.0\nCopyright Battelle Memorial Institute 2015";
  QMessageBox::information(this, "About", msg);
}

void MainWindow::on_actionLoad_triggered()
{
  QString settingsFile = QFileDialog::getOpenFileName(this, tr("Load Settings"), "", tr("Tracker Settings (*.ini)"));
  setFormValues(settingsFile);
}

void MainWindow::on_actionSave_triggered()
{

  //Assumes the video file has an extension after the final period.
  //QString defaultFile = ui->inputText->toPlainText().remove(QRegExp("\\.[a-zA-Z0-9]*$")).append(".ini");
  QString defaultFile = "my_tracker_settings.ini";
  QString settingsFile = QFileDialog::getSaveFileName(this,
                                                      tr("Save Settings"),
                                                      defaultFile,
                                                      tr("Tracker Settings (*.ini)")
                                                      );


  if (!settingsFile.isNull())
  {
    settingsFile = (settingsFile.endsWith(".ini") ? settingsFile : settingsFile.append(".ini"));
    QSettings settings(settingsFile, QSettings::IniFormat);
    settings.beginGroup("ThermalTrackerSettings");

    settings.setValue("outputDir", ui->outputText->toPlainText());
    settings.setValue("inputFile", ui->inputText->toPlainText());
    settings.setValue("windowSize", ui->winSize->text());
    settings.setValue("framesPerSecond", ui->fpsCheckbox->checkState() ? ui->fps->text() : "-1");
    settings.setValue("fracImageX", ui->fractionWidth->text());
    settings.setValue("fracImageY", ui->fractionHeight->text());
    settings.setValue("minTrackObjects", ui->minObjects->text());
    settings.setValue("minObjectPix", ui->minPixels->text());
    settings.setValue("maxFrameDiff", ui->maxFrames->text());
    //settings.setValue("view", ui->processViewCheck->checkState());
    settings.setValue("trackPositions", ui->trackPosCheck->checkState());
    settings.setValue("trackBlobs", ui->trackBlobsCheck->checkState());


    //settings.setValue("state1", saveState());
    settings.endGroup();
  }

}

void MainWindow::on_actionRestore_defaults_triggered()
{
    QString settingsFile = QCoreApplication::applicationDirPath()+"/defaults.ini";
    setFormValues(settingsFile);
}

void MainWindow::processing_completed()
{
    //This slot gets called when a thread process dies, completes successfully, or is interrupted
    //by the user. No matter what, we must change the button text back to start and enable it if
    //it has been disabled.
    ui->processButton->setText("START");
    ui->processButton->setEnabled(true);

    setFormItemsEnabled(true);

    disconnect(ui->processButton, &QAbstractButton::clicked, this, &MainWindow::stop);
    connect(ui->processButton, &QAbstractButton::clicked, this, &MainWindow::start);

}
