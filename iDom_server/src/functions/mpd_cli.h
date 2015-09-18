#ifndef MPD_CLI_H
#define MPD_CLI_H

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libmpd-1.0/libmpd/libmpd.h>
#include <libmpd-1.0/libmpd/debug_printf.h>
#include "../blockQueue/blockqueue.h"
#include "../wiadomosc/wiadomosc.h"
#include "../functions/functions.h"

void  *main_mpd_cli(void *data );

#endif // MPD_CLI_H
