/*
 * trial.c
 *
 *  Created on: Feb 4, 2016
 *      Author: ChrisG
 */

#define EV_COMPAT3 0

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/serial.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#include <ev.h>

ev_io iow;
ev_io ior;

uint8_t readBuffer[200];
uint8_t writeBuffer[10];

void somecallbackwrite(ev_loop *loop, ev_io *iowatch, int revents)
{
    char buffer[10];
    char c;
    int i;
    int count;

    //usleep(50000);
    memset(&writeBuffer, 0, sizeof(writeBuffer));

    // Some command
    writeBuffer[0] = 0x55;
    writeBuffer[1] = 0xaa;
    writeBuffer[2] = 0x08;
    writeBuffer[3] = 0x02;
    writeBuffer[4] = 0x01;
    writeBuffer[5] = 0x00;
    writeBuffer[6] = 0x6B;
    writeBuffer[7] = 0x07;

    printf("prior to write\n");
    i = write(iowatch->fd, &writeBuffer, 8);
    printf(" wrote(%d) = %d\n", iowatch->fd, i);
    printf("after write\n");

    ev_io_stop(loop, &iow);
    printf(" ev_io_stop(%p, %p) - write watcher\n", loop, &iow);
    ev_io_start(loop, &ior);
    printf(" ev_io_start(%p, %p) - read watcher\n", loop, &ior);

}

void somecallbackread(ev_loop *loop, ev_io *iowatch, int revents)
{

    uint8_t c;
    int i = 1;
    int count = 0;

    //usleep(50000);

    printf("prior to read\n");
    while (i > 0) {

        i = read(iowatch->fd, &readBuffer, 40);
        if (i > 0) {
            count += i;
            //printf("%02X ", (uint8_t)c);
        } else {
            if (i == EAGAIN) {
                printf("EAGAIN\n");
                return;
            }
        }
    }
    printf("\nread(%d) = %d\n", iowatch->fd, count);
    printf("after read\n");

    ev_io_stop(loop, &ior);
    printf(" ev_io_stop(%p, %p) - read watcher\n", loop, &ior);
    ev_io_start(loop, &iow);
    printf(" ev_io_start(%p, %p) - write watcher\n", loop, &iow);
}

main()
{
    ev_loop *mainloop;
    int fd;

    // Open file
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    printf("0x%d\n", fd);
    if (fd < 0) {
        printf("%s\n", strerror(errno));
        assert(0);
    }

    // Set Non blocking
    int flags = fcntl(fd, F_GETFL, 0);
    printf("0x%d\n", flags);
    if (flags < 0) {
        printf("%s\n", strerror(errno));
        assert(0);
    }
    flags = flags | O_NONBLOCK;
    flags = fcntl(fd, F_SETFL, flags);
    if (flags != 0)
        assert(0);
    printf("0x%d\n", flags);

    if (fd >= 0) {
        struct termios TermIOInterface;
        struct serial_rs485 CtrlRS485;

        // Get serial Attr
        tcgetattr(fd, &TermIOInterface);

        TermIOInterface.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
                | IGNCR | ICRNL | IXON);
        TermIOInterface.c_oflag &= ~OPOST;
        TermIOInterface.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        TermIOInterface.c_cflag &= ~(CSIZE | PARENB);
        TermIOInterface.c_cflag |= CS8;

        // Set the baud rate
        cfsetospeed(&TermIOInterface, B115200);
        cfsetispeed(&TermIOInterface, B115200);

        tcsetattr(fd, TCSANOW, &TermIOInterface);

        mainloop = ev_default_loop(EVFLAG_AUTO);
        ev_io_init(&iow, somecallbackwrite, fd, EV_WRITE);
        ev_io_init(&ior, somecallbackread, fd, EV_READ);
        ev_io_start(mainloop, &iow);
        ev_io_start(mainloop, &ior);
        ev_run(mainloop, 0);
        close(fd);
    } else {
        printf("errno = %d\n", errno);
    }
}
