/*
 * Fifo.h
 * 
 * Creator: Yin
 * Date: 2017. 6. 21
 */  

#ifndef RING_FIFO_H
#define RING_FIFO_H

void fifo_init(int id, int size);
void fifo_deinit(int id);
void fifo_reset(int id);
int fifo_is_full(int id);
int fifo_is_empty(int id);
int fifo_space(int id);
int fifo_capacity(int id);
int fifo_write(int id, unsigned char* buffer, int size);
int fifo_read(int id, unsigned char* buffer, int size);

#endif	/* ifndef LAME_GET_AUDIO_H */

