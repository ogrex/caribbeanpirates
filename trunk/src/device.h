#ifndef _CODER_H
#define _CODER_H
#include "def.h"



struct file_item
{
	rt_uint16_t size;
	char items[10][50];
};
void snd_write(rt_uint16_t*buffer, size_t size);
int init_dev(int freq);
void sd_item(struct file_item* file);

#endif
