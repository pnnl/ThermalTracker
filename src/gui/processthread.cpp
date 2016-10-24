#include "processthread.hpp"
#include "extracttracks.hpp"
#include "processvideo.hpp"
#include "trackfeatures.hpp"
#include "trackblobs.hpp"
#include "trackpositions.hpp"

ProcessThread::ProcessThread(QObject *parent) : QThread(parent), stdoutSink(parent, false),
      stderrSink(parent, true), abort(false) {
}

ProcessThread::~ProcessThread(){
  abort = true;
  wait();
}

void ProcessThread::process(Parameters params){
  this->params = params;
  start();
}

void ProcessThread::abortProcess() {
  abort = true;
}

void ProcessThread::run() {
      Logger<QtOutputSink, QtOutputSink> logger;
      logger.setOutSink(stdoutSink);
      logger.setErrSink(stderrSink);

      int result = 0;

      try {
        result = processVideo(params);
      } catch(...) {
        result = -1;
      }

      if (result != 0) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Something went wrong during the processing step. Aborting.", true);
          QCoreApplication::postEvent(parent(), event);
          emit processComplete();
          return;
      }

      if (abort) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Aborted at user request.", true);
          QCoreApplication::postEvent(parent(), event);
          abort = false;
          emit processComplete();
          return;
      }

      if (result == 0) {
        try {
          result = extractTracks(params);
        } catch(...) {
          result = -1;
        }

        if (result != 0) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Something went wrong during the extract tracks step. Aborting.", true);
          QCoreApplication::postEvent(parent(), event);
          emit processComplete();
          return;
        }
      }

      if (abort) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Aborted at user request.", true);
          QCoreApplication::postEvent(parent(), event);
          abort = false;
          emit processComplete();
          return;
      }


      if (result == 0) {
        try {
          result = trackFeatures(params);
        } catch(...) {
          result = -1;
        }

        if (result != 0) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Something went wrong during the track features step. Aborting.", true);
          QCoreApplication::postEvent(parent(), event);
          emit processComplete();
          return;
        }
      }

      if (abort) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Aborted at user request.", true);
          QCoreApplication::postEvent(parent(), event);
          abort = false;
          emit processComplete();
          return;
      }

      if (result == 0 && params.trackPositions) {
        try {
          result = trackPositions(params);
        } catch(...) {
          result = -1;
        }
        if (result != 0) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Something went wrong during the track positions step. Aborting.", true);
          QCoreApplication::postEvent(parent(), event);
          emit processComplete();
          return;
        }

      }

      if (abort) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Aborted at user request.", true);
          QCoreApplication::postEvent(parent(), event);
          abort = false;
          emit processComplete();
          return;
      }

      if (result == 0 && params.trackBlobs) {
        try {
          result = trackBlobs(params);
        } catch(...) {
          result = -1;
        }

        if (result != 0) {
          ProcessingMessageEvent *event = new ProcessingMessageEvent("Something went wrong during the track features step. Aborting.", true);
          QCoreApplication::postEvent(parent(), event);
          emit processComplete();
          return;
        }
      }

      ProcessingMessageEvent *event = new ProcessingMessageEvent("Processing finished.", false);

      QCoreApplication::postEvent(parent(), event);

      emit processComplete();
      return;


}
