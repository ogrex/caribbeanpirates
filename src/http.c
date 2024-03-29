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
//#include "uart_def.h"
#ifdef WIN_32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
bool http_session_init()
{
// Initialize Winsock
iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
if (iResult != 0) {
    printf("WSAStartup failed: %d\n", iResult);
    return 1;
}
 printf("initiates use of the Winsock DLL \n", iResult);
 return 0;

}
#endif

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "28992"
#define MAX_MSG_SIZE 1024

#define HTTP_RCV_TIMEO	6000 /* 6 second */


	//WSADATA wsaData;
	int iResult;

const char _http_get[] = "GET %s HTTP/1.1\r\n"
						"Host: %s\r\n"
						"Accept-Language: zh-cn"
						"Accept-Charset: utf-16"
						"User-Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; )\r\n"
						"Connection: Keep-Alive\r\n"
						"Cookie: name=\"RT-Thread\"; ac=\"1281620086\"\r\n\r\n";
				


const char _shoutcast_get[] = "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: RT-Thread HTTP Agent\r\nIcy-MetaData: 1\r\nConnection: close\r\n\r\n";
rt_uint32_t total = 0;


void test ()
{
int cli_sockfd;

int retval;

struct hostent	*lphost;

char *cliaddr="127.0.0.1";

char *server_name="www.baidu.com";




struct sockaddr_in ser_addr,cli_addr;

cli_sockfd=socket(AF_INET,SOCK_STREAM,0);//AF_INET:IPv4 address family. SOCK_STREAM: Uses TCP.



if(cli_sockfd<0)
{
fprintf(stderr,"socker Error:%d\n",cli_sockfd);
return ;
}

printf("socket descriptor:%d \n", cli_sockfd);







memset(&ser_addr,0,sizeof(struct sockaddr_in));
memset(&cli_addr,0,sizeof(struct sockaddr_in));

cli_addr.sin_family=AF_INET;
cli_addr.sin_addr.s_addr=INADDR_ANY;//inet_addr(cliaddr);
cli_addr.sin_port = htons(27015);
printf("client address:\t%s\t%d \n", cliaddr,cli_addr.sin_addr.s_addr);






ser_addr.sin_family=AF_INET;
lphost= gethostbyname(server_name);
ser_addr.sin_addr.s_addr=((struct in_addr *)lphost->h_addr)->s_addr;//inet_addr(seraddr);
ser_addr.sin_port=ntohs(80);
printf("server address:\t%s\t%d \n", server_name,ser_addr.sin_addr.s_addr);


/*
if(bind(cli_sockfd,(struct sockaddr*)&cli_addr,sizeof(struct sockaddr_in))<0)
{
printf("Bind Error\n");
return 0;
}
printf("Bind Success!\n");
*/
// For client applications, the socket can become bound implicitly to a local address using connect()


if(connect(cli_sockfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))!=0)
{

printf("Connect Error\n");

//WSACleanup();

return ;
}
printf("Connect Success!\n\n");

char msg[MAX_MSG_SIZE]="GET / HTTP/1.1\r\n"
	"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg\r\n"
	"Accept-Language: zh-cn\r\n"
	"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; )\r\n"
	"Host: www.baidu.com\r\n\r\n\r\n";


retval=send(cli_sockfd,msg,sizeof(msg),0);
printf("Message Sended:%d/%d\n\n",retval,sizeof(msg));
printf("%s",msg);




while(retval>0 )
{
memset(msg,0,sizeof(msg));
retval=recv(cli_sockfd,msg,MAX_MSG_SIZE,0); 
printf("\n\nReceived:%d\n\n",retval);
printf("%.1024s",msg);
}



close(cli_sockfd);
//WSACleanup();
}


/*
 * resolve server address
 * @param server the server sockaddress
 * @param url the input URL address, for example, http://www.rt-thread.org/index.html
 * @param host_addr the buffer pointer to save server host address
 * @param request the pointer to point the request url, for example, /index.html
 *
 * @return 0 on resolve server address OK, others failed
 */
int http_resolve_address(struct sockaddr_in *server, const char * url, char *host_addr, char** request)
{
	char *ptr;
	char port[6] = "80"; /* default port of 80(HTTP) */
	int i = 0, is_domain;
	struct hostent	*lphost;;//struct hostent *hptr;



	printf("url:\t%s\n",url);
	/* strip http: */
	ptr = (char *)strchr(url, ':');
	if (ptr != NULL)
	{
		url = ptr + 1;
	}

	/* URL must start with double forward slashes. */
	if((url[0] != '/') || (url[1] != '/' )) return -1;

	url += 2; is_domain = 0;
	i = 0;
	/* allow specification of port in URL like http://www.server.net:8080/ */
	while (*url)
	{
		if (*url == '/') break;
		if (*url == ':')
		{
			unsigned char w;
			for (w = 0; w < 5 && url[w + 1] != '/' && url[w + 1] != '\0'; w ++)
				port[w] = url[w + 1];
			
			/* get port ok */
			port[w] = '\0';
			url += w + 1;
			break;
		}

		if ((*url < '0' || *url > '9') && *url != '.')
			is_domain = 1;
		host_addr[i++] = *url;
		url ++;
	}
	*request = (char*)url;
	printf("port:\t%s\nis_domain:\t%d\n",port,is_domain);

	/* get host addr ok. */
	host_addr[i] = '\0';

	if (is_domain)
	{
		/* resolve the host name. */
		lphost = gethostbyname(host_addr);
		if(lphost == 0)
		{
			printf("HTTP: failed to resolve domain '%s'\n", host_addr);
			return -1;
		}
		server->sin_addr.s_addr=((struct in_addr *)lphost->h_addr)->s_addr;
	}
	else
	{

		server->sin_addr.s_addr=inet_addr(host_addr);

	}
	/* set the port */
	printf("inet_addr:\t0x%X\n", server->sin_addr.s_addr);

	server->sin_port = ntohs((int) strtol(port, NULL, 10));
	server->sin_family = AF_INET;

	return 0;
}




//
// When a request has been sent, we can expect mime headers to be
// before the data.  We need to read exactly to the end of the headers
// and no more data.  This readline reads a single char at a time.
//
int http_read_line( int socket, char * buffer, int size )
{
	char * ptr = buffer;
	int count = 0;
	int rc;

	// Keep reading until we fill the buffer.
	while ( count < size )
	{
		rc = recv( socket, ptr, 1, 0 );
		if ( rc <= 0 ) return rc;

		if ((*ptr == '\n'))
		{
			ptr ++;
			count++;
			break;
		}

		// increment after check for cr.  Don't want to count the cr.
		count++;
		ptr++;
	}

	// Terminate string
	*ptr = '\0';

	// return how many bytes read.
	return count;
}


int http_is_error_header(char *mime_buf)
{
	char *line;
	int i;
	int code;

	line = strstr(mime_buf, "HTTP/1.");
	line += strlen("HTTP/1.");

	// Advance past minor protocol version number
	line++;

	// Advance past any whitespace characters
	while((*line == ' ') || (*line == '\t')) line++;

	// Terminate string after status code
	for(i = 0; ((line[i] != ' ') && (line[i] != '\t')); i++);
	line[i] = '\0';

	code = (int)strtol(line, 0, 10);
	if( code == 200 )
		return 0;
	else
		return code;
}


int shoutcast_is_error_header(char *mime_buf)
{
	char *line;
	int i;
	int code;

	line = strstr(mime_buf, "ICY");
	line += strlen("ICY");

	// Advance past minor protocol version number
	line++;

	// Advance past any whitespace characters
	while((*line == ' ') || (*line == '\t')) line++;

	// Terminate string after status code
	for(i = 0; ((line[i] != ' ') && (line[i] != '\t')); i++);
	line[i] = '\0';

	code = (int)strtol(line, 0, 10);
	if( code == 200 )
		return 0;
	else
		return code;
}

int http_parse_content_length(char *mime_buf)
{
	char *line;

	line = strstr(mime_buf, "CONTENT-LENGTH:");
	line += strlen("CONTENT-LENGTH:");

	// Advance past any whitepace characters
	while((*line == ' ') || (*line == '\t')) line++;

	return (int)strtol(line, 0, 10);
}


//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
static int http_connect(struct http_session* session,
    struct sockaddr_in * server, char *host_addr, const char *url)
{
	int socket_handle;
	int peer_handle;
	int rc;
	char mimeBuffer[100];
	int timeout = HTTP_RCV_TIMEO;

	if((socket_handle = socket( AF_INET,SOCK_STREAM,0 )) < 0)//AF_INET:IPv4 address family. SOCK_STREAM: Uses TCP.
	{
		printf( "HTTP: SOCKET FAILED\n" );
		return -1;
	}

	printf("socket descriptor:%d \n", socket_handle);

	/* set recv timeout option */
	setsockopt(socket_handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	peer_handle = connect( socket_handle, (struct sockaddr *) server, sizeof(*server));
	if ( peer_handle < 0 )
	{
		printf( "HTTP: CONNECT FAILED %i\n", peer_handle );
		return -1;
	}
	printf("Connect Success!\n\n");



	
		char *buf;
		rt_uint32_t length;

		buf = (char *)malloc (512);
		if (*url)
			length = snprintf(buf, 512, _http_get, url, host_addr);
		else
			length = snprintf(buf, 512, _http_get, "/", host_addr);
		
		rc = send(socket_handle , buf, length, 0);


		 printf("HTTP request Sended:%d/%d\n\n%s", rc,(int)length,buf);

		free(buf);
	

	// We now need to read the header information
	while ( 1 )
	{
		unsigned int i;

		// read a line from the header information.
		rc = http_read_line( socket_handle, mimeBuffer, 100 );
		// rt_kprintf(">> %s\n", mimeBuffer);


		if ( rc < 0 ) return rc;

		// End of headers is a blank line.  exit.
		if (rc == 0) break;
		if ((rc == 2) && (mimeBuffer[0] == '\r')) break;

		printf("%s",mimeBuffer);
		// Convert mimeBuffer to upper case, so we can do string comps
		for(i = 0; i < strlen(mimeBuffer); i++)

			mimeBuffer[i] = toupper(mimeBuffer[i]);

		if(strstr(mimeBuffer, "HTTP/1.")) // First line of header, contains status code. Check for an error code
		{
			rc = http_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("HTTP: status code = %d!\n", rc);
				return -rc;
			}
		}

		if(strstr(mimeBuffer, "CONTENT-LENGTH:"))
		{
			session->size = http_parse_content_length(mimeBuffer);
		}

	}


		
	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return socket_handle;
}






struct http_session* http_session_open(const char* url)
{
	int peer_handle = 0;
	struct sockaddr_in server;
	char *request, host_addr[32];
	struct http_session* session;

    session = (struct http_session*) malloc(sizeof(struct http_session));
	if(session == 0) return 0;

	session->size = 0;
	session->position = 0;
	


	/* Check valid IP address and URL */
	if(http_resolve_address(&server, url, &host_addr[0], &request) != 0)
	{
		free(session);
		return 0;
	}

	// Now we connect and initiate the transfer by sending a
	// request header to the server, and receiving the response header
	if((peer_handle = http_connect(session, &server, host_addr, request)) < 0)
	{
        printf("HTTP: failed to connect to '%s'!\n", host_addr);
		free(session);
		return 0;
	}
	

	printf("\nsession size:\t%d \n",(int)session->size);
	printf("\naudio_buffer\n");
	// http connect returns valid socket.  Save in handle list.
	session->socket = peer_handle;

	/* open successfully */
	return session;
}


rt_size_t http_session_read(struct http_session* session, char *buffer, rt_size_t length)
{
	int bytesRead = 0;
	int totalRead = 0;
	int left = length;

	// Read until: there is an error, we've read "size" bytes or the remote
	//             side has closed the connection.
	do
	{
		bytesRead = recv(session->socket, buffer + totalRead, left, 0);
		if(bytesRead <= 0) break;

		left -= bytesRead;
		totalRead += bytesRead;
	} while(left);

	return totalRead;
}

int http_session_close(struct http_session* session)
{
   close(session->socket); //in windows use closesocket,in linux use close
	free(session);

	return 0;
}





//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
static int shoutcast_connect(struct shoutcast_session* session,
    struct sockaddr_in* server, char* host_addr, const char* url)
{
	int  socket_handle;
	int peer_handle;
	int rc;
	char mimeBuffer[256];
	int timeout = HTTP_RCV_TIMEO;

		char *buf;
		rt_uint32_t length;

	if((socket_handle = socket( AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf( "ICY: SOCKET FAILED\n" );
		return -1;
	}

	printf("socket descriptor:%d \n", socket_handle);

	/* set recv timeout option */
	setsockopt(socket_handle, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(timeout));

	peer_handle = connect( socket_handle, (struct sockaddr *) server, sizeof(*server));
	if ( peer_handle < 0 )
	{
		printf( "ICY: CONNECT FAILED %i\n", peer_handle );
	//	close(socket_handle);
		
		return -1;
	}

	printf("Connect Success!\n\n");

	{


		buf = (char *)malloc (512);
		if (*url)
			length = snprintf(buf, 512, _shoutcast_get, url, host_addr);
		else
			length = snprintf(buf, 512, _shoutcast_get, "/", host_addr);

		rc = send(socket_handle, buf, length, 0);

		printf("SHOUTCAST request Sended:%d/%d\n\n%s", rc,(int)length,buf);
		
		/* release buffer */
		free(buf);
	}

	/* read the header information */
	while ( 1 )
	{
		// read a line from the header information.
		rc = http_read_line(socket_handle, mimeBuffer, 100);
		printf(">>%s", mimeBuffer);

		if ( rc < 0 )
		{
			//lwip_close(socket_handle);
			return rc;
		}

		// End of headers is a blank line.  exit.
		if (rc == 0) break;
		if ((rc == 2) && (mimeBuffer[0] == '\r')) break;

		if(strstr(mimeBuffer, "ICY")) // First line of header, contains status code. Check for an error code
		{
			rc = shoutcast_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("ICY: status code = %d!\n", rc);
			//	lwip_close(socket_handle);
				return -rc;
			}
		}
		
		if (strstr(mimeBuffer, "HTTP/1."))
		{
			rc = http_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("HTTP: status code = %d!\n", rc);
	//			lwip_close(socket_handle);
				return -rc;
			}
		}

		if (strstr(mimeBuffer, "icy-name:"))
		{
			/* get name */
			char* name;

			name = mimeBuffer + strlen("icy-name:");
			session->station_name = strdup(name);
			printf("station name: %s\n", session->station_name);
		}

		if (strstr(mimeBuffer, "icy-br:"))
		{
			/* get bitrate */
			session->bitrate = strtol(mimeBuffer + strlen("icy-br:"), 0, 10);
			printf("bitrate: %d\n", session->bitrate);
		}

		if (strstr(mimeBuffer, "icy-metaint:"))
		{
			/* get metaint */
			session->metaint = strtol(mimeBuffer + strlen("icy-metaint:"), 0, 10);
			printf("metaint: %d\n", (int)session->metaint);
		}
		
		if (strstr(mimeBuffer, "content-type:"))
		{
			/* check content-type */
			if (strstr(mimeBuffer, "audio/mpeg") == 0)
			{
				printf("ICY content is not audio/mpeg.\n");
		//		lwip_close(socket_handle);
				return -1;
			}
		}

		if (strstr(mimeBuffer, "Content-Type:"))
		{
			/* check content-type */
			if (strstr(mimeBuffer, "audio/mpeg") == 0)
			{
				printf("ICY content is not audio/mpeg.\n");
			//	lwip_close(socket_handle);
				return -1;
			}
		}
	}

	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return socket_handle;
}








struct shoutcast_session* shoutcast_session_open(const char* url)
{
	int peer_handle = 0;
	struct sockaddr_in server;
	char *request, host_addr[32];
	struct shoutcast_session* session;

	total = 0;
	
    session = (struct shoutcast_session*) malloc(sizeof(struct shoutcast_session));
	if(session == 0) return 0;

	session->metaint = 0;
	session->current_meta_chunk = 0;
	session->bitrate = 0;
	session->station_name = 0;

	/* Check valid IP address and URL */
	if(http_resolve_address(&server, url, &host_addr[0], &request) != 0)
	{
		free(session);
		return 0;
	}



	if((peer_handle = shoutcast_connect(session, &server, host_addr, request)) < 0)
	{
        printf("SHOUTCAST: failed to connect to '%s'!\n", host_addr);
		if (session->station_name != 0)
			free(session->station_name);
		free(session);
		return 0;
	}


	// http connect returns valid socket.  Save in handle list.
	session->socket = peer_handle;
	/* open successfully */
	return session;
}



rt_size_t shoutcast_session_read(struct shoutcast_session* session, rt_uint8_t *buffer, rt_size_t length)
{
	int bytesRead = 0;
	int totalRead = 0;
	int left = length;
	static rt_uint32_t first_meta_size = 0;

	// Read until: there is an error, we've read "size" bytes or the remote
	//             side has closed the connection.
	do
	{
		bytesRead = recv(session->socket, buffer + totalRead, left, 0);
		if(bytesRead <= 0) 
		{
			break;
		}

		total += bytesRead;

		left -= bytesRead;
		totalRead += bytesRead;
	} while(left);

	/* handle meta */
	if (first_meta_size > 0)
	{
		/* skip meta data */
		memmove(&buffer[0], &buffer[first_meta_size], totalRead - first_meta_size);

		// rt_kprintf("remove meta: len %d\n", first_meta_size);

		totalRead = totalRead - first_meta_size;
		first_meta_size = 0;
		session->current_meta_chunk = totalRead;
	}
	else
	{
		if (session->current_meta_chunk + totalRead == session->metaint)
		{
			rt_uint8_t meta_data;
			recv(session->socket, &meta_data, 1, 0);
		
			/* remove meta data in next packet */
			first_meta_size = meta_data * 16;
			session->current_meta_chunk = 0;

			// rt_kprintf("get meta: len %d\n", first_meta_size);
		}
		else if (session->current_meta_chunk + totalRead > session->metaint)
		{
			int meta_length, next_chunk_length;

			// rt_kprintf("c: %d, total: %d, m: %d\n", session->current_meta_chunk, totalRead, session->metaint);

			/* get the length of meta data */
			meta_length = buffer[session->metaint - session->current_meta_chunk] * 16;
			next_chunk_length = totalRead - (session->metaint - session->current_meta_chunk) - 
				(meta_length + 1);

			// rt_kprintf("l: %d, n: %d\n", meta_length, next_chunk_length);

			/* skip meta data */
			memmove(&buffer[session->metaint - session->current_meta_chunk], 
				&buffer[session->metaint - session->current_meta_chunk + meta_length + 1],
				next_chunk_length);

			/* set new current meta chunk */
			session->current_meta_chunk = next_chunk_length;
			totalRead = totalRead - (meta_length + 1);
		}
		else 
		{
			session->current_meta_chunk += totalRead;
		}
	}

	return totalRead;
}




/*

void getjpg(char * url)
{
		rt_size_t length;
		struct http_session* session;
		char *buffer,*ptr;
		//FILE *stream;
		int uart_fd;
		int nread;
		char protocol_buff[5];
		char data_buff[20];
		
		uart_fd = Uart_Init(uart_dev);

//存放路径
//if((stream  = fopen("/hello.jpg", "w+b" ))==NULL)
//return;
//else printf("file is open!\n");

#ifdef WIN_32
http_session_init();
#endif

session=http_session_open(url);
buffer =(char*)malloc(session->size);
ptr=buffer;

	do
	{
		length = http_session_read(session, ptr, buffer + session->size - ptr);
		if (length <= 0) break;
		ptr += length;
		
	} while (ptr < buffer + session->size);
	//fwrite(buffer,1,(int) session->size,stream);

	printf("here!\n");
	write(uart_fd, p_start, 1);									//发出图像传输的开始信息
	if((nread = uart_read(uart_fd, protocol_buff, 1 , 2)) > 0)//接收反馈回来的信息，并准备传输图像
	{
		if(protocol_buff[0] == COM_START)
		{
			printf("begin to send jpg's length!\n");
			sprintf(data_buff ,"%d",session->size);
			write(uart_fd, data_buff, 20);
			printf("length is send\n\n");
			printf("wait for it's ready!\n");
			if((nread = uart_read(uart_fd, protocol_buff, 1 ,1)) > 0)
			{
				if(protocol_buff[0] == COM_START)
				{	
					printf("ready to send jpg data!\n");
					write(uart_fd, buffer, (int)session->size);
					printf("jpg data is send!\n");
					printf("wait for it's ready!\n");
					if((nread = uart_read(uart_fd, protocol_buff, 1, 1)) > 0)
					{
						if(protocol_buff[0] == COM_START)
						{
							printf("ready to send P_END!\n");
							write(uart_fd, p_end, 1);
							printf("p_end is send!\n");
							printf("wait for it's ready!\n");
							if((nread = uart_read(uart_fd, protocol_buff, 1, 1)) > 0)
							{
								if(protocol_buff[0] == COM_START)
								printf("p_end is send!\n");
								else printf("have not recive p_end!\n");
							}
							else printf("P_END isn't read!\n");
						}
						else printf("not ready!\n");
					}
					else printf("can't begin to send data!\n");
				}
				else printf("have not read com_start!\n");
			}
			else printf("Don't recive com_start!\n");
		}
		else printf("have not read data!\n");
	}
	else printf("error!\n");


free(buffer);
close(uart_fd);
//fclose( stream );
}

*/
