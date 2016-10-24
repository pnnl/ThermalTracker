#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <memory>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream_buffer.hpp>

namespace bio = boost::iostreams;

template <class TOutSink, class TErrSink>
class Logger {
 public:
  Logger() {
    oldOut = std::cout.rdbuf();
    oldErr = std::cerr.rdbuf();
  }

  ~Logger() {
    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);
  }

  void setOutSink(TOutSink sink) {
    outStream.reset(new bio::stream_buffer<TOutSink>);
    outStream->open(sink);
    oldOut = std::cout.rdbuf(outStream.get());
  }

  void setErrSink(TErrSink sink) {
    errStream.reset(new bio::stream_buffer<TErrSink>);
    errStream->open(sink);
    oldErr = std::cerr.rdbuf(errStream.get());
  }

 private:
  std::streambuf *oldOut;
  std::streambuf *oldErr;
  std::unique_ptr<bio::stream_buffer<TOutSink> > outStream;
  std::unique_ptr<bio::stream_buffer<TErrSink> > errStream;
};

#endif
