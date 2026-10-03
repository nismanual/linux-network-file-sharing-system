#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd;
    char buffer[256];

    fd = open("/dev/capstone_device", O_RDWR);

    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }

    printf("Device opened successfully.\n");

    const char *message = "Hello from user space!";

    if (write(fd, message, strlen(message)) < 0)
    {
        perror("Write failed");
        close(fd);
        return 1;
    }

    printf("Data written to driver: %s\n", message);

    memset(buffer, 0, sizeof(buffer));

    if (read(fd, buffer, sizeof(buffer) - 1) < 0)
    {
        perror("Read failed");
        close(fd);
        return 1;
    }

    printf("Data read from driver: %s\n", buffer);

    close(fd);

    printf("Device closed successfully.\n");

    return 0;
}
