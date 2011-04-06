#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>



#include "mp3dec.h"
#include "rtdef.h"

#define MP3_AUDIO_BUF_SZ    (5 * 1024)
#ifndef MIN
#define MIN(x, y)			((x) < (y)? (x) : (y))
#endif


#define MP3_DECODE_MP_CNT   2
#define MP3_DECODE_MP_SZ    2560

rt_uint8_t mp3_fd_buffer[MP3_AUDIO_BUF_SZ];
rt_uint32_t current_offset = 0;
int current_sample_rate = 0;


int fd;	/* sound device file descriptor */



struct mp3_decoder
{
    /* mp3 information */
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
    rt_uint32_t frames;

    /* mp3 file descriptor */
	rt_size_t (*fetch_data)(void* parameter, rt_uint8_t *buffer, rt_size_t length);
	void* fetch_parameter;

    /* mp3 read session */
    rt_uint8_t *read_buffer, *read_ptr;
    rt_int32_t  read_offset;
    rt_uint32_t bytes_left, bytes_left_before_decoding;

};


void mp3_decoder_init(struct mp3_decoder* decoder)
{

	/* init read session */
	decoder->read_ptr = 0;
	decoder->bytes_left_before_decoding = decoder->bytes_left = 0;
	decoder->frames = 0;

    decoder->read_buffer = &mp3_fd_buffer[0];
	if (decoder->read_buffer == 0) return;

    decoder->decoder = MP3InitDecoder();


}


struct mp3_decoder* mp3_decoder_create()
{
    struct mp3_decoder* decoder;
	/* allocate object */
    decoder = (struct mp3_decoder*) malloc (sizeof(struct mp3_decoder));
    if (decoder != 0)
    {
        mp3_decoder_init(decoder);
    }

    return decoder;
}



static rt_int32_t mp3_decoder_fill_buffer(struct mp3_decoder* decoder)
{
	rt_size_t bytes_read;
	rt_size_t bytes_to_read;

	// rt_kprintf("left: %d. refilling inbuffer...\n", decoder->bytes_left);
	if (decoder->bytes_left > 0)
	{
		// better: move unused rest of buffer to the start
		memmove(decoder->read_buffer, decoder->read_ptr, decoder->bytes_left);
	}

	bytes_to_read = (MP3_AUDIO_BUF_SZ - decoder->bytes_left) & ~(512 - 1);

	bytes_read = decoder->fetch_data(decoder->fetch_parameter,
		(rt_uint8_t *)(decoder->read_buffer + decoder->bytes_left),
        bytes_to_read);

	if (bytes_read != 0)
	{
		decoder->read_ptr = decoder->read_buffer;
		decoder->read_offset = 0;
		decoder->bytes_left = decoder->bytes_left + bytes_read;
		return 0;
	}
	else
	{
		printf("can't read more data\n");
		return -1;
	}
}

void* sbuf_alloc()
{
    return malloc(MP3_DECODE_MP_SZ * 2);
}


void sbuf_release(void* ptr)
{
    free(ptr);
}


int mp3_decoder_run(struct mp3_decoder* decoder)
{
	int err;
	rt_uint16_t* buffer;
	rt_uint32_t  delta;


//	if (player_is_playing() != RT_TRUE) return -1;

	if ((decoder->read_ptr == NULL) || decoder->bytes_left < 2*MAINBUF_SIZE)
	{
		if(mp3_decoder_fill_buffer(decoder) != 0)
			return -1;
	}

	// rt_kprintf("read offset: 0x%08x\n", decoder->read_ptr - decoder->read_buffer);
	decoder->read_offset = MP3FindSyncWord(decoder->read_ptr, decoder->bytes_left);
	if (decoder->read_offset < 0)
	{
		/* discard this data */
		printf("outof sync, byte left: %d\n", decoder->bytes_left);

		decoder->bytes_left = 0;
		return 0;
	}

	decoder->read_ptr += decoder->read_offset;
	delta = decoder->read_offset;
	decoder->bytes_left -= decoder->read_offset;
	if (decoder->bytes_left < 1024)
	{
		/* fill more data */
		if(mp3_decoder_fill_buffer(decoder) != 0)
			return -1;
	}

    /* get a decoder buffer */
    buffer = (rt_uint16_t*)sbuf_alloc();
	decoder->bytes_left_before_decoding = decoder->bytes_left;

	err = MP3Decode(decoder->decoder, &decoder->read_ptr,
        (int*)&decoder->bytes_left, (short*)buffer, 0);
	delta += (decoder->bytes_left_before_decoding - decoder->bytes_left);

	current_offset += delta;


	// rt_kprintf("bytes left after decode: %d\n", decoder->bytes_left);

	decoder->frames++;

	if (err != ERR_MP3_NONE)
	{
		switch (err)
		{
		case ERR_MP3_INDATA_UNDERFLOW:
			printf("ERR_MP3_INDATA_UNDERFLOW\n");
			decoder->bytes_left = 0;
			if(mp3_decoder_fill_buffer(decoder) != 0)
			{
				/* release this memory block */
				sbuf_release(buffer);
				return -1;
			}
			break;

		case ERR_MP3_MAINDATA_UNDERFLOW:
			/* do nothing - next call to decode will provide more mainData */
			printf("ERR_MP3_MAINDATA_UNDERFLOW\n");
			break;

		default:
			printf("unknown error: %d, left: %d\n", err, decoder->bytes_left);

			// skip this frame
			if (decoder->bytes_left > 0)
			{
				decoder->bytes_left --;
				decoder->read_ptr ++;
			}
			else
			{
				// TODO
				assert(0);
			}
			break;
		}

		/* release this memory block */
		sbuf_release(buffer);
	}
	else
	{
		int outputSamps;
		/* no error */
		MP3GetLastFrameInfo(decoder->decoder, &decoder->frame_info);


		printf("outputSamps:%d samplerate:%d bps:%d bitPerSample:%d frame:%d ",
			decoder->frame_info.outputSamps,
			decoder->frame_info.samprate,
			decoder->frame_info.bitrate,
			decoder->frame_info.bitsPerSample,
			decoder->frames);
		fflush(stdout);
		printf("\r");

        /* set sample rate */
		if (decoder->frame_info.samprate != current_sample_rate)
		{
			current_sample_rate = decoder->frame_info.samprate;




			//rt_device_control(decoder->snd_device, CODEC_CMD_SAMPLERATE, &current_sample_rate);
		}

		/* write to sound device */
		outputSamps = decoder->frame_info.outputSamps;
		if (outputSamps > 0)
		{
			if (decoder->frame_info.nChans == 1)
			{
				int i;
				for (i = outputSamps - 1; i >= 0; i--)
				{
					buffer[i * 2] = buffer[i];
					buffer[i * 2 + 1] = buffer[i];
				}
				outputSamps *= 2;
			}

		
			write(fd, buffer, outputSamps * sizeof(rt_uint16_t));


	
		//	rt_device_write(decoder->snd_device, 0, buffer, outputSamps * sizeof(rt_uint16_t));
		}
		else
		{
			/* no output */
			sbuf_release(buffer);
		}
	}

	return 0;
}


rt_size_t fd_fetch(void* parameter, rt_uint8_t *buffer, rt_size_t length)
{

	int read_bytes;

	read_bytes = fread((char*)buffer,1,(int) length,(FILE *)parameter);

	if (read_bytes <= 0) return 0;

	return read_bytes;
}
void mp3(char* filename)
{

	struct mp3_decoder* decoder;
	

	FILE *stream;



   if( (stream  = fopen(filename, "rb" )) == NULL )
	   printf( "The file %s was not opened\n" ,filename);
   else
      printf( "The file %s was opened\n"  ,filename);


		decoder = mp3_decoder_create();
		if (decoder != NULL)
		{
			decoder->fetch_data = fd_fetch;
			decoder->fetch_parameter = stream;
//
			current_offset = 0;
//
			while (mp3_decoder_run(decoder) != -1);
//
			/* delete decoder object */
			//mp3_decoder_delete(decoder);
		}
		fclose(stream);

}




int main()
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


  arg = 22050;	   /* sampling rate */
  status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
  if (status == -1)
    perror("SOUND_PCM_WRITE_WRITE ioctl failed");




mp3("/home/bin/Business.mp3");

return 0;
}
