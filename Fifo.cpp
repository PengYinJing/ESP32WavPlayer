/*
 * Fifo.cpp
 * 
 * Creator: Yin
 * Date: 2017. 6. 21
 */	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fifo.h"

#define MAX_FIFO_COUNT	1
static unsigned char* s_fifo[MAX_FIFO_COUNT] = { NULL };
static int s_fifo_rindex[MAX_FIFO_COUNT];
static int s_fifo_windex[MAX_FIFO_COUNT];
static int s_fifo_size[MAX_FIFO_COUNT];

void fifo_init(int id, int size)
{
	s_fifo[id] = (unsigned char*)malloc(size);
	s_fifo_size[id] = size;
	s_fifo_rindex[id] = 0;
	s_fifo_windex[id] = 0;
}

void fifo_deinit(int id)
{
	if (s_fifo[id])
		free(s_fifo[id]);
	s_fifo[id] = NULL;
}

void fifo_reset(int id)
{
  s_fifo_rindex[id] = 0;
  s_fifo_windex[id] = 0;
}

int fifo_is_full(int id)
{
	return ((s_fifo_rindex[id] - s_fifo_windex[id]) % s_fifo_size[id]) == 1;
}

int fifo_is_empty(int id)
{
	return s_fifo_rindex[id] == s_fifo_windex[id];
}

int fifo_space(int id)
{
	int count = (s_fifo_size[id] + s_fifo_windex[id] - s_fifo_rindex[id]) % s_fifo_size[id];
	return s_fifo_size[id] - count - 1;
}

int fifo_capacity(int id)
{
	return (s_fifo_size[id] + s_fifo_windex[id] - s_fifo_rindex[id]) % s_fifo_size[id];
}

int fifo_write(int id, unsigned char* buffer, int size)
{
	int space = fifo_space(id);
	if (size > space)
		size = space;
	if (s_fifo_rindex[id] > s_fifo_windex[id])
	{
		memcpy(s_fifo[id] + s_fifo_windex[id], buffer, size);
		s_fifo_windex[id] += size;
	}
	else
	{
		space = s_fifo_size[id] - s_fifo_windex[id];
		if (space >= size)
		{
			memcpy(s_fifo[id] + s_fifo_windex[id], buffer, size);
			s_fifo_windex[id] += size;
		}
		else
		{
			memcpy(s_fifo[id] + s_fifo_windex[id], buffer, space);
			memcpy(s_fifo[id], buffer + space, size - space);
			s_fifo_windex[id] = size - space;
		}
	}
	return size;
}

int fifo_read(int id, unsigned char* buffer, int size)
{
	int capacity = fifo_capacity(id);
	if (size > capacity)
		size = capacity;
	if (s_fifo_rindex[id] < s_fifo_windex[id])
	{
		memcpy(buffer, s_fifo[id] + s_fifo_rindex[id], size);
		s_fifo_rindex[id] += size;
	}
	else
	{
		capacity = s_fifo_size[id] - s_fifo_rindex[id];
		if (capacity >= size)
		{
			memcpy(buffer, s_fifo[id] + s_fifo_rindex[id], size);
			s_fifo_rindex[id] += size;
		}
		else
		{
			memcpy(buffer, s_fifo[id] + s_fifo_rindex[id], capacity);
			memcpy(buffer + capacity, s_fifo[id], size - capacity);
			s_fifo_rindex[id] = size - capacity;
		}
	}
	return size;
}


