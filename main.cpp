// main.cpp Multithreading SPI
// c++11
// g++ -std=c++11 -lpthread

#include <stdio.h>
#include "inc/linux_spi.h"

#define LOOP_FOR_GOOD 1U

__attribute__((noinline)) int exec();

static C_Linux_SPI              spi_dev1;

     
__attribute__((noinline)) C_Linux_SPI *getSPI_dev1(void)   { return (&spi_dev1);     }


__attribute__((noinline)) int main()
{
     init_spi_dev(&spi_dev1);
     printf("\ninit spi\n");
     exec();
     printf("\n exec done");

   


    // while (LOOP_FOR_GOOD)
    // {
    //     printf("\n while 1 loop");
        
    // }  
     spi_dev1.dev_close();  
     printf("\n main EXIT_SUCCESS \n\n ");
     
     return 0;  // EXIT_SUCCESS;
     
}