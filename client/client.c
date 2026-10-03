#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

/* Send the complete buffer */
static int send_all(int socket_fd, const char *buffer, size_t length)
{
    size_t total_sent = 0;

    while (total_sent < length)
    {
        ssize_t bytes_sent = send(
            socket_fd,
            buffer + total_sent,
            length - total_sent,
            0
        );

        if (bytes_sent <= 0)
            return -1;

        total_sent += bytes_sent;
    }

    return 0;
}

/* Receive data until the server closes the connection */
static void receive_response(int socket_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(
                socket_fd,
                buffer,
                sizeof(buffer) - 1,
                0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0)
        perror("recv");
}

/* LIST command */
static void list_files(void)
{
    int socket_fd;
    struct sockaddr_in server_address;
    const char *command = "LIST\n";

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        return;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(
            AF_INET,
            SERVER_IP,
            &server_address.sin_addr) <= 0)
    {
        perror("Invalid server address");
        close(socket_fd);
        return;
    }

    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(socket_fd);
        return;
    }

    send_all(socket_fd, command, strlen(command));

    printf("\nFiles available on server:\n");
    printf("---------------------------------\n");

    receive_response(socket_fd);

    close(socket_fd);
}

/* INFO command */
static void file_info(void)
{
    char filename[256];
    char command[512];

    printf("Enter filename: ");
    scanf("%255s", filename);

    snprintf(
        command,
        sizeof(command),
        "INFO %s\n",
        filename
    );

    int socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        SERVER_IP,
        &server_address.sin_addr
    );

    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(socket_fd);
        return;
    }

    send_all(socket_fd, command, strlen(command));

    printf("\nFile information:\n");
    printf("---------------------------------\n");

    receive_response(socket_fd);

    close(socket_fd);
}

/* SEARCH command */
static void search_file(void)
{
    char filename[256];
    char command[512];

    printf("Enter filename to search: ");
    scanf("%255s", filename);

    snprintf(
        command,
        sizeof(command),
        "SEARCH %s\n",
        filename
    );

    int socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        SERVER_IP,
        &server_address.sin_addr
    );

    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(socket_fd);
        return;
    }

    send_all(socket_fd, command, strlen(command));

    printf("\nSearch result:\n");
    printf("---------------------------------\n");

    receive_response(socket_fd);

    close(socket_fd);
}

/* DOWNLOAD command */
static void download_file(void)
{
    char filename[256];
    char command[512];

    printf("Enter filename to download: ");
    scanf("%255s", filename);

    snprintf(
        command,
        sizeof(command),
        "DOWNLOAD %s\n",
        filename
    );

    int socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        SERVER_IP,
        &server_address.sin_addr
    );

    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(socket_fd);
        return;
    }

    send_all(socket_fd, command, strlen(command));

    char buffer[BUFFER_SIZE];

    ssize_t bytes_received = recv(
        socket_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytes_received <= 0)
    {
        printf("No response from server.\n");
        close(socket_fd);
        return;
    }

    buffer[bytes_received] = '\0';

    if (strncmp(
            buffer,
            "ERROR",
            5) == 0)
    {
        printf("Server response: %s", buffer);
        close(socket_fd);
        return;
    }

    long file_size;

    if (sscanf(
            buffer,
            "FILESIZE:%ld\n",
            &file_size) != 1)
    {
        printf("Unexpected server response: %s\n", buffer);
        close(socket_fd);
        return;
    }

    FILE *file = fopen(
        filename,
        "wb"
    );

    if (file == NULL)
    {
        perror("Unable to create local file");
        close(socket_fd);
        return;
    }

    long total_received = 0;

    while (total_received < file_size)
    {
        bytes_received = recv(
            socket_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytes_received <= 0)
            break;

        fwrite(
            buffer,
            1,
            bytes_received,
            file
        );

        total_received += bytes_received;
    }

    fclose(file);
    close(socket_fd);

    if (total_received == file_size)
    {
        printf(
            "\nDownload completed successfully.\n"
        );

        printf(
            "File saved as: %s\n",
            filename
        );

        printf(
            "Bytes received: %ld\n",
            total_received
        );
    }
    else
    {
        printf(
            "\nDownload incomplete.\n"
        );

        printf(
            "Expected: %ld bytes\n",
            file_size
        );

        printf(
            "Received: %ld bytes\n",
            total_received
        );
    }
}

/* UPLOAD command */
static void upload_file(void)
{
    char filename[256];
    char filepath[512];
    struct stat file_info;

    printf("Enter filename from client directory: ");
    scanf("%255s", filename);

    snprintf(
        filepath,
        sizeof(filepath),
        "%s",
        filename
    );

    int file_fd = open(
        filepath,
        O_RDONLY
    );

    if (file_fd < 0)
    {
        perror("Unable to open file");
        return;
    }

    if (fstat(file_fd, &file_info) < 0)
    {
        perror("Unable to get file information");
        close(file_fd);
        return;
    }

    long file_size = file_info.st_size;

    printf(
        "File selected: %s\n",
        filename
    );

    printf(
        "File size: %ld bytes\n",
        file_size
    );

    int socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        close(file_fd);
        return;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        SERVER_IP,
        &server_address.sin_addr
    );

    printf("Connecting to server...\n");

    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(file_fd);
        close(socket_fd);
        return;
    }

    printf("Connected to server.\n");

    char command[512];

    snprintf(
        command,
        sizeof(command),
        "UPLOAD %s %ld\n",
        filename,
        file_size
    );

    send_all(
        socket_fd,
        command,
        strlen(command)
    );

    printf(
        "Command sent: %s",
        command
    );

    printf("Uploading file...\n");

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    long total_sent = 0;

    while ((bytes_read = read(
                file_fd,
                buffer,
                sizeof(buffer))) > 0)
    {
        if (send_all(
                socket_fd,
                buffer,
                bytes_read) < 0)
        {
            perror("Upload failed");
            close(file_fd);
            close(socket_fd);
            return;
        }

        total_sent += bytes_read;
    }

    close(file_fd);

    printf(
        "Upload data sent successfully.\n"
    );

    printf(
        "Bytes sent: %ld\n",
        total_sent
    );

    printf("\nServer response:\n");

    receive_response(socket_fd);

    close(socket_fd);

    printf("\nClient stopped.\n");
}

/* Main menu */
int main(void)
{
    int choice;

    while (1)
    {
        printf("\n");
        printf("=================================\n");
        printf("       FILE SHARING CLIENT       \n");
        printf("=================================\n");
        printf("1. List Files\n");
        printf("2. File Information\n");
        printf("3. Search File\n");
        printf("4. Download File\n");
        printf("5. Upload File\n");
        printf("6. Exit\n");
        printf("=================================\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");

            while (getchar() != '\n')
                ;

            continue;
        }

        switch (choice)
        {
            case 1:
                list_files();
                break;

            case 2:
                file_info();
                break;

            case 3:
                search_file();
                break;

            case 4:
                download_file();
                break;

            case 5:
                upload_file();
                break;

            case 6:
                printf("Exiting client.\n");
                return 0;

            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}
