/*
 * ESP32WavPlayer.ino
 * 
 * Creator: Yin
 * Date: 2017. 6. 20
 */

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <driver/i2s.h>
#include <SD.h>
#include "Global.h"
#include "LinkedList.h"

static uint8_t* s_pcm_buffer = NULL;
static int s_pcm_buffer_size = 0;
static boolean s_i2s_configured = false;

static LinkedList<String>* s_wav_file_list = NULL;
static int s_wav_file_index = -1;

static TimerHandle_t s_button_timer = NULL;
static int s_button_timer_id = 1;
static boolean s_button_level = false;
static int s_button_pin = 4;

void i2s_init()
{
  Serial.println("Initializing I2S...");
  i2s_config_t i2s_config;
  i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  i2s_config.sample_rate = 44100;
  i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  i2s_config.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
  i2s_config.dma_buf_count = 32;
  i2s_config.dma_buf_len = 64;
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;

  i2s_pin_config_t pin_config = {
    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = 22,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_stop(I2S_NUM_0);
}

void i2s_configure(struct WAV_HEADER* wav_header)
{
  Serial.println("Configuring I2S...");
  i2s_channel_t ch = I2S_CHANNEL_STEREO;
  i2s_bits_per_sample_t bits = I2S_BITS_PER_SAMPLE_16BIT;
  if (wav_header->channels == 1)
    ch = I2S_CHANNEL_MONO;

  if (wav_header->bits_per_sample == 8)
    bits = I2S_BITS_PER_SAMPLE_8BIT;
  else if (wav_header->bits_per_sample == 24)
    bits = I2S_BITS_PER_SAMPLE_24BIT;
  else if (wav_header->bits_per_sample == 32)
    bits = I2S_BITS_PER_SAMPLE_32BIT;
  
  i2s_set_clk(I2S_NUM_0, wav_header->sample_rate, bits, ch);
}

void button_init()
{
  Serial.println("Initializing Button...");
  pinMode(s_button_pin, INPUT);
  attachInterrupt(s_button_pin, button_intr_cb, FALLING);
  s_button_level = true;
  s_button_timer = xTimerCreate("ButtonTimer", 50 / portTICK_PERIOD_MS, false, &s_button_timer_id, button_timer_cb);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Started wav player");

  int err = sd_file_init();
  while (err == -1)
  {
    delay(1000);
    err = sd_file_init();
  }
  s_wav_file_list = (LinkedList<String>*)sd_file_lists("/");

  button_init();
  i2s_init();
  wav_init();
  if (s_wav_file_list->size() > 0)
  {
    s_wav_file_index = 0;
    play_wav(s_wav_file_list->get(s_wav_file_index));
  }
  else
    Serial.println("There is no WAV file");
}

void loop()
{
  if (!s_i2s_configured)
    return;
  //static int last_time = -1;
  //int cur_time = millis();
  //if (last_time == -1 || cur_time - last_time >= 5)
  {
    int byte_left = wav_read(s_pcm_buffer, s_pcm_buffer_size);
    int byte_written = 0;
    uint8_t* buf = s_pcm_buffer;
    while (byte_left > 0)
    {
      byte_written = i2s_write_bytes(I2S_NUM_0, (const char*)buf, byte_left, 0);
      byte_left -= byte_written;
      buf += byte_written;
    }
    //last_time = cur_time;
  }
}

void play_wav(String file)
{
  Serial.printf("Start playing wav (%s)...\n", file.c_str());
  struct WAV_HEADER header;
  int err = wav_open(file.c_str(), &header);
  if (err == 0)
  {
    int pcm_buffer_size = header.channels * (header.bits_per_sample / 8) * header.sample_rate / 100;
    if (s_pcm_buffer_size != pcm_buffer_size)
    {
      if (s_pcm_buffer)
        free(s_pcm_buffer);
      s_pcm_buffer_size = pcm_buffer_size;
      s_pcm_buffer = (uint8_t*)malloc(s_pcm_buffer_size);
    }
    i2s_configure(&header);
    s_i2s_configured = true;
    i2s_start(I2S_NUM_0);
    
    wav_play();
  }
}

void stop_wav()
{
  wav_close();
  i2s_stop(I2S_NUM_0);
  s_i2s_configured = false;
}

void button_timer_cb(void* arg)
{
  if (digitalRead(s_button_pin))
  {
    //xTimerStop(s_button_timer, 0);
    s_button_level = true;
    attachInterrupt(s_button_pin, button_intr_cb, FALLING);

    // Button pressed
    Serial.println("Stopping WAV...");
    stop_wav();
    
    if (s_wav_file_list->size() > 0)
    {
      Serial.println("Playing next WAV file...");
      s_wav_file_index = (s_wav_file_index + 1) % s_wav_file_list->size();
      play_wav(s_wav_file_list->get(s_wav_file_index));
    }
  }
  else
    attachInterrupt(s_button_pin, button_intr_cb, RISING);
}

void button_intr_cb()
{
  if (s_button_level)
  {
    s_button_level = false;
    attachInterrupt(s_button_pin, button_intr_cb, RISING);
  }
  else
    xTimerStart(s_button_timer, 0);
}

