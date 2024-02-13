/*
 * linux_spi.cpp
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "inc/linux_spi.h"
// #include "inc/gpiod.h"
#include "inc/gpio_dart.h"


C_Linux_SPI::C_Linux_SPI() :
    _errno(ERROR_DEV_NOT_OPEN),
    _dev_fd(-1)
{

    memset(&_spi_tr, 0, sizeof(struct spi_ioc_transfer));
}



C_Linux_SPI::~C_Linux_SPI()
{
    dev_close();
}



int C_Linux_SPI::dev_open(const char *spi_dev_name)
{ 
    int ret = 1;
    if( !spi_dev_name )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }
    dev_close(); //close old dev

    _dev_fd = open(spi_dev_name, O_RDWR);
    if( _dev_fd == -1 )
    {
        _errno = ERROR_CANT_OPEN_DEV;
        return -1;
    }
    
    /* create the pin for spi CS pin */
    ret = gpio_export();
    if (ret > 0)   {
        perror("\nError gpio_export: linux_spi.cpp \n");
    }
    ret = gpio_dir();
    if (ret > 0)   {
        perror("\nError gpio_dir: linux_spi.cpp \n");
    }
    ret = gpio_set("1");    // desables spi slave, mms
    if (ret > 0)   {
        perror("\nError gpio_set: linux_spi.cpp \n");
    }
    printf("\n write pin successful \n");
    return 0; //good job
}



void C_Linux_SPI::dev_close()
{
    //gpio_CS_SPI_MMS_SS = IMX_GPIO_NR(GPIO_PORT_4_SPI_SS_MMS, SS_PIN_SPI_MMS);
    // fd_CS_SPI_MMS_SS = open("/sys/class/gpio/unexport", O_WRONLY);

    // sprintf(buf_CS_SPI_MMS_SS, "%d", gpio_CS_SPI_MMS_SS);

    // if ((write(fd_CS_SPI_MMS_SS, buf_CS_SPI_MMS_SS, strlen(buf_CS_SPI_MMS_SS))) == -1)    {
    //     perror("\n\tfd for GPIO CS fail.3 Please increase the size of buf buf_CS_SPI_MMS_SS[MAX_BUF] \n");
    // }  
    // close(fd_CS_SPI_MMS_SS);



    if( _dev_fd != -1 )
        close(_dev_fd);


    _dev_fd = -1;
    _errno  = ERROR_DEV_NOT_OPEN;

}



int C_Linux_SPI::set_mode(unsigned char mode)
{

    if( _dev_fd == -1 )
        return -1;


    if( ioctl(_dev_fd, SPI_IOC_WR_MODE, &mode) == -1 )
    {
        _errno = ERROR_CANT_SET_MODE;
        return -1;
    }

    if( ioctl(_dev_fd, SPI_IOC_RD_MODE, &mode) == -1 )
    {
        _errno = ERROR_CANT_SET_MODE;
        return -1;
    }


    return 0; //good job
}



int C_Linux_SPI::get_mode(unsigned char *mode)
{

    if( _dev_fd == -1 )
        return -1;


    if( !mode )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }


    if( ioctl(_dev_fd, SPI_IOC_RD_MODE, mode) == -1 )
    {
        _errno = ERROR_CANT_GET_MODE;
        return -1;
    }


    return 0; //good job
}



int C_Linux_SPI::set_bits_per_word(unsigned char bits_per_word)
{

    if( _dev_fd == -1 )
        return -1;


    if( ioctl(_dev_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1 )
    {
        _errno = ERROR_CANT_SET_BITS;
        return -1;
    }


    _spi_tr.bits_per_word = bits_per_word;


    return 0; //good job
}



int C_Linux_SPI::get_bits_per_word(unsigned char *bits_per_word)
{

    if( _dev_fd == -1 )
        return -1;


    if( !bits_per_word )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }


    if( ioctl(_dev_fd, SPI_IOC_RD_BITS_PER_WORD, bits_per_word) == -1 )
    {
    if( _dev_fd == -1 )
        return -1;
    }
    return 0; //good job
}



int C_Linux_SPI::get_max_speed_hz(uint32_t *max_speed_hz)
{ 

    if( _dev_fd == -1 )
        return -1;


    if( !max_speed_hz )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }


    if( ioctl(_dev_fd, SPI_IOC_RD_MAX_SPEED_HZ, max_speed_hz) == -1 )
    {
        _errno = ERROR_CANT_GET_SPEED;
        return -1;
    }


    return 0; //good job
}



int C_Linux_SPI::read(void *buf, uint32_t len)
{
    int ret;

    if( _dev_fd == -1 )
        return -1;


    if( !buf )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }


    _spi_tr.tx_buf = (uintptr_t)NULL;
    _spi_tr.rx_buf = (uintptr_t)buf;
    _spi_tr.len    = len;


    ret = ioctl(_dev_fd, SPI_IOC_MESSAGE(1), &_spi_tr);
    printf("\n\tRx ret = %d; \n", ret);
    if( ret < 1 )
    {
        _errno = ERROR_CANT_READ;
        return -1;
    }


    return ret; //good job
}


int C_Linux_SPI::set_max_speed_hz(uint32_t max_speed_hz)
{

    if( _dev_fd == -1 )
        return -1;


    if( ioctl(_dev_fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_speed_hz) == -1 )
    {
        _errno = ERROR_CANT_SET_SPEED;
        return -1;
    }

    _spi_tr.speed_hz = max_speed_hz;


    return 0; //good job
}


int C_Linux_SPI::write_SPI(const void *buf, uint32_t len)
{

    int ret;
    

// 3 bytes = 13 pulses 200nsec. speed_hz(8000000)
// 1 byte =  5 pulses 200nsec. speed_hz(8000000)
// 2 bytes = 10 pulses 200nsec. speed_hz(8000000)

// 1 byte =  5 pulses 400nsec. speed_hz(4000000)
// 2 bytes = 9 pulses 400nsec. speed_hz(4000000)


    if( _dev_fd == -1 )
        return -1;


    if( !buf )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }


 /* set the pin */
    // sprintf(buf_CS_SPI_MMS_SS, "/sys/class/gpio/gpio%d/value", gpio_CS_SPI_MMS_SS);

    // fd_CS_SPI_MMS_SS = open(buf_CS_SPI_MMS_SS, O_WRONLY);

     
    // // usleep(100);
    // /* Set GPIO low status to enable SPI slave, MMS spi */
    // if ((write(fd_CS_SPI_MMS_SS, "1", 1)) == -1)    {
    //     perror("\n\tfd for GPIO CS fail.7 Please increase the size of buf buf_CS_SPI_MMS_SS[MAX_BUF] \n");
    // }  
    

    #ifdef TEMP_TEST_SPI_TX_PULSE_TEST 
        uint8_t temp[2] = {0xaa, 0xaa};  // 0xaa, 0xaa, 0xaa, 0xaa,
        _spi_tr.tx_buf = (uintptr_t)temp;
        _spi_tr.len    = (uint8_t) sizeof(temp);
    #else  
        _spi_tr.tx_buf = (uintptr_t)buf;
        _spi_tr.len    = len;
    #endif
            
    _spi_tr.rx_buf = (uintptr_t)NULL;    
    

    ret = ioctl(_dev_fd, SPI_IOC_MESSAGE(1), &_spi_tr);
    if( ret < 1 )
    {
        _errno = ERROR_CANT_WRITE;
        return -1;
    }

    /* Set GPIO high status to disable SPI slave, MMS spi */
   // sleep(1);
    // if ((write(fd_CS_SPI_MMS_SS, "1", 1)) == -1)    {
    //     perror("\n\tfd for GPIO CS fail.6 Please increase the size of buf buf_CS_SPI_MMS_SS[MAX_BUF] \n");
    // } 
    // close(fd_CS_SPI_MMS_SS);

printf("\n\t ioctl of spi_Tx\n");
    return ret; //good job
}



int C_Linux_SPI::send_tr(spi_ioc_transfer *config_msg, uint32_t how_many_SPI_dev)  /* designed for multi SPI device */
{

    int ret;

    if( _dev_fd == -1 )
        return -1;


    if( !config_msg )
    {
        _errno = ERROR_BAD_PARAM;
        return -1;
    }

    /* set CS pin Low so mms can b eenabled */
    ret = gpio_set("0");    // enables spi slave, mms
    if (ret > 0)   {
        perror("\nError gpio_set: linux_spi.cpp \n");
    }
    printf("\n write pin successful \n");
    ret = ioctl(_dev_fd, SPI_IOC_MESSAGE(how_many_SPI_dev), config_msg);    
    printf("send_tr ret == %d\n", ret);
    if( ret < 1 )
    {
        _errno = ERROR_CANT_SEND_TR;
        return -1;
    }

    /* set CS pin High so mms can be desabled */
    ret = gpio_set("1");    // desables spi slave, mms
    if (ret > 0)   {
        perror("\nError gpio_set: linux_spi.cpp \n");
    }
    printf("\n write pin successful \n");

    return ret; // ret == 0, good job
}



const char *C_Linux_SPI::strerror(C_Linux_SPI::SPI_Error error)
{

    switch (error)
    {

        case ERROR_DEV_NOT_OPEN:
            return "device is not open";

        case ERROR_CANT_OPEN_DEV:
            return "cant open device";

        case ERROR_CANT_SET_MODE:
            return "cant set mode";

        case ERROR_CANT_GET_MODE:
            return "cant get mode";

        case ERROR_CANT_SET_BITS:
            return "cant set bits";

        case ERROR_CANT_GET_BITS:
            return "cant get bits";

        case ERROR_CANT_SET_SPEED:
            return "cant set speed";

        case ERROR_CANT_GET_SPEED:
            return "cant get speed";

        case ERROR_BAD_PARAM:
            return "bad param";

        case ERROR_CANT_READ:
            return "cant read";

        case ERROR_CANT_WRITE:
            return "cant write";

        case ERROR_CANT_SEND_TR:
            return "cant send";

        default:
            return "unknown error";
    }
}
