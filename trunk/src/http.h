#ifndef __HTTP_H__
#define __HTTP_H__


#include "def.h"

struct http_session
{
    char* user_agent;
	int   socket;

    /* size of http file */
    rt_size_t size;
    rt_off_t  position;
};


struct shoutcast_session
{
	int   socket;

	/* shoutcast name and bitrate */
	char* station_name;
	int   bitrate;

	/* size of meta data */
	rt_size_t metaint;
	rt_size_t current_meta_chunk;
};




//bool http_session_init();
struct http_session* http_session_open(const char* url);
rt_size_t http_session_read(struct http_session* session,char  *buffer, rt_size_t length);
rt_off_t http_session_seek(struct http_session* session, rt_off_t offset, int mode);
int http_session_close(struct http_session* session);
void test();


struct shoutcast_session* shoutcast_session_open(const char* url);
rt_size_t shoutcast_session_read(struct shoutcast_session* session, rt_uint8_t *buffer, rt_size_t length);



void getjpg(char * url);



#endif
