#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

#include "inc/spi_CS_pin.h"

static enum MODE {
    INFO,
    EXPORT,
    UNEXPORT,
    SETDIR,
    SETINT,
    READ,
    SETVAL,
    INVALID = -1,
} mode = INVALID;

static char param[32] = "out";
static int dev = GPIO_PORT_4_SPI_SS_MMS, pin = SS_PIN_SPI_MMS;
static int gpionum = IMX_GPIO_NR(GPIO_PORT_4_SPI_SS_MMS, SS_PIN_SPI_MMS);

 int file_exists(char* path) {
    return (access(path, F_OK) == 0);
}

 int is_exported() {
    char tmpstr[MAX_BUF] = {0};
    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/value", gpionum);
    return file_exists(tmpstr);
}

 int gpio_export() {
    char tmpstr[MAX_BUF] = {0};
    
    if (is_exported()) {
        fprintf(stderr, "Error GPIO%d[%d] is already exported\n", dev, pin);
        return 1;
    }

    int fd = open("/sys/class/gpio/export", O_SYNC | O_WRONLY);
    if (fd < 0) {
        perror("Error gpio_export: open");
        return 1;
    }
    
    snprintf(tmpstr, MAX_BUF, "%d", gpionum);
    ssize_t r = write(fd, tmpstr, strlen(tmpstr));
    if (r < 0) {
        perror("Error gpio_export: write");
        return 1;
    }

    close(fd);

    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d", gpionum);
    if (!file_exists(tmpstr)) {
        fprintf(stderr, "Error. GPIO did not export successfully!\n");
        return 1;
    }

    printf("OK. Exported GPIO%d[%d] successfully: /sys/class/gpio/gpio%d\n", dev, pin, gpionum);

    return 0;
}

 int gpio_unexport() {
    char tmpstr[MAX_BUF] = {0};
    
    if (!is_exported()) {
        fprintf(stderr, "GPIO%d[%d] not exported\n", dev, pin);
        return 1;
    }

    int fd = open("/sys/class/gpio/unexport", O_SYNC | O_WRONLY);
    if (fd < 0) {
        perror("gpio_unexport: open");
        return 1;
    }

    snprintf(tmpstr, MAX_BUF, "%d", gpionum);
    ssize_t r = write(fd, tmpstr, strlen(tmpstr));
    if (r < 0) {
        perror("gpio_unexport: write");
        return 1;
    }

    close(fd);

    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/value", gpionum);
    if (file_exists(tmpstr)) {
        fprintf(stderr, "GPIO was not unexported correctly!\n");
        return 1;
    }

    printf("Unexported GPIO%d[%d] successfully\n", dev, pin);

    return 0;
}

 int gpio_dir() {
    if (strcmp(param, "in") != 0 && strcmp(param, "out") != 0) {
        fprintf(stderr, "Error.  -d: invalid argument: %s\n", param);
        return 1;
    }

    if (!is_exported()) {
        fprintf(stderr, "Error.  GPIO%d[%d] is not exported\n", dev, pin);
        return 1;
    }

    char tmpstr[MAX_BUF] = {0};

    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/direction", gpionum);
    int fd = open(tmpstr, O_SYNC | O_WRONLY);
    if (fd < 0) {
        perror("Error. gpio_dir: open");
        return 1;
    }

    ssize_t r = write(fd, param, strlen(param));  /* setting direction over here */
    if (r < 0) {
        perror("Error. gpio_dir: write");
        return 1;
    }

    close(fd);

    printf("OK. Set GPIO%d[%d] direction to %s\n", dev, pin, param);

    return 0;
}

 int gpio_int_edge() {
    if (strcmp(param, "rising") != 0
            && strcmp(param, "falling") != 0
            && strcmp(param, "both") != 0
            && strcmp(param, "none") != 0) {
        fprintf(stderr, "Error. -i: invalid argument: %s\n", param);
        return 1;
    }

    if (!is_exported()) {
        fprintf(stderr, "Error.  GPIO%d[%d] is not exported\n", dev, pin);
        return 1;
    }

    char tmpstr[MAX_BUF] = {0};

    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/edge", gpionum);
    int fd = open(tmpstr, O_SYNC | O_WRONLY);
    if (fd < 0) {
        perror("Error.  gpio_int: open");
        return 1;
    }

    ssize_t r = write(fd, param, strlen(param));
    if (r < 0) {
        perror("Error.  gpio_int: write");
        return 1;
    }

    close(fd);

    printf("Set GPIO%d[%d] edge to %s\n", dev, pin, param);

    return 0;
}

 int gpio_read() {
    int out = 0;

    if (!is_exported()) {
        fprintf(stderr, "GPIO%d[%d] is not exported\n", dev, pin);
        return 1;
    }

    char val = 0;
    char tmpstr[MAX_BUF] = {0};

    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/value", gpionum);
    int fd = open(tmpstr, O_RDONLY);
    if (fd < 0) {
        perror("gpio_read: open");
        return 1;
    }

    ssize_t r = read(fd, &val, 1);
    if (r < 0) {
        perror("gpio_read: read");
        return 1;
    }

    close(fd);

    printf("GPIO%d[%d]'s value: %c\n", dev, pin, val);
    out = (int) (val - 48);  // converts char to int
    return out;
}

 int gpio_set(const char *str) {       /* str = "1" or "0"  */
    if (strlen(str) > 1 || strchr("01", *str) == NULL) {
        fprintf(stderr, "Error. -s: invalid argument: %s\n", str);
        return 1;
    }

    if (!is_exported()) {
        fprintf(stderr, "Error. GPIO%d[%d] is not exported\n", dev, pin);
        return 1;
    }

    char tmpstr[MAX_BUF] = {0};
    snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/value", gpionum);
    int fd = open(tmpstr, O_SYNC | O_WRONLY);
    if (fd < 1) {
        perror("gpio_set: open");
        return 1;
    }

    ssize_t r = write(fd, str, 1);      // sizeof(str) is 1
    if (r < 0) {
        perror("gpio_set: write");
        return 1;
    }

    close(fd);

    printf("GPIO%d[%d]'s value set to %s\n", dev, pin, str);

    return 0;
}

 int gpio_info() {
    if (!is_exported()) {
        fprintf(stderr, "GPIO%d[%d] is not exported\n", dev, pin);
        return 1;
    }

    const char* files[6] = {
        "active_low",
        "direction",
        "edge",
        "label",
        "uevent",
        "value",
    };

    char tmpstr[MAX_BUF] = {0};
    int fd;
    ssize_t r;
    for (int i = 0; i < 6; i++) {
        snprintf(tmpstr, MAX_BUF, "/sys/class/gpio/gpio%d/%s", gpionum, files[i]);
        fd = open(tmpstr, O_RDONLY);
        if (fd < 0) {
            perror("gpio_info: open");
            return 1;
        }

        memset(tmpstr, 0, MAX_BUF);
        r = read(fd, tmpstr, MAX_BUF - 1);
        if (r < 0) {
            perror("gpio_info: read");
            return 1;
        }

        if (tmpstr[strlen(tmpstr)-1] == '\n') {
            tmpstr[strlen(tmpstr)-1] = 0;
        }
        
        printf("GPIO%d[%d].%s: %s\n", dev, pin, files[i], tmpstr);

        close(fd);
    }

    return 0;
}

 void print_usage(const char* argv0) {
    printf("Usage: %s [-eudirsI] dev pin\n", argv0);
    puts("  -e --export     export a GPIO device\n"
         "  -u --unexport   unexport a GPIO device\n"
         "  -d --dir        set the direction for a GPIO device\n"
         "                  (either 'in' or 'out')\n"
         "  -i --int        set interrupts for a GPIO device\n"
         "                  (one of 'rising', 'falling', 'none', 'both')\n"
         "  -r --read       read the current value of a GPIO device\n"
         "  -s --set        set the current value of a GPIO device\n"
         "                  (either 1 or 0)\n"
         "  -I --info       get information about a GPIO device\n");
    exit(1);
}

 void parse_opts(int argc, char** argv) {
    static const struct option lopts[] = {
        { "export",     0, 0, 'e' },
        { "unexport",   0, 0, 'u' },
        { "dir",        1, 0, 'd' },
        { "int",        1, 0, 'i' },
        { "read",       0, 0, 'r' },
        { "set",        1, 0, 's' },
        { "info",       0, 0, 'I' },
        { NULL, 0, 0, 0 },
    };
    int c;
    while (-1 != (c = getopt_long(argc, argv, "eud:i:rs:I", lopts, NULL))) {
        if (mode != INVALID) {
            print_usage(argv[0]);
        }
        switch(c) {
            case 'e':
                mode = EXPORT;
                break;
            case 'u':
                mode = UNEXPORT;
                break;
            case 'd':
                mode = SETDIR;
                strncpy(param, optarg, 31);
                break;
            case 'i':
                mode = SETINT;
                strncpy(param, optarg, 31);
                break;
            case 'r':
                mode = READ;
                break;
            case 's':
                mode = SETVAL;
                strncpy(param, optarg, 31);
                break;
            case 'I':
                mode = INFO;
                break;
            default:
                print_usage(argv[0]);
        }
    }

    if (optind + 1 >= argc) {
        print_usage(argv[0]);
    }
    
    dev = atoi(argv[optind]);
    pin = atoi(argv[optind+1]);
    if (dev < 0 || pin < 0 || pin > 31) {
        fprintf(stderr, "Invalid GPIO device: %d %d\n", dev, pin);
        exit(1);
    }
    gpionum = 32 * dev + pin;
}

// int main(int argc, char** argv) {
//     parse_opts(argc, argv);

//     switch(mode) {
//         case EXPORT:
//             return gpio_export();
//             break;
//         case UNEXPORT:
//             return gpio_unexport();
//             break;
//         case SETDIR:
//             return gpio_dir();
//             break;
//         case SETINT:
//             return gpio_int();
//             break;
//         case READ:
//             return gpio_read();
//             break;
//         case SETVAL:
//             return gpio_set();
//             break;
//         case INFO:
//             return gpio_info();
//             break;
//         case INVALID:
//         default:
//             print_usage(argv[0]);
//     }

//     return 0;
// }
