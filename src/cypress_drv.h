/**
*  @file      cypress_drv.h
*  @brief     cypress_drv
*  @author    Zack Li
*  @date      1 -2020
*  @copyright
*/

#ifndef _CYPRESS_DRV_H_
#define _CYPRESS_DRV_H_

#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <poll.h>


#define NOT_TOUCH		 0
#define PROX      	   	 16
#define VOL_UP        	 17
#define VOL_DOWN         18
#define FORMATION     	 24
#define PLAY_PAUSE    	 20


#endif

