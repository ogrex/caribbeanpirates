#include "mp3.h"
#include <stdlib.h>
#include <string.h> 
#include "uart_def.h"


#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <sys/wait.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include "http.h"
#include "malloc.h"

#include "uart.h"
#include "douban_radio.h"

#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/

/************
发送图片
*************/
int send_jpg(int uart_fd, struct douban_radio *douban)
{
		rt_size_t length;
		struct http_session* session;
		char *buffer,*ptr;

		int index = 0;
		char index_buff[1] ={0};

		char protocol_buff[5]={0};
		char length_buff[10]={0};
		
		tcflush(uart_fd, TCIOFLUSH);

		char command_buff[1] = {JPG_INDEX};
		write(uart_fd, command_buff, 1);
		printf("JPG_INDEX is send \n");
		
	if(uart_read(uart_fd, index_buff, 1, 1) > 0)
	{
		index = atoi(index_buff);
		printf("jpg index is read %d!\n", index);

		session=http_session_open(douban->items[index].picture);
		buffer =(char*)malloc(session->size);
		ptr=buffer;

	do
	{
		length = http_session_read(session, ptr, buffer + session->size - ptr);
		if (length <= 0) break;
		ptr += length;
		
	} while (ptr < buffer + session->size);

			tcflush(uart_fd, TCIOFLUSH);
			printf("begin to send jpg's length!\n");
			sprintf(length_buff ,"%d",session->size);
			printf("length_buff is %s\n", length_buff);
			write(uart_fd, length_buff, 10);
			printf("length is send\n\n");
			printf("wait for it's ready!\n");
	
			if(uart_read(uart_fd, protocol_buff, 1 ,2) > 0)
			{
				if(protocol_buff[0] == JPG_LENGTH_END)
				{	
					printf("ready to send jpg data!\n");
					write(uart_fd, buffer, (int)session->size);
					printf("jpg data is send!\n");
					printf("wait for it's ready!\n");
					if( uart_read(uart_fd, protocol_buff, 1, 8) > 0)
					{
						if(protocol_buff[0] == JPG_END)
						{
							printf("jpg_end\n");
							return 1;
						}
						else
						{
						 	printf("error 4!\n");
						}
					}
					else 
					{
						printf("error 3!\n");
					}		
				}
				else 
				{
					printf("error 2!\n");
				}
			}
			else 
			{
				printf("error 1!\n");
			}

		free(buffer);
	}
}

/***********
发送列表标题
***********/
int send_title(int uart_fd, struct douban_radio *douban)
{
	 	int index = 0;
		char index_buff[1] ={0};

		char protocol_buff[5]={0};
		char length_buff[10]={0};

		char command_buff[1] = {TITLE_INDEX};
		write(uart_fd, command_buff, 1);
	
	if(uart_read(uart_fd, index_buff, 5, 1) == 5)
	{
		index = atoi(index_buff);
		printf("title index is read %d!\n", index);
		printf("title is %s\n", douban->items[index].title);

			tcflush(uart_fd, TCIOFLUSH);
			printf("begin to send title's length!\n");
			sprintf(length_buff ,"%d",strlen(douban->items[index].title));
			printf("length_buff is %s\n", length_buff);
			write(uart_fd, length_buff, 10);
			printf("length is send\n\n");
			printf("wait for it's ready!\n");

		if(uart_read(uart_fd, protocol_buff, 1 ,1) > 0)
			{
				if(protocol_buff[0] == TITLE_LENGTH_END)
				{	
					printf("ready to send title data!\n");
					write(uart_fd, douban->items[index].title, strlen(douban->items[index].title));
					printf("title data is send!\n");
				//	printf("wait for it's ready!\n");
				/*
					if( uart_read(uart_fd, protocol_buff, 1, 3) > 0)
					{
						if(protocol_buff[0] == TITLE_END)
						{
							printf("title_end\n");
							return 1;
						}
						else
						{
						 	printf("title error 4!\n");
						}
					
					}
					else 
					{
						printf("title error 3!\n");
					}	*/	
				}
				else 
				{
					printf("title error 2!\n");
				}
			}
			else 
			{
				printf("title error 1!\n");
			}
	}

}


/***********
发送列表作者
***********/
int send_artist(int uart_fd, struct douban_radio *douban)
{
		int index = 0;
		char index_buff[1] ={0};

		char protocol_buff[5]={0};
		char length_buff[10]={0};

		char command_buff[1] = {ARTIST_INDEX};
		write(uart_fd, command_buff, 1);
	
	if(uart_read(uart_fd, index_buff, 5, 1) == 5)
	{
		index = atoi(index_buff);
		printf("artist index is read %d!\n", index);
		printf("artist is %s\n", douban->items[index].title);

			tcflush(uart_fd, TCIOFLUSH);
			printf("begin to send artist's length!\n");
			sprintf(length_buff ,"%d",strlen(douban->items[index].artist));
			printf("length_buff is %s\n", length_buff);
			write(uart_fd, length_buff, 10);
			printf("length is send\n\n");
			printf("wait for it's ready!\n");

		if(uart_read(uart_fd, protocol_buff, 1 ,1) > 0)
			{
				if(protocol_buff[0] == ARTIST_LENGTH_END)
				{	
					printf("ready to send artist data!\n");
					write(uart_fd, douban->items[index].artist, strlen(douban->items[index].artist));
					printf("artist data is send!\n");
					//printf("wait for it's ready!\n");
				/*
					if( uart_read(uart_fd, protocol_buff, 1, 1) > 0)
					{
						if(protocol_buff[0] == TITLE_END)
						{
							printf("artist_end\n");
							return 1;
						}
						else
						{
						 	printf("error 4!\n");
						}
					}
					else 
					{
						printf("error 3!\n");
					}	
				*/	
				}
				else 
				{
					printf("error 2!\n");
				}
			}
			else 
			{
				printf("error 1!\n");
			}
	}

}

/**********
播放音乐
播放音乐时，若有按键消息则返回
***********/
void play_song(int uart_fd, struct douban_radio *douban)
{
	int index = 0;
	char index_buff[5] = {0};
	char command_buff[1] = {PLAY_INDEX};
	write(uart_fd, command_buff, 1);
	
	if(uart_read(uart_fd, index_buff, 5, 1) == 5)
	{
		index = atoi(index_buff);
		printf("play index is read %d!\n", index);
		printf("%s\n", douban->items[index].url);
		douban_radio(douban->items[index].url);
	}
}


/**************
当index超出当前列表时，重新载入列表
***************/ 
void load_list(int uart_fd, struct douban_radio *douban)
{
	char command_buff[1] = {0};
	command_buff[0] = LOADLIST_BEGIN;
	write(uart_fd, command_buff, 1);

	free(douban);
	printf("\n\n\n\n\n\nget new list\n\n\n\n\n\n");
	douban=(struct douban_radio*) malloc (sizeof(struct douban_radio));
	memset(douban, 0, sizeof(struct douban_radio));
	douban->channel=2;	

	douban_radio_playlist_load(douban);
	
	command_buff[0] = LOADLIST_END;
	write(uart_fd, command_buff, 1);
	
}

/**************
当index超出当前列表时，重新载入列表
***************/ 
void stop(int uart_fd)
{
	char command_buff[1] = {STOP};
	write(uart_fd, command_buff, 1);
}

/*************************************************************************
******************main***********************
************************************************************************/
int uart_fd;
char protocol_buff[1];

int main()
{

struct douban_radio* douban;
douban=(struct douban_radio*) malloc (sizeof(struct douban_radio));
memset(douban, 0, sizeof(struct douban_radio));

douban->channel=2;

douban_radio_playlist_load(douban);

int mp3_out = 0;
char com_temp[1];

init_dev(22050);
uart_fd = Uart_Init(uart_dev);

printf("douban channel %d \n", douban->channel);

while(1)													//主消息循环
{
	if(uart_read(uart_fd, protocol_buff, 1, 0) > 0)
	//if((mp3_out == 1) || (uart_read(uart_fd, protocol_buff, 1, 0) > 0))
	{
		//mp3_out = 0;
		//printf("get data\n");
		switch(protocol_buff[0])
		{	
			case COM_START:
				com_temp[0] = COM_END;
				write(uart_fd, com_temp, 1);
				break;
			case COM_TITLE:
				printf("send title!\n");
				send_title(uart_fd, douban);
				break;
			case COM_ARTIST:
				printf("send artist!\n");
				send_artist(uart_fd, douban);	
				break;
			case COM_JPG:
				printf("send jpg!\n");
				send_jpg(uart_fd, douban);
				break;
			case COM_PLAY:
				printf("douban channel %d \n", douban->channel);
				play_song(uart_fd, douban);
				//printf("play song1\n");
				//ice_mp3("http://190.220.157.52:8000");
				//mp3("/song1.mp3");
				//mp3_out = 1;								
				break;
			case NEW_LIST:
				load_list(uart_fd, douban);
			case COM_STOP:
				stop(uart_fd);
				printf("stop!\n");
				break;			
			default:
				break;
		}
	}
}

/*

int index;
struct douban_radio* douban;
douban=(struct douban_radio*) malloc (sizeof(struct douban_radio));
memset(douban, 0, sizeof(struct douban_radio));
douban->channel=2;

douban_radio_playlist_load(douban);
printf("items:\t%d\n",douban->size );
	for (index = 0; index < douban->size; index ++)
	{
		printf("picture: \t%s\n", douban->items[index].picture);
		printf("title  : \t%s\n", douban->items[index].title);
		printf("artist : \t%s\n", douban->items[index].artist);
		printf("url    : \t%s\n\n", douban->items[index].url);
		printf("index %d \n", index);
	}

free(douban);
printf("\n\n\n\n\n");


douban=(struct douban_radio*) malloc (sizeof(struct douban_radio));
memset(douban, 0, sizeof(struct douban_radio));
douban->channel=2;
douban_radio_playlist_load(douban);
//load_list(douban);


printf("items:\t%d\n",douban->size );
	for (index = 0; index < douban->size; index ++)
	{
		printf("picture: \t%s\n", douban->items[index].picture);
		printf("title  : \t%s\n", douban->items[index].title);
		printf("artist : \t%s\n", douban->items[index].artist);
		printf("url    : \t%s\n\n", douban->items[index].url);
		printf("index %d \n", index);
	}
*/

return 0;



}
