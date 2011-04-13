#include <stdio.h>


#include "playerui.h"

void player_set_buffer_status(rt_bool_t buffering)
{

    if (buffering == RT_TRUE)
        printf("buffering...");
    else
        printf("playing...");
//TODO
}