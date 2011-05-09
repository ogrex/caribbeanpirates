#ifndef __UART_DEF_H__
#define __UART_DEF_H__

// com def
#define CFALSE  				-1    // err state
#define CTRUE   				0

#define COM_START				0x00
#define COM_END				0x00

#define COM_EXIT				0x00

#define COM_PLAY				0x11
#define PLAY_INDEX			0x13

#define COM_PAUSE				0x00
#define COM_STOP				0x17
#define COM_NEXT				0x12 	// to be changed
#define COM_LAST				0x00
#define COM_SPK_ON			0x00  // (COM_SPK_ON + vol) control the volume 
#define COM_SPK_OFF			0x00
#define COM_PROG_BAR			0x00  // (COM_PROG_BAR + pos) control the play progress
#define COM_LOCAL_LIST		0x00
#define COM_LOCAL_SONG		0x00  // (COM_LOCAL_SONG + Local_song_id) play Local_song_id-th song
#define COM_NET_LIST			0x00
#define COM_NET_SONG			0x00  // (COM_NET_SONG + Net_song_id) play Net_song_id-th song
#define COM_RADIO_DOUBAN	0x00
#define COM_RADIO_OTHER		0x00  // (COM_RADIO_OTHER + Radio_id) choose Radio_id-th radio  (to be changed)
#define COM_STYLE_CN			0x00
#define COM_STYLE_EA			0x00
#define COM_STYLE_JP			0x00
#define COM_PICTURE			0x00  // how to deal with the additional info ? (to be changed)
#define COM_TITLE				0x03
#define COM_ARTIST			0x04

#define COM_P_TYPE			0x00  // maybe no use (to be changed)
#define COM_P_LEN				0x00
#define COM_T_LEN				0x00
#define COM_A_LEN				0x00
#define COM_LIST_LEN			0x00

#define JPG_INDEX				0x15
#define JPG_LENGTH_END		0x06
#define JPG_END				0x07
#define COM_JPG				0x08
#define NEW_LIST				0x09
#define JPG_BEGIN				0x24
#define LOADLIST_BEGIN		0x18
#define LOADLIST_END			0x19
#define ARTIST_BEGIN			0x25
#define TITLE_INDEX			0x26
#define TITLE_LENGTH_END	0x27
#define TITLE_END				0x19
#define ARTIST_INDEX			0x2a
#define ARTIST_LENGTH_END	0x2b
#define ARTIST_END			0x2c

#define STOP					0x16

#define	uart_dev		 "/dev/ttyS1"

#define TIME_OUT				2000
#define JPG_SIZE				10000

#endif
