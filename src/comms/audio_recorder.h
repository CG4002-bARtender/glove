#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "../config.h"

class AudioRecorder
{
public:
  AudioRecorder();
  ~AudioRecorder();

  bool begin();
  bool recordChunk();
  void reset();
  bool sendWav(const char* host, int port);

  size_t samplesRecorded() const;
  float secondsRecorded() const;
  bool isFull() const;

private:
  void writeWavHeader(uint8_t* header, size_t dataSize);

  int16_t* m_buffer;
  size_t m_bufferSize;
  size_t m_writePos;
  bool m_allocated;
};
