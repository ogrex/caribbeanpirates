#include <stdio.h>
#include "mp3.h"

int main()
{
	char *xx="http://190.220.157.52:8000";

init_dev(22050);

ice_mp3(xx);
douban_radio(2);
return 0;



}
