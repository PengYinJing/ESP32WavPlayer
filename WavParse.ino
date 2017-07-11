/*
 * WavParse.ino
 * 
 * Creator: Yin
 * Date: 2017. 6. 20
 */
 
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include "Global.h"
#include "Fifo.h"

void* s_wav_file = NULL;

struct WAV_HEADER s_wav_header;
uint8_t* s_data_buffer = NULL;
int s_data_buffer_size = 0;

TimerHandle_t s_timer = NULL;
int s_timer_id = 0;

//static int s_write_size = 0;
//static int s_read_size = 0;

void read_file_handler(void* arg)
{
  int remain_size = s_wav_header.data_offset + s_wav_header.data_size - sd_file_tell(s_wav_file);
  int data_size = s_data_buffer_size;
  int fifo_size = fifo_space(0);
  int read_size = 0;

  if (fifo_size == 0)
  {
    Serial.println("=============== FIFO is full ===============");
    return;
  }
  
  if (fifo_size < data_size)
    data_size = fifo_size;
  
  if (remain_size >= data_size)
    read_size = sd_file_read(s_wav_file, s_data_buffer, data_size);
  else
  {
    if (remain_size > 0)
      read_size = sd_file_read(s_wav_file, s_data_buffer, remain_size);
    sd_file_seek(s_wav_file, s_wav_header.data_offset);
    read_size += sd_file_read(s_wav_file, &s_data_buffer[read_size], data_size - read_size);
  }

  //s_write_size += read_size;
  fifo_write(0, s_data_buffer, read_size);
}

static boolean wav_parse_header()
{
  struct WAV_HEADER header;
  unsigned char buffer4[4];
  unsigned char buffer2[2];
  int read_size = 0;
  
  memset(&header, 0, sizeof(struct WAV_HEADER));
  
  read_size = sd_file_read(s_wav_file, header.riff, sizeof(header.riff));
  if (read_size != sizeof(header.riff) || strcmp((char*)header.riff, "RIFF"))
    return false;
  Serial.printf("%s \n", header.riff);
  
  read_size = sd_file_read(s_wav_file, buffer4, sizeof(buffer4));
  if (read_size != sizeof(buffer4))
    return false;
  header.overall_size  = buffer4[0] | (buffer4[1] << 8) |  (buffer4[2] << 16) | (buffer4[3] << 24);
  Serial.printf("Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);

  read_size = sd_file_read(s_wav_file, header.wave, sizeof(header.wave));
  if (read_size != sizeof(header.wave) || strcmp((char*)header.wave, "WAVE"))
    return false;
  Serial.printf("Wave marker: %s\n", header.wave);

  read_size = sd_file_read(s_wav_file, header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker));
  if (read_size != sizeof(header.fmt_chunk_marker))
    return false;
  Serial.printf("Fmt marker: %s\n", header.fmt_chunk_marker);
  
  read_size = sd_file_read(s_wav_file, buffer4, sizeof(buffer4));
  if (read_size != sizeof(buffer4))
    return false;
  header.length_of_fmt = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
  Serial.printf("Length of Fmt header: %u \n", header.length_of_fmt);

  read_size = sd_file_read(s_wav_file, buffer2, sizeof(buffer2));
  if (read_size != sizeof(buffer2))
    return false;
  header.format_type = buffer2[0] | (buffer2[1] << 8);

  char format_name[10] = "";
  if (header.format_type == 1)
    strcpy(format_name,"PCM");
  else if (header.format_type == 6)
  {
    strcpy(format_name, "A-law");
    return false;
  }
  else if (header.format_type == 7)
  {
    strcpy(format_name, "Mu-law");
    return false;
  }
  Serial.printf("Format type: %u %s \n", header.format_type, format_name);

  read_size = sd_file_read(s_wav_file, buffer2, sizeof(buffer2));
  if (read_size != sizeof(buffer2))
    return false;
  header.channels = buffer2[0] | (buffer2[1] << 8);
  Serial.printf("Channels: %u \n", header.channels);

  read_size = sd_file_read(s_wav_file, buffer4, sizeof(buffer4));
  if (read_size != sizeof(buffer4))
    return false;
  header.sample_rate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
  Serial.printf("Sample rate: %u\n", header.sample_rate);
  
  read_size = sd_file_read(s_wav_file, buffer4, sizeof(buffer4));
  if (read_size != sizeof(buffer4))
    return false;
  header.byterate  = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);
  Serial.printf("Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate * 8);

  read_size = sd_file_read(s_wav_file, buffer2, sizeof(buffer2));
  if (read_size != sizeof(buffer2))
    return false;
  header.block_align = buffer2[0] | (buffer2[1] << 8);
  Serial.printf("Block Alignment: %u \n", header.block_align);

  read_size = sd_file_read(s_wav_file, buffer2, sizeof(buffer2));
  if (read_size != sizeof(buffer2))
    return false;
  header.bits_per_sample = buffer2[0] | (buffer2[1] << 8);
  Serial.printf("Bits per sample: %u \n", header.bits_per_sample);
  
  read_size = sd_file_read(s_wav_file, header.data_chunk_header, sizeof(header.data_chunk_header));
  if (read_size != sizeof(header.data_chunk_header))
    return false;
  Serial.printf("Data Marker: %s \n", header.data_chunk_header);

  read_size = sd_file_read(s_wav_file, buffer4, sizeof(buffer4));
  if (read_size != sizeof(buffer4))
    return false;
  header.data_size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24 );
  Serial.printf("Size of data chunk: %u \n", header.data_size);

  header.data_offset = sd_file_tell(s_wav_file);
  Serial.printf("Wav data position: %d\n", header.data_offset);
  
  // calculate no.of samples
  long num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
  Serial.printf("Number of samples:%lu \n", num_samples);
  
  long size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
  Serial.printf("Size of each sample:%ld bytes\n", size_of_each_sample);
  
  // calculate duration of file
  float duration_in_seconds = (float) header.overall_size / header.byterate;
  Serial.printf("Approx.Duration in seconds=%f\n", duration_in_seconds);

  memcpy(&s_wav_header, &header, sizeof(struct WAV_HEADER));

  return true;
}

void wav_init()
{
  Serial.println("Initializing WAV...");
  fifo_init(0, 32768);
  s_timer = xTimerCreate("ReadTimer", 5 / portTICK_PERIOD_MS, true, &s_timer_id, read_file_handler);
}

int wav_open(const char* filename, struct WAV_HEADER* header)
{
  s_wav_file = sd_file_open(filename);
  if (!s_wav_file)
  {
    Serial.println("File open error!!!");
    return -1;
  }
  if (!wav_parse_header())
    return -2;
  
  s_data_buffer_size = s_wav_header.channels * (s_wav_header.bits_per_sample/ 8) * s_wav_header.sample_rate / 50;
  s_data_buffer = (uint8_t*)malloc(s_data_buffer_size);

  if (header)
    memcpy(header, &s_wav_header, sizeof(struct WAV_HEADER));
  
  return 0;
}

void wav_play()
{
  xTimerStart(s_timer, 0);
}

void wav_close()
{
  if (s_timer)
    xTimerStop(s_timer, portMAX_DELAY);

  if (s_wav_file)
    sd_file_close(s_wav_file);
  s_wav_file = NULL;
  
  memset(&s_wav_header, 0, sizeof(struct WAV_HEADER));
  fifo_reset(0);
  
  if (s_data_buffer)
  {
    free(s_data_buffer);
    s_data_buffer = NULL;
    s_data_buffer_size = 0;
  }
}

int wav_read(uint8_t* data, int max_size)
{
  int data_size = max_size;
  if (data_size > fifo_capacity(0))
    data_size = fifo_capacity(0);
  
  //s_read_size += data_size;
  if (data_size > 0)
    fifo_read(0, data, data_size);
  else
    Serial.println("*************** FIFO is empty ***************");
  //Serial.printf("Read size: %d, Write size: %d\n", s_read_size, s_write_size);
  return data_size;
}

