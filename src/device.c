#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <dirent.h>

#include "def.h"
#include "device.h"

int mix_fd;

const char *sound_device_names[] = SOUND_DEVICE_NAMES;




void sd_item(struct file_item* file)
{

	int i=0;
	char mm[30];
	char * mn="/mnt/";	
	struct dirent * direntp;
	DIR * dirp;
	dirp=opendir("/mnt/");

	while ( (file->size<10) && (direntp =readdir(dirp))!=NULL)
	{
		
		if(strstr(direntp->d_name,".mp3")!=NULL) {
			
			sprintf(file->items[file->size],"%s",direntp->d_name);
			
			
			file->size++;
		}

	}
}


#if (defined i386 && defined linux)

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

  snd_pcm_uframes_t frames; 
  snd_pcm_t *handle;
int init_dev(int freq)
{

  snd_pcm_hw_params_t *params;
  int rc;
  int dir;
  unsigned int val;

  int size;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */


  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  val = freq;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);
 /* Set period size to 32 frames. */
  frames = 32;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  size = frames * 4; /* 2 bytes/sample, 2 channels */

 //buffer = (char *) malloc(size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
printf("period time:%d\nbuffersize:%d\n",val,size);
}


void snd_write(rt_uint16_t*buffer, size_t size)
{
static int rc;
rc=snd_pcm_writei(handle, buffer, size/4);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)size/4) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
  
}



#else


int fd;	/* sound device file descriptor */
void snd_write(rt_uint16_t*buffer, size_t size)
{
write(fd, buffer, size);
}



int init_dev(int freq)
{

int arg,status;
  arg = 2;  /* mono or stereo */ 

 /* open sound device */
  fd = open("/dev/dsp", 1);
  if (fd < 0) {
    perror("open of /dev/dsp failed");
    exit(1);
  }

  /* set sampling parameters */
  arg = 16;	   /* sample size */
  status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
  if (status == -1)
    perror("SOUND_PCM_WRITE_BITS ioctl failed");


  arg = 2;  /* mono or stereo */
  status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
  if (status == -1)
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");


  arg = freq;	   /* sampling rate */
  status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
  if (status == -1)
    perror("SOUND_PCM_WRITE_WRITE ioctl failed");

  mix_fd = open("/dev/mixer", O_RDONLY);
  if (mix_fd == -1) {
    perror("unable to open /dev/mixer");
    exit(1);
  }
return 0;
}

#endif
