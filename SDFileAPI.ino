/*
 * SDFile.ino
 * 
 * Creator: Yin
 * Date: 2017. 7.4
 */

#include <SD.h>
#include "LinkedList.h"

static LinkedList<String> s_file_list = LinkedList<String>();
static File s_opened_file;

boolean sd_file_init()
{
  Serial.println("Initializing SD card...");
  if (!SD.begin())
  {
    Serial.println("SD card mount failed!!!");
    return false;
  }

  uint8_t card_type = SD.type();
  Serial.print("SD card type: ");
  
  if (card_type == SD_CARD_TYPE_SD1)
    Serial.println("SD1");
  else if (card_type == SD_CARD_TYPE_SD2)
    Serial.println("SD2");
  else if ( card_type == SD_CARD_TYPE_SDHC)
    Serial.println("SDHC");
  else
    Serial.println("UNKNOWN");
  
  return true;
}

void* sd_file_open(const char* path)
{
  s_opened_file = SD.open(path);
  return &s_opened_file;
}

int sd_file_read(void* handle, void* buffer, int size)
{
  File* file = (File*)handle;
  if (file)
    return file->read(buffer, size);
  return 0;
}

int sd_file_write(void* handle, void* buffer, int size)
{
  File* file = (File*)handle;
  if (file)
    return file->write((const char*)buffer, size);
  return 0;
}

boolean sd_file_seek(void* handle, int position)
{
  File* file = (File*)handle;
  if (file)
    return file->seek(position);
  return false;
}

int sd_file_tell(void* handle)
{
  File* file = (File*)handle;
  if (file)
    return file->position();
  return -1;
}

void sd_file_close(void* handle)
{
  File* file = (File*)handle;
  if (file)
    file->close();
}

static void get_wav_files(LinkedList<String>* file_list, File dir, String path)
{
   while(true)
   {
     File entry =  dir.openNextFile();
     if (!entry)
       break;
     
     //Serial.print(entry.name());
     if (entry.isDirectory())
       get_wav_files(file_list, entry, path + entry.name() + "/");
     else
     {
      String filename = path + entry.name();
      if (filename.endsWith(".wav") || filename.endsWith(".WAV"))
        file_list->add(filename);
     }
     entry.close();
   }
}

void* sd_file_lists(const char* path)
{
  File root = SD.open(path);
  get_wav_files(&s_file_list, root, path);
  root.close();
  return &s_file_list;
}

