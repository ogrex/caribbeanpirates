#ifndef __HTTP_H__
#define __HTTP_H__


#include "rtdef.h"

struct http_session
{
    char* user_agent;
	int   socket;

    /* size of http file */
    rt_size_t size;
    rt_off_t  position;
};

//bool http_session_init();
struct http_session* http_session_open(const char* url);
rt_size_t http_session_read(struct http_session* session,char  *buffer, rt_size_t length);
rt_off_t http_session_seek(struct http_session* session, rt_off_t offset, int mode);
int http_session_close(struct http_session* session);
void test();

#endif
