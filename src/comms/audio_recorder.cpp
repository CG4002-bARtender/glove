#include "audio_recorder.h"
#include <driver/i2s.h>

AudioRecorder::AudioRecorder()
    : m_buffer(nullptr),
      m_bufferSize(config::kRecordBufferSize),
      m_writePos(0),
      m_allocated(false)
{
}

AudioRecorder::~AudioRecorder()
{
  if (m_buffer)
  {
    free(m_buffer);
    m_buffer = nullptr;
  }
}

bool AudioRecorder::begin()
{
  if (m_allocated) return true;

  m_buffer = (int16_t*)malloc(m_bufferSize);
  if (!m_buffer)
  {
    Serial.printf("ERROR: Failed to allocate %d bytes for recording buffer\n", m_bufferSize);
    Serial.printf("Free heap: %d bytes, max contiguous: %d bytes\n",
                  ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    return false;
  }

  Serial.printf("Recording buffer allocated: %d bytes (%.1f sec max)\n",
                m_bufferSize, config::kMaxRecordSeconds);
  Serial.printf("Free heap after alloc: %d bytes\n", ESP.getFreeHeap());
  m_allocated = true;
  return true;
}

bool AudioRecorder::recordChunk()
{
  if (!m_allocated || isFull()) return false;

  static const size_t kI2sReadSamples = 128;
  int32_t i2sBuf[kI2sReadSamples];
  size_t bytesRead = 0;

  esp_err_t err = i2s_read(I2S_NUM_0, i2sBuf, sizeof(i2sBuf), &bytesRead, 100);
  if (err != ESP_OK || bytesRead == 0) return true;

  size_t samplesRead = bytesRead / sizeof(int32_t);
  size_t spaceLeft = (m_bufferSize - m_writePos) / sizeof(int16_t);
  size_t toWrite = (samplesRead < spaceLeft) ? samplesRead : spaceLeft;

  int16_t* dst = (int16_t*)((uint8_t*)m_buffer + m_writePos);

  // INMP441 outputs 24-bit data left-justified in 32-bit word.
  // Shift right by 16 to get signed 16-bit range.
  for (size_t i = 0; i < toWrite; i++)
  {
    dst[i] = (int16_t)(i2sBuf[i] >> 16);
  }

  m_writePos += toWrite * sizeof(int16_t);
  return !isFull();
}

void AudioRecorder::reset()
{
  m_writePos = 0;
}

size_t AudioRecorder::samplesRecorded() const
{
  return m_writePos / sizeof(int16_t);
}

float AudioRecorder::secondsRecorded() const
{
  return (float)samplesRecorded() / (float)config::kSampleRate;
}

bool AudioRecorder::isFull() const
{
  return m_writePos >= m_bufferSize;
}

bool AudioRecorder::sendWav(const char* host, int port)
{
  if (m_writePos == 0)
  {
    Serial.println("Nothing to send (buffer empty).");
    return false;
  }

  Serial.printf("Connecting to TCP %s:%d ...\n", host, port);

  WiFiClient client;
  if (!client.connect(host, port, config::kTcpConnectTimeoutMs))
  {
    Serial.println("TCP connection failed!");
    return false;
  }

  Serial.println("TCP connected. Sending WAV...");

  uint8_t header[config::kWavHeaderSize];
  writeWavHeader(header, m_writePos);
  client.write(header, config::kWavHeaderSize);

  size_t sent = 0;
  uint8_t* data = (uint8_t*)m_buffer;
  while (sent < m_writePos)
  {
    size_t chunk = m_writePos - sent;
    if (chunk > config::kTcpSendChunkSize)
      chunk = config::kTcpSendChunkSize;

    size_t written = client.write(data + sent, chunk);
    if (written == 0)
    {
      Serial.println("TCP send error!");
      client.stop();
      return false;
    }
    sent += written;
  }

  client.flush();
  client.stop();

  Serial.printf("WAV sent: %d bytes (header) + %d bytes (PCM) = %d total\n",
                config::kWavHeaderSize, m_writePos,
                config::kWavHeaderSize + m_writePos);
  return true;
}

void AudioRecorder::writeWavHeader(uint8_t* h, size_t dataSize)
{
  uint32_t fileSize = config::kWavHeaderSize + dataSize - 8;
  uint32_t byteRate = config::kSampleRate * config::kRecordChannels * (config::kRecordBitsPerSample / 8);
  uint16_t blockAlign = config::kRecordChannels * (config::kRecordBitsPerSample / 8);

  // RIFF header
  h[0]='R'; h[1]='I'; h[2]='F'; h[3]='F';
  h[4] = fileSize & 0xFF;
  h[5] = (fileSize >> 8) & 0xFF;
  h[6] = (fileSize >> 16) & 0xFF;
  h[7] = (fileSize >> 24) & 0xFF;
  h[8]='W'; h[9]='A'; h[10]='V'; h[11]='E';

  // fmt subchunk
  h[12]='f'; h[13]='m'; h[14]='t'; h[15]=' ';
  h[16]=16; h[17]=0; h[18]=0; h[19]=0;
  h[20]=1;  h[21]=0;
  h[22]=config::kRecordChannels; h[23]=0;

  uint32_t sr = config::kSampleRate;
  h[24]=sr&0xFF; h[25]=(sr>>8)&0xFF; h[26]=(sr>>16)&0xFF; h[27]=(sr>>24)&0xFF;

  h[28]=byteRate&0xFF; h[29]=(byteRate>>8)&0xFF; h[30]=(byteRate>>16)&0xFF; h[31]=(byteRate>>24)&0xFF;
  h[32]=blockAlign&0xFF; h[33]=(blockAlign>>8)&0xFF;
  h[34]=config::kRecordBitsPerSample; h[35]=0;

  // data subchunk
  h[36]='d'; h[37]='a'; h[38]='t'; h[39]='a';
  h[40]=dataSize&0xFF; h[41]=(dataSize>>8)&0xFF; h[42]=(dataSize>>16)&0xFF; h[43]=(dataSize>>24)&0xFF;
}
