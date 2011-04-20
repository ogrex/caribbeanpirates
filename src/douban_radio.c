#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "http.h"
#include "douban_radio.h"
#include "JSON_parser.h"


#define DOUBAN_RADIO_URL 			"http://douban.fm/j/mine/playlist"
#define DOUBAN_RADIO_URL_CHANNEL	"http://douban.fm/j/mine/playlist?type=n&channel=%d"


#define URL_SIZE	128


#define PARSE_TYPE_UNKNOW	0x00
#define PARSE_TYPE_PICTURE	0x01
#define PARSE_TYPE_ARTIST	0x02
#define PARSE_TYPE_TITLE	0x03
#define PARSE_TYPE_URL		0x04

static int _parse_callback(void* ctx, int type, const JSON_value* value)
{
	struct douban_radio* douban;
	struct douban_song_item* item;
	static rt_uint32_t last_parse_type = PARSE_TYPE_UNKNOW;

	douban = (struct douban_radio*) ctx;
	item = &douban->items[douban->size];

	switch (type)
	{
    case JSON_T_KEY:
        // rt_kprintf("key = '%s', value = ", value->vu.str.value);
		if (strcmp(value->vu.str.value, "picture") == 0)
		{
			last_parse_type = PARSE_TYPE_PICTURE;
		}
		else if (strcmp(value->vu.str.value, "artist") == 0)
		{
			last_parse_type = PARSE_TYPE_ARTIST;
		}
		else if (strcmp(value->vu.str.value, "title") == 0)
		{
			last_parse_type = PARSE_TYPE_TITLE;
		}
		else if (strcmp(value->vu.str.value, "url") == 0)
		{
			last_parse_type = PARSE_TYPE_URL;
		}
		else if (strcmp(value->vu.str.value, "aid") == 0)
		{
			last_parse_type = PARSE_TYPE_UNKNOW;
			/* move to next item */
			douban->size += 1;
			// rt_kprintf("move to next item: %d\n", douban->size);
			if (douban->size >= DOUBAN_SONG_MAX)
				/* terminate parse */
				return 0;
		}
        break;

    case JSON_T_STRING:
		switch (last_parse_type)
		{
		case PARSE_TYPE_PICTURE:
			item->picture = strdup(value->vu.str.value);  //in linux using strdup 
			break;
		case PARSE_TYPE_ARTIST:
			item->artist = strdup(value->vu.str.value);
			break;
		case PARSE_TYPE_TITLE:
			item->title = strdup(value->vu.str.value);
			break;
		case PARSE_TYPE_URL:
			item->url = strdup(value->vu.str.value);
			break;
		default:
			break;
		}
        // rt_kprintf("string: '%s'\n", value->vu.str.value);
        break;
	}

	if (type != JSON_T_KEY)
		last_parse_type = PARSE_TYPE_UNKNOW;

	return 1;
}

void douban_radio_parse(struct douban_radio* douban, const char* buffer, rt_size_t length)
{
	JSON_config config;
	struct JSON_parser_struct* jc = NULL;
	const char* ptr;

	init_JSON_config(&config);
    config.depth                  = 19;
    config.callback               = &_parse_callback;
	config.callback_ctx           = douban;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;

	jc = new_JSON_parser(&config);
	ptr = buffer;
	while (ptr < buffer + length)
	{
		if (!JSON_parser_char(jc, *ptr++))
		{
			// rt_kprintf("JSON_parser_error: parse failed\n");
			break;
		}
	}

	if (!JSON_parser_done(jc))
	{
		// rt_kprintf("JSON_parser_end: syntax error\n");
	}

	delete_JSON_parser(jc);
}





int douban_radio_playlist_load(struct douban_radio* douban)
{
	rt_uint32_t index;	
	rt_size_t length;
	char *buffer, *url;
	char *ptr;
	struct http_session* session;


	for (index = 0; index < douban->size; index ++)
	{
		free(douban->items[index].artist);
		free(douban->items[index].title);
		free(douban->items[index].url);
		free(douban->items[index].picture);
	}

	url = (char*)malloc(URL_SIZE);
	snprintf(url, URL_SIZE,DOUBAN_RADIO_URL_CHANNEL, douban->channel);
	//在linux下面没有sprintf_s这个，但是可以用snprintf替代，参数都不用改
	session = http_session_open(url);
	if (session == 0) goto __exit;


	buffer =(char*)malloc(session->size);
	ptr = buffer;

	do
	{
		length = http_session_read(session, ptr, buffer + session->size - ptr);
		if (length <= 0) break;
		ptr += length;
	} while (ptr < buffer + session->size);
	length = ptr - buffer;
	printf("total %d bytes\n", (int)length);
	http_session_close(session); session = 0;

	/* parse douban song list */
	douban->current = douban->size = 0;
	douban_radio_parse(douban, buffer, length);


	return 0;


	__exit:
	//if (buffer != 0) free(buffer);
	if (url != 0) free(url);
	//if (session != 0) http_session_close(session);

	return -1;
}




rt_size_t douban_radio_read(struct douban_radio* douban, char *buffer, rt_size_t length)
{
	rt_size_t rx_length;
	char* ptr;

	if (douban->current >= douban->size) 
	{
		douban_radio_playlist_load(douban);
		printf("wrong\n");
		return 0;
	}

	ptr = buffer;
	while (length > 32)                  //why > 32?
	{
		if (douban->session == 0)
		{
			/* create a http session */
			// rt_kprintf("open session url[%d]: %s\n", douban->current,
			//	douban->items[douban->current].url);
			douban->session = http_session_open(douban->items[douban->current].url);
			printf("I am here\n");
			if (douban->session == 0)
			{
				/* can't open this link */
				douban->current ++;
				// rt_kprintf("open session failed, move to %d\n", douban->current);
				if (douban->current >= douban->size)
				{
					douban_radio_playlist_load(douban);
					break;
				}
			}
		}

		/* read http client data */
		rx_length = http_session_read(douban->session, ptr, length);
		printf("read data from the mp3 http_session\n");
		if (rx_length <= 0)
		{
			/* close this session */
			http_session_close(douban->session);
			douban->session = 0;
			douban->current ++;
			// rt_kprintf("close session, move to %d\n", douban->current);
			if (douban->current >= douban->size)
			{
				douban_radio_playlist_load(douban);
				break;
			}
		}
		else
		{
			ptr += rx_length;
			length -= rx_length;
		}
	}

	return ptr - buffer;
}
