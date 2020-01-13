/**
*  @file      cypress_drv.c
*  @brief     cypress_drv
*  @author    Zack Li
*  @date      1 -2020
*  @copyright
*/

#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <poll.h>
#include <stdint.h>
#include "cypress_drv.h"


#define I2C_FILE_NAME   "/dev/i2c-3"
#define CYPRESS_I2C_ADDR        0x08

#define RECBUF_SIZE         17
#define BUTTON_STATUS_BIT   16

#define READ_SLEEP_TIME 200000
#define BUTTON_RELEASE  1
#define IS_NOT_BUTTON   0


static int fd_i2c;

static int i2c_open_cypress(void)
{
    int ret;
    int val;

    fd_i2c = open(I2C_FILE_NAME, O_RDWR);

    if(fd_i2c < 0)
    {
        perror("Unable to open i2c control file");

        return -1;
    }

    printf("open i2c file success %d\n",fd_i2c);

    ret = ioctl(fd_i2c, I2C_SLAVE_FORCE, CYPRESS_I2C_ADDR);
    if (ret < 0)
    {
        perror("i2c: Failed to set i2c device address\n");
        return -1;
    }

    printf("i2c: set i2c device address success\n");

    val = 3;
    ret = ioctl(fd_i2c, I2C_RETRIES, val);
    if(ret < 0)
    {
        printf("i2c: set i2c retry times err\n");
    }

    printf("i2c: set i2c retry times %d\n",val);

    return 0;
}

static int i2c_write_cypress(unsigned char dev_addr, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;

    struct i2c_msg messages;


    messages.addr = dev_addr;  //device address
    messages.flags = 0;    //write
    messages.len = len;
    messages.buf = val;  //data

    data.msgs = &messages;
    data.nmsgs = 1;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        perror("---");
        printf("write ioctl err %d\n",fd_i2c);
        return -1;
    }

    return 0;
}

static int i2c_read_cypress(unsigned char addr, unsigned char *reg, unsigned char *val, unsigned char len)
{
    int ret;
    int i;

    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[2];

    messages[0].addr = addr;  //device address
    messages[0].flags = 0;    //write
    messages[0].len = 1;
    messages[0].buf = reg;  //reg address

    messages[1].addr = addr;       //device address
    messages[1].flags = I2C_M_RD;  //read
    messages[1].len = len;
    messages[1].buf = val;

    data.msgs = messages;
    data.nmsgs = 2;

    if(ioctl(fd_i2c, I2C_RDWR, &data) < 0)
    {
        perror("---");
        printf("read ioctl err %d\n",fd_i2c);

        return -1;
    }

    return 0;
}

/*
*	NOT_TOUCH		stop led and delay 15s
*	PROX			start led forever
*	VOL_UP			adk-message-send 'button_pressed{button:\"VOLUP\"}'
*	VOL_DOWN		adk-message-send 'button_pressed{button:\"VOLDOWN\"}'
*	PLAY_PAUSE		adk-message-send 'button_pressed{button:\"PLAY\"}'
*	LIBERTY_PULL	adk-message-send 'button_pressed{button:\"ACTION\"}'
*	FORMATION		adk-message-send 'button_pressed{button:\"FORMATION\"}'
*/
static void adk_message_send(unsigned char button_status)
{
    unsigned char adk_buf[64] = {0};
    switch (button_status)
    {
        case NOT_TOUCH:
			printf(".NOT_TOUCH \n");
            break;
        case PROX:
			printf(".PROX \n");
            break;
        case VOL_UP:
			printf(".VOL_UP \n");
            //system(" adk-message-send 'button_pressed{button:\"VOLUP\"}' ");
            break;
        case VOL_DOWN:
			printf(".VOL_DOWN \n");
            //system(" adk-message-send 'button_pressed{button:\"VOLDOWN\"}' ");
            break;
        case PLAY_PAUSE:
			printf(".PLAY_PAUSE \n");
            //system(" adk-message-send 'button_pressed{button:\"PLAY\"}' ");
            break;
        case LIBERTY_PULL:
			printf(".LIBERTY_PULL \n");
            //system(" adk-message-send 'button_pressed{button:\"ACTION\"}' ");
            break;
        case FORMATION:
			printf(".FORMATION \n");
            //system(" adk-message-send 'button_pressed{button:\"FORMATION\"}' ");
            break;
        default:

            break;
    }

}

static int is_buttons(unsigned char recbuf)
{
    unsigned char whats = 1;
    switch (recbuf)
    {
        case NOT_TOUCH:
            break;
        case PROX:
            break;
        case VOL_UP:
            break;
        case VOL_DOWN:
            break;
        case PLAY_PAUSE:
            break;
        case LIBERTY_PULL:
            break;
        case FORMATION:
            break;
        default:
            whats = 0;
            break;
    }
    return whats;
}

int main(int argc, char* argv[])
{
    //int button_test_times = 0;
    unsigned char read_reg= 0;
    unsigned char recbuf[RECBUF_SIZE] = {0};
    unsigned char last_status = 0;
    unsigned char release_flag = IS_NOT_BUTTON;
    unsigned char release_status = 0;
    unsigned char result_status = NOT_TOUCH;
	unsigned char prox_status = 0;

    if(i2c_open_cypress() != 0)
    {
        printf("i2c can't open cypress!\n");
        return -1;
    }

    while(1)
    {
        usleep(READ_SLEEP_TIME);
        i2c_read_cypress(CYPRESS_I2C_ADDR, &read_reg, recbuf, RECBUF_SIZE);
        if(is_buttons(recbuf[BUTTON_STATUS_BIT]))
        {
            if(last_status != recbuf[BUTTON_STATUS_BIT])//whether the capsense status change
            {
                if(release_flag == BUTTON_RELEASE)//whether the button release
                {
                    release_flag = IS_NOT_BUTTON;
                    result_status = release_status;
                    adk_message_send(result_status);
                    //printf(".%d\n",++button_test_times);
                }
                release_flag = BUTTON_RELEASE; //modify the button release flag
                if ((recbuf[BUTTON_STATUS_BIT] == NOT_TOUCH)|(recbuf[BUTTON_STATUS_BIT] == PROX)) //the change is  proximity or notouch
                {
                    release_flag = IS_NOT_BUTTON;
                    result_status = recbuf[BUTTON_STATUS_BIT];
					//avoid proximity always trigger
					if(prox_status != recbuf[BUTTON_STATUS_BIT]){
                    	adk_message_send(result_status);
					}
					prox_status = recbuf[BUTTON_STATUS_BIT];
				}
                release_status = recbuf[BUTTON_STATUS_BIT]; //save the last button status
            }
            last_status = recbuf[BUTTON_STATUS_BIT]; //save the last status
        }
        else
        {
            printf(".%d \n", recbuf[BUTTON_STATUS_BIT]);
        }

    }

    return 0;
}


