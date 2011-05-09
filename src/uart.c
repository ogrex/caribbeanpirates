/* 
** $Id: button.c,v 1.16 2007-10-25 08:25:16 weiym Exp $
**
** Listing 20.1
**
** button.c: Sample program for MiniGUI Programming Guide
**     Usage of BUTTON control.
**
** Copyright (C) 2004 ~ 2007 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/

# include <sys/mman.h>
#include "uart.h"

#include "http.h"
#include "douban_radio.h"

/*********************************************************************************************
*************************************串口设置函数******************************************
**********************************************************************************************/
/***@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void*/

int speed_arr[] = { B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200,38400,  19200,  9600,  4800,  2400,  1200,  300,
	    115200,38400,  19200,  9600, 4800, 2400, 1200,  300, };
void set_speed(int fd, int speed)
{
  int   i;
  int   status;
  struct termios   Opt;
  tcgetattr(fd, &Opt);
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
   {
   	if  (speed == name_arr[i])
   	{
   	    tcflush(fd, TCIOFLUSH);
    	cfsetispeed(&Opt, speed_arr[i]);
    	cfsetospeed(&Opt, speed_arr[i]);
    	status = tcsetattr(fd, TCSANOW, &Opt);
    	if  (status != 0)
            //perror("tcsetattr fd1");
     	return;
     	}
   tcflush(fd,TCIOFLUSH);
   }
}


/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄*
*@param  databits 类型  int 数据位   取值 为 7 或者8*
*@param  stopbits 类型  int 停止位   取值为 1 或者2*
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	
 if  ( tcgetattr( fd,&options)  !=  0)
  {
  	perror("Can't Setup Serial!\n");
  	return -1;
  }
		//串口的参数的其他设置方法，在新的read函数之下貌似没有什么影响
		options.c_cflag |= CLOCAL | CREAD; 
      options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
      options.c_oflag &= ~OPOST; 
		options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

/*
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  //Input
	options.c_oflag  &= ~OPOST;   //Output
*/
/*
	options.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);		//Runge
   options.c_cflag &= ~CSIZE;
  */
  switch (databits) //设置数据位数
  {
  	case 7:
  		options.c_cflag |= CS7;
  		break;
  	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		return -1;
	}
  switch (parity)
  	{
  	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;  // Clear parity enable 
		options.c_iflag &= ~INPCK;     //Enable parity checking 
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  //设置为奇效验
		options.c_iflag |= INPCK;             // Disnable parity checking
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;    // Enable parity
		options.c_cflag &= ~PARODD;   //转换为偶效验
		options.c_iflag |= INPCK;      // Disnable parity checking
		break;
	case 'S':
	case 's': 								 //as no parity
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		return -1;
		}
  // 设置停止位 
  switch (stopbits)
  	{
  	case 1:
  		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		return -1;
	}
  //Set input parity option 
  if (parity != 'n')
    options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 1; // 15 seconds
    options.c_cc[VMIN] = 0;

  tcflush(fd,TCIFLUSH); //Update the options and do it NOW
  if (tcsetattr(fd,TCSANOW,&options) != 0)
  	{
  		perror("Can't Enable Serial!\n");
		return -1;
	}
  return 0;
 }

//重写了read函数，函数中使用了select函数 , 每次读取一个字节的数据
ssize_t	tread(int fd, void *buf, size_t nbytes, unsigned int timout)
{
	int nfds;
	fd_set readfds;
	struct timeval tv;

	tv.tv_sec = timout;
	tv.tv_usec = 1;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	nfds = select(fd+1, &readfds, NULL, NULL, &tv);
	if (nfds <= 0) 
	{
		if (nfds == 0)
			errno = ETIME;
		return(-1);
	}
	return(read(fd, buf, nbytes));
}

//串口每次读的长度不一样，所以采用控制读多少数据
ssize_t	uart_read(int fd, void *buf, size_t nbytes, unsigned int timout)
{
       size_t      nleft;
       ssize_t     nread;
 
       nleft = nbytes;
       while (nleft > 0) 
		{
              if ((nread = tread(fd, buf, nleft, timout)) < 0) 
				   {
                     if (nleft == nbytes)
                            return(-1); /* error, return -1 */
                     else
                            break;      /* error, return amount read so far */
              } else if (nread == 0) 
					 {
                     break;          /* EOF */
             				   }
              nleft -= nread;
              buf += nread;
   		    }
       return(nbytes - nleft);      /* return >= 0 */
}




/**
*@breif 打开串口
*/
int OpenDev(char *Dev)
{
int	fd = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
	if (-1 == fd)
		{ /*设置数据位数*/
			perror("Can't Open Serial Port");
			return -1;
		}
	else
	return fd;

}

int Uart_Init(char *Dev)					//打开串口 设置波特率115200 数据位8 两个结束位 奇偶校验位
{
	int fd;
	if((fd = OpenDev(Dev)) > 0)
	{
		//set_speed(fd, 115200);
		set_speed(fd, 9600);
		if (set_Parity(fd,8,2,'N')== -1)
		  {
			printf("Set Parity Error\n");
			return -1;
		}
		else return fd;
	}
	else
	{
		return -1;	
	}
	
}
/********************************************************************************************
**************************************串口设置函数结束***********************************
*********************************************************************************************/

/*
int send_list(struct douban_radio *douban)
{
	int index;
	for(index = 0; index < 10; index++)
	{
		
	}
} 
*/
