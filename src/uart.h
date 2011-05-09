#ifndef __UART_H__
#define __UART_H__


void set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);
ssize_t	tread(int fd, void *buf, size_t nbytes, unsigned int timout);
ssize_t	uart_read(int fd, void *buf, size_t nbytes, unsigned int timout);
int OpenDev(char *Dev);
int Uart_Init(char *Dev);

#endif
