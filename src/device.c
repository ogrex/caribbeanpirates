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
int fd;	/* sound device file descriptor */
const char *sound_device_names[] = SOUND_DEVICE_NAMES;




void snd_write(rt_uint16_t*buffer, size_t size)
{
write(fd, buffer, size);
}


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



typedef struct SNDPCMContainer {
	snd_pcm_t *handle;
	snd_output_t *log;
	snd_pcm_uframes_t chunk_size;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_format_t format;
	rt_uint16_t channels;
	size_t chunk_bytes;
	size_t bits_per_sample;
	size_t bits_per_frame;

	rt_uint8_t *data_buf;
} SNDPCMContainer_t;

int SNDWAV_SetParams(SNDPCMContainer_t *sndpcm,int num)
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_format_t format;
	rt_uint32_t exact_rate;
	rt_uint32_t buffer_time, period_time;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);



	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(sndpcm->handle, hwparams) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_any\n");
		goto ERR_SET_PARAMS;
	}

	if (snd_pcm_hw_params_set_access(sndpcm->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_access\n");
		goto ERR_SET_PARAMS;
	}

	format= SND_PCM_FORMAT_S16_LE;


	if (snd_pcm_hw_params_set_format(sndpcm->handle, hwparams, format) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_format\n");
		goto ERR_SET_PARAMS;
	}
	sndpcm->format = format;


	/* Set number of channels */
	if (snd_pcm_hw_params_set_channels(sndpcm->handle, hwparams, 2) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_channels\n");
		goto ERR_SET_PARAMS;
	}
	sndpcm->channels =2;

	exact_rate = num;
	if (snd_pcm_hw_params_set_rate_near(sndpcm->handle, hwparams, &exact_rate, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_rate_near\n");
		goto ERR_SET_PARAMS;
	}
	if (num != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n",
			num, exact_rate);
	}
	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max\n");
		goto ERR_SET_PARAMS;
	}
	buffer_time = 208950;
	period_time = buffer_time / 4;

	if (snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, hwparams, &buffer_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near\n");
		goto ERR_SET_PARAMS;
	}

	if (snd_pcm_hw_params_set_period_time_near(sndpcm->handle, hwparams, &period_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near\n");
		goto ERR_SET_PARAMS;
	}
	/* Set hw params */
	if (snd_pcm_hw_params(sndpcm->handle, hwparams) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params(handle, params)\n");
		goto ERR_SET_PARAMS;
	}
	snd_pcm_hw_params_get_period_size(hwparams, &sndpcm->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(hwparams, &sndpcm->buffer_size);
	if (sndpcm->chunk_size == sndpcm->buffer_size) {
		fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)\n"), sndpcm->chunk_size, sndpcm->buffer_size);
		goto ERR_SET_PARAMS;
	}

	sndpcm->bits_per_sample = snd_pcm_format_physical_width(format);
	sndpcm->bits_per_frame = sndpcm->bits_per_sample *2;

	sndpcm->chunk_bytes = sndpcm->chunk_size * sndpcm->bits_per_frame / 8;
//chunk_size:number of frames
	//chunk_bytes=chunk_size*bitperframe/8
	//if chunk_bytes=5120 buffer_time=232200
	printf("%d\n",sndpcm->chunk_size);
printf("%d\n",sndpcm->chunk_bytes);
	return 0;

ERR_SET_PARAMS:
	return -1;
}





SNDPCMContainer_t playback;




int init_dev(int freq)
{
static	char *devicename = "default";

	memset(&playback, 0x0, sizeof(playback));


	if (snd_output_stdio_attach(&playback.log, stderr, 0) < 0) {
		fprintf(stderr, "Error snd_output_stdio_attach\n");
		goto Err;
	}
	if (snd_pcm_open(&playback.handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_open [ %s]\n", devicename);
		goto Err;
	}
	if (SNDWAV_SetParams(&playback, freq) < 0) {
		fprintf(stderr, "Error set_snd_pcm_params\n");
		goto Err;
	}
	//snd_pcm_dump(playback.handle, playback.log);

	return 0;


	//snd_pcm_drain(playback.handle);
	//snd_output_close(playback.log);
	//snd_pcm_close(playback.handle);

	Err:
		if (playback.data_buf) free(playback.data_buf);
		if (playback.log) snd_output_close(playback.log);
		if (playback.handle) snd_pcm_close(playback.handle);
		return -1;
}

ssize_t SNDWAV_WritePcm(rt_uint16_t *buffer, size_t wcount)
{
	ssize_t r;
	ssize_t result = 0;
	rt_uint8_t *data = buffer;

	if (wcount < playback.chunk_size) {
		snd_pcm_format_set_silence(playback.format,
			data + wcount * playback.bits_per_frame / 8,
			(playback.chunk_size - wcount) * playback.channels);
		wcount = playback.chunk_size;
	}
	while (wcount > 0) {
		r = snd_pcm_writei(playback.handle, data, wcount);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < wcount)) {
			snd_pcm_wait(playback.handle, 1000);
		} else if (r == -EPIPE) {
			snd_pcm_prepare(playback.handle);
			fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
		} else if (r == -ESTRPIPE) {
			fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n");
		} else if (r < 0) {
			fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(r));
			exit(-1);
		}
		if (r > 0) {
			result += r;
			wcount -= r;
			data += r * playback.bits_per_frame / 8;
		}
	}
	return result;
}


#else
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
