#include "rtdef.h"
#include "netbuffer.h"
#include "stdio.h"
#include "playerui.h"

#include <memory.h>


#define MP3_DECODE_MP_CNT   2
#define MP3_DECODE_MP_SZ    2560

/* netbuf worker stat */
#define NETBUF_STAT_STOPPED		0
#define NETBUF_STAT_BUFFERING	1
#define NETBUF_STAT_SUSPEND		2
#define NETBUF_STAT_STOPPING	3


/* net buffer module */
struct net_buffer
{
    /* read index and save index in the buffer */
	rt_size_t read_index, save_index;

    /* buffer data and size of buffer */
	rt_uint8_t* buffer_data;
	rt_size_t data_length;
	rt_size_t size;

	/* buffer ready water mater */
	rt_uint32_t ready_wm, resume_wm;
	rt_bool_t is_wait_ready;
    //rt_sem_t wait_ready, wait_resume;

	/* netbuf worker stat */
	rt_uint8_t stat;
};



struct net_buffer_job
{
	rt_size_t (*fetch)(rt_uint8_t* ptr, rt_size_t len, void* parameter);
	void (*close)(void* parameter);

	void* parameter;
};






static struct net_buffer _netbuf;

/* netbuf worker public API */
rt_size_t net_buf_read(rt_uint8_t* buffer, rt_size_t length)
{
    rt_size_t data_length, read_index;
	//rt_uint32_t level;

	data_length = _netbuf.data_length;

    if ((data_length == 0) &&
		(_netbuf.stat == NETBUF_STAT_BUFFERING || _netbuf.stat == NETBUF_STAT_SUSPEND))
    {
    	//rt_err_t result;

        /* buffer is not ready. */
        _netbuf.is_wait_ready = RT_TRUE;
		printf("wait ready, data len: %d, stat %d\n", data_length, _netbuf.stat);
		/* set buffer status to buffering */
		player_set_buffer_status(RT_TRUE);
      //  result = rt_sem_take(_netbuf.wait_ready, RT_WAITING_FOREVER);

		/* take semaphore failed, netbuf worker is stopped */
		//if (result != RT_EOK) return 0;
    }

    /* get read and save index */
    read_index = _netbuf.read_index;
    /* re-get data legnth */
    data_length = _netbuf.data_length;

	/* set the length */
	if (length > data_length) length = data_length;

	// rt_kprintf("data len: %d, read idx %d\n", data_length, read_index);
    if (data_length > 0)
    {
        /* copy buffer */
        if (_netbuf.size - read_index > length)
        {
           memcpy(buffer, &_netbuf.buffer_data[read_index],
                length);
			_netbuf.read_index += length;
        }
        else
        {
            memcpy(buffer, &_netbuf.buffer_data[read_index],
                _netbuf.size - read_index);
            memcpy(&buffer[_netbuf.size - read_index],
                &_netbuf.buffer_data[0],
                length - (_netbuf.size - read_index));
			_netbuf.read_index = length - (_netbuf.size - read_index);
        }

		/* update length of data in buffer */
		//level = rt_hw_interrupt_disable();
		_netbuf.data_length -= length;
		data_length = _netbuf.data_length;

        if ((_netbuf.stat == NETBUF_STAT_SUSPEND) && data_length < _netbuf.resume_wm)
        {
        	_netbuf.stat = NETBUF_STAT_BUFFERING;
			//rt_hw_interrupt_enable(level);

			/* resume netbuf worker */
			// rt_kprintf("stat[suspend] -> buffering\n");
          //  rt_sem_release(_netbuf.wait_resume);
        }
		else
		{
		//	rt_hw_interrupt_enable(level);
		}
    }
	
    return length;
}



int net_buf_start_job(rt_size_t (*fetch)(rt_uint8_t* ptr, rt_size_t len, void* parameter),
	void (*close)(void* parameter),
	void* parameter)
{
	struct net_buffer_job job;
	//rt_uint32_t level;

	/* job message */
	job.fetch = fetch;
	job.close = close;
	job.parameter = parameter;

	//level = rt_hw_interrupt_disable();
	/* check netbuf worker is stopped */
	if (_netbuf.stat == NETBUF_STAT_STOPPED)
	{
		/* change stat to buffering if netbuf stopped */
		_netbuf.stat = NETBUF_STAT_BUFFERING;
	//	rt_hw_interrupt_enable(level);

		printf("stat[stoppped] -> buffering\n");

		//rt_mq_send(_netbuf_mq, (void*)&job, sizeof(struct net_buffer_job));
		return 0;
	}
//	rt_hw_interrupt_enable(level);

	return -1;
}