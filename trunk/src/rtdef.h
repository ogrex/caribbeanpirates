#ifndef __RT_DEF_H__
#define __RT_DEF_H__




#define WIN_32
/* date type defination					*/
typedef signed 	 char  					rt_int8_t;
typedef signed 	 short 					rt_int16_t;
typedef signed 	 long  					rt_int32_t;
typedef unsigned char  					rt_uint8_t;
typedef unsigned short 					rt_uint16_t;
typedef unsigned long  					rt_uint32_t;
typedef int 							rt_bool_t;

/* 32bit CPU */
typedef long 							rt_base_t;
typedef unsigned long 					rt_ubase_t;

/* RT-Thread definitions */
typedef rt_base_t						rt_err_t;		/* Type for error number.					*/
typedef rt_uint32_t						rt_time_t;		/* Type for time stamp. 					*/
typedef rt_uint32_t						rt_tick_t;		/* Type for tick count. 					*/
typedef rt_base_t						rt_flag_t;		/* Type for flags. 							*/
typedef rt_ubase_t						rt_size_t;		/* Type for size number.					*/
typedef rt_ubase_t						rt_dev_t;		/* Type for device.							*/
typedef rt_uint32_t						rt_off_t;		/* Type for offset.							*/

/* RT-Thread bool type definitions 		*/
#define RT_TRUE 						1
#define RT_FALSE 						0

/* maximun value of base type 			*/
#define RT_UINT8_MAX					0xff			/* Maxium number of UINT8.					*/
#define RT_UINT16_MAX					0xffff			/* Maxium number of UINT16.					*/
#define RT_UINT32_MAX					0xffffffff		/* Maxium number of UINT32.					*/
#define RT_TICK_MAX						RT_UINT32_MAX	/* Maxium number of tick                    */




/**
 * @addtogroup Error
 */
/*@{*/
/* RT-Thread error code definitions */
#define RT_EOK							0				/* There is no error 						*/
#define RT_ERROR						1				/* A generic error happens 					*/
#define RT_ETIMEOUT						2				/* Timed out 								*/
#define RT_EFULL						3				/* The resource is full						*/
#define RT_EEMPTY						4				/* The resource is empty 					*/
#define RT_ENOMEM						5				/* No memory								*/
#define RT_ENOSYS						6				/* No system 								*/
#define RT_EBUSY						7				/* Busy										*/
/*@}*/

#endif

