#ifndef PROCESS_PARAMS_HPP
#define PROCESS_PARAMS_HPP

#include <cstring>
#include <string>

#define PARAMS_MAX_STR_LEN 256

// Parameters for track extraction.
struct Parameters {
 public:
  std::string inputFile() {
    return std::string(_inputFile);
  }

  void inputFile(std::string input) {
    strncpy(_inputFile, input.c_str(), PARAMS_MAX_STR_LEN);
  }

  std::string outputDir() {
    return std::string(_outputDir);
  }

  void outputDir(std::string output) {
    strncpy(_outputDir, output.c_str(), PARAMS_MAX_STR_LEN);
  }

  float windowSize;
  float framesPerSecond;
  float	     fracImageX; // fraction of image width to search for next object in track
  float	     fracImageY; // fraction of image height to search for next object in track
  unsigned int minTrackObjects; // minimum number of objects needed to form a track
  unsigned int minObjectPix;    // minimum number of pixels to form an object
  unsigned int maxFrameDiff;    // maximum number of frames between consecutive track objects
  float        thresholdFactor; // "on" pixel threshold is mean + thresholdFactor * std
  bool view;
  bool trackPositions;
  bool trackBlobs;
 private:
  // Not strings for thread safety
  char _outputDir[PARAMS_MAX_STR_LEN];
  char _inputFile[PARAMS_MAX_STR_LEN];
};

#endif
