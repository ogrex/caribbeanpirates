//#ifndef __DOUBAN_RADIO_H__
//#define __DOUBAN_RADIO_H__
#include "rtdef.h"


struct douban_song_item
{
	char* artist;
	char* title;
	char* url;
	char* picture;
};

#define DOUBAN_SONG_MAX		10


struct douban_radio
{
	rt_uint16_t size;
	rt_uint16_t current;
	int channel;

	struct http_session* session;
	struct douban_song_item items[DOUBAN_SONG_MAX];
};


int douban_radio_playlist_load(struct douban_radio* douban);
rt_size_t douban_radio_read(struct douban_radio* douban, char *buffer, rt_size_t length);





//#endif
