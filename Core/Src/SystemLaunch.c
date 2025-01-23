#include "main.h"
#include "debug.h"

extern const struct user_init * _userInit_start;
extern const struct user_init * _userInit_stop;
static const struct user_init ** initConfig;

void SysDriverInit(void) {
	initConfig = &_userInit_start;
	while(initConfig < &_userInit_stop){
	   (*initConfig)->init();
	   //DEBUG_PRINTF("%s is startup!\n", (*initConfig)->name);
	   initConfig++;
	}

}


