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

static int fd_i2c;

int i2c_open_cypress(void)
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

void adk_message_send(unsigned char button_status)
{
    unsigned char adk_buf[64] = {0};
    switch (button_status)
    {
        case NOT_TOUCH:

            break;
        case PROX:

            break;
        case VOL_UP:

            system(" adk-message-send 'button_pressed{button:\"VOLUP\"}' ");
            break;
        case VOL_DOWN:

            system(" adk-message-send 'button_pressed{button:\"VOLDOWN\"}' ");
            break;
        case PLAY_PAUSE:

            system(" adk-message-send 'button_pressed{button:\"PLAY\"}' ");
            break;
        case LIBERTY_PULL:

            system(" adk-message-send 'button_pressed{button:\"ACTION\"}' ");
            break;
        case FORMATION:

            system(" adk-message-send 'button_pressed{button:\"FORMATION\"}' ");
            break;
        default:

            break;
    }

}

int is_buttons(unsigned char recbuf)
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
    int i;
    int button_test_times = 0;
    unsigned char read_reg= 0;
    unsigned char recbuf[17] = {0};
    unsigned char last_status = 0;
    unsigned char release_flag = 0;
    unsigned char release_status = 0;
    unsigned char result_status = 0;
	
    if(argc > 1)
    {
        for(i = 0; i < argc; i++)
        {
            printf("Argument %d is %s\n", i, argv[i]);
        }
    }

    if(i2c_open_cypress() != 0)
    {
        printf("i2c can't open cypress!\n");
        return -1;
    }


    while(1)
    {
        usleep(200000);
		i2c_read_cypress(0x08, &read_reg, recbuf, 17);
		if(is_buttons(recbuf[16]) != 0)
        {
            if(last_status != recbuf[16])//判断是否有变化
            {
                if(release_flag == 1)//判断是否是按键松开了
                {
                    release_flag = 0;
                    result_status = release_status;
                    adk_message_send(result_status);
                    //printf(".%d\n",++button_test_times);
                }
                release_flag = 1; //按键松开的标志
                if ((recbuf[16] == 0)|(recbuf[16] == 1)) //判断变化的状态是 proximity 还是 notouch
                {
                    release_flag=0;
                    result_status = recbuf[16];
                    adk_message_send(result_status);
                }
                release_status = recbuf[16]; //产生变化时候的状态
            }
            last_status = recbuf[16]; //记录接收到的前一个状态
        }
        else
        {
            printf(".%d \n",recbuf[16]);
        }
		
		
    }

    return 0;
}


