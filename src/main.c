#include <stdio.h>
#include <unistd.h>
#include "mp3.h"
#include "device.h"
int main()
{
int i;
	char xx[50]="http://190.220.157.52:8000";



//struct file_item* file=(struct file_item*) malloc(sizeof(struct file_item));
//memset( file,0,sizeof(struct file_item));
//sd_item(file);
//for(i=0;i<file->size;i++) printf("%s\n",file->items[i]);



init_dev(22050);


//sprintf(xx,"%s%s","/mnt/",file->items[2]);
//mp3("/nfs/home/caribbeanpirates/bin/test.mp3");
//ice_mp3(xx);
douban_radio(2);
return 0;



}
