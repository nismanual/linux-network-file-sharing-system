#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define BACKLOG 5

#define SHARED_DIR "../shared_files"
#define DRIVER_DEVICE "/dev/capstone_device"


/* =========================================================
   WRITE EVENT TO LINUX CHARACTER DEVICE DRIVER
   ========================================================= */

static void log_to_driver(const char *event)
{
    int driver_fd;

    driver_fd = open(
        DRIVER_DEVICE,
        O_WRONLY
    );

    if (driver_fd < 0)
    {
        perror("Unable to open capstone driver");

        return;
    }

    if (write(
            driver_fd,
            event,
            strlen(event)) < 0)
    {
        perror("Unable to write to capstone driver");
    }

    close(driver_fd);
}


/* =========================================================
   LIST FILES
   ========================================================= */

static void send_file_list(int client_fd)
{
    DIR *directory;
    struct dirent *entry;

    char response[BUFFER_SIZE];
    size_t used = 0;

    directory = opendir(SHARED_DIR);

    if (directory == NULL)
    {
        const char *error =
            "ERROR: Unable to open shared directory.\n";

        write(client_fd, error, strlen(error));

        return;
    }

    used += snprintf(
        response + used,
        sizeof(response) - used,
        "Files available:\n"
    );

    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (used < sizeof(response) - 1)
        {
            used += snprintf(
                response + used,
                sizeof(response) - used,
                "- %s\n",
                entry->d_name
            );
        }
    }

    closedir(directory);

    write(
        client_fd,
        response,
        used
    );
}


/* =========================================================
   FILE INFORMATION
   ========================================================= */

static void send_file_info(
    int client_fd,
    const char *filename)
{
    char filepath[BUFFER_SIZE];

    struct stat file_stat;

    char response[BUFFER_SIZE];

    snprintf(
        filepath,
        sizeof(filepath),
        "%s/%s",
        SHARED_DIR,
        filename
    );

    if (stat(filepath, &file_stat) < 0)
    {
        snprintf(
            response,
            sizeof(response),
            "ERROR: File '%s' not found.\n",
            filename
        );

        write(
            client_fd,
            response,
            strlen(response)
        );

        return;
    }

    if (!S_ISREG(file_stat.st_mode))
    {
        const char *error =
            "ERROR: Requested item is not a regular file.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }

    char time_buffer[64];

    struct tm *time_info =
        localtime(&file_stat.st_mtime);

    strftime(
        time_buffer,
        sizeof(time_buffer),
        "%Y-%m-%d %H:%M:%S",
        time_info
    );

    snprintf(
        response,
        sizeof(response),

        "File Information\n"
        "-------------------------\n"
        "Name       : %s\n"
        "Size       : %ld bytes\n"
        "Permissions: %o\n"
        "Owner UID  : %d\n"
        "Group GID  : %d\n"
        "Modified   : %s\n",

        filename,
        (long)file_stat.st_size,
        file_stat.st_mode & 0777,
        file_stat.st_uid,
        file_stat.st_gid,
        time_buffer
    );

    write(
        client_fd,
        response,
        strlen(response)
    );
}


/* =========================================================
   SEARCH FILE
   ========================================================= */

static void search_file(
    int client_fd,
    const char *filename)
{
    DIR *directory;
    struct dirent *entry;

    char response[BUFFER_SIZE];

    directory = opendir(SHARED_DIR);

    if (directory == NULL)
    {
        const char *error =
            "ERROR: Unable to open shared directory.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (strcmp(entry->d_name, filename) == 0)
        {
            snprintf(
                response,
                sizeof(response),
                "File found: %s\n",
                filename
            );

            write(
                client_fd,
                response,
                strlen(response)
            );

            closedir(directory);

            return;
        }
    }

    closedir(directory);

    snprintf(
        response,
        sizeof(response),
        "File not found: %s\n",
        filename
    );

    write(
        client_fd,
        response,
        strlen(response)
    );
}


/* =========================================================
   DOWNLOAD FILE
   ========================================================= */

static void send_file(
    int client_fd,
    const char *filename)
{
    char filepath[BUFFER_SIZE];

    char buffer[BUFFER_SIZE];

    int file_fd;

    ssize_t bytes_read;

    struct stat file_stat;

    char header[256];


    /* -----------------------------------------------------
       Filename validation
       ----------------------------------------------------- */

    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\') != NULL)
    {
        const char *error =
            "ERROR: Invalid filename.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    snprintf(
        filepath,
        sizeof(filepath),
        "%s/%s",
        SHARED_DIR,
        filename
    );


    if (stat(filepath, &file_stat) < 0)
    {
        const char *error =
            "ERROR: File not found.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    if (!S_ISREG(file_stat.st_mode))
    {
        const char *error =
            "ERROR: Requested item is not a regular file.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    file_fd = open(
        filepath,
        O_RDONLY
    );

    if (file_fd < 0)
    {
        const char *error =
            "ERROR: Unable to open file.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    /* -----------------------------------------------------
       Send file size header
       ----------------------------------------------------- */

    snprintf(
        header,
        sizeof(header),
        "FILESIZE:%ld\n",
        (long)file_stat.st_size
    );


    if (write(
            client_fd,
            header,
            strlen(header)) < 0)
    {
        perror("write");

        close(file_fd);

        return;
    }


    printf(
        "Sending file: %s\n",
        filename
    );

    printf(
        "File size: %ld bytes\n",
        (long)file_stat.st_size
    );


    /* -----------------------------------------------------
       Send file data
       ----------------------------------------------------- */

    while ((bytes_read = read(
                file_fd,
                buffer,
                sizeof(buffer))) > 0)
    {
        ssize_t total_sent = 0;

        while (total_sent < bytes_read)
        {
            ssize_t bytes_sent =
                write(
                    client_fd,
                    buffer + total_sent,
                    bytes_read - total_sent
                );

            if (bytes_sent < 0)
            {
                perror("write");

                close(file_fd);

                return;
            }

            total_sent += bytes_sent;
        }
    }


    if (bytes_read < 0)
    {
        perror("read");
    }


    close(file_fd);


    /* -----------------------------------------------------
       Log download to Linux driver
       ----------------------------------------------------- */

    char driver_event[512];

    snprintf(
        driver_event,
        sizeof(driver_event),
        "DOWNLOAD:%s",
        filename
    );

    log_to_driver(driver_event);


    printf(
        "File transfer completed.\n"
    );

    printf(
        "Driver event logged: %s\n",
        driver_event
    );
}


/* =========================================================
   UPLOAD FILE
   ========================================================= */

static void receive_file(
    int client_fd,
    const char *filename,
    long file_size)
{
    char filepath[BUFFER_SIZE];

    char buffer[BUFFER_SIZE];

    int file_fd;

    long total_received = 0;

    ssize_t bytes_received;


    /* -----------------------------------------------------
       Filename validation
       ----------------------------------------------------- */

    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\') != NULL)
    {
        const char *error =
            "ERROR: Invalid filename.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    snprintf(
        filepath,
        sizeof(filepath),
        "%s/%s",
        SHARED_DIR,
        filename
    );


    file_fd = open(
        filepath,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (file_fd < 0)
    {
        const char *error =
            "ERROR: Unable to create destination file.\n";

        write(
            client_fd,
            error,
            strlen(error)
        );

        return;
    }


    printf(
        "Receiving file: %s\n",
        filename
    );

    printf(
        "Expected size: %ld bytes\n",
        file_size
    );


    /* -----------------------------------------------------
       Receive exact number of file bytes
       ----------------------------------------------------- */

    while (total_received < file_size)
    {
        long remaining =
            file_size - total_received;

        size_t amount =
            BUFFER_SIZE;

        if (remaining < (long)amount)
        {
            amount =
                (size_t)remaining;
        }


        bytes_received = read(
            client_fd,
            buffer,
            amount
        );


        if (bytes_received < 0)
        {
            perror("read");

            close(file_fd);

            return;
        }


        if (bytes_received == 0)
        {
            printf(
                "Connection closed before upload completed.\n"
            );

            close(file_fd);

            return;
        }


        /* -------------------------------------------------
           Handle partial writes to disk
           ------------------------------------------------- */

        ssize_t total_written = 0;

        while (total_written < bytes_received)
        {
            ssize_t bytes_written =
                write(
                    file_fd,
                    buffer + total_written,
                    bytes_received - total_written
                );

            if (bytes_written < 0)
            {
                perror("write");

                close(file_fd);

                return;
            }

            total_written +=
                bytes_written;
        }


        total_received +=
            bytes_received;
    }


    close(file_fd);


    /* -----------------------------------------------------
       Log upload to Linux driver
       ----------------------------------------------------- */

    char driver_event[512];

    snprintf(
        driver_event,
        sizeof(driver_event),
        "UPLOAD:%s",
        filename
    );

    log_to_driver(driver_event);


    printf(
        "Upload completed successfully.\n"
    );

    printf(
        "Bytes received: %ld\n",
        total_received
    );

    printf(
        "Driver event logged: %s\n",
        driver_event
    );


    /* -----------------------------------------------------
       Send success response
       ----------------------------------------------------- */

    char response[256];

    snprintf(
        response,
        sizeof(response),
        "UPLOAD_SUCCESS:%s\n",
        filename
    );


    write(
        client_fd,
        response,
        strlen(response)
    );
}


/* =========================================================
   MAIN SERVER
   ========================================================= */

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    char buffer[BUFFER_SIZE];


    /* -----------------------------------------------------
       Create socket
       ----------------------------------------------------- */

    server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd < 0)
    {
        perror("socket");

        return 1;
    }


    printf(
        "Server socket created.\n"
    );


    /* -----------------------------------------------------
       Allow address reuse
       ----------------------------------------------------- */

    int opt = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)) < 0)
    {
        perror("setsockopt");

        close(server_fd);

        return 1;
    }


    /* -----------------------------------------------------
       Configure server address
       ----------------------------------------------------- */

    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );


    server_addr.sin_family =
        AF_INET;

    server_addr.sin_addr.s_addr =
        INADDR_ANY;

    server_addr.sin_port =
        htons(PORT);


    /* -----------------------------------------------------
       Bind
       ----------------------------------------------------- */

    if (bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("bind");

        close(server_fd);

        return 1;
    }


    printf(
        "Server bound to port %d.\n",
        PORT
    );


    /* -----------------------------------------------------
       Listen
       ----------------------------------------------------- */

    if (listen(
            server_fd,
            BACKLOG) < 0)
    {
        perror("listen");

        close(server_fd);

        return 1;
    }


    printf(
        "Server listening...\n"
    );


    /* -----------------------------------------------------
       Client loop
       ----------------------------------------------------- */

    while (1)
    {
        printf(
            "\nWaiting for a client...\n"
        );


        client_len =
            sizeof(client_addr);


        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len
        );


        if (client_fd < 0)
        {
            perror("accept");

            continue;
        }


        printf(
            "Client connected.\n"
        );


        /* -------------------------------------------------
           Read command one byte at a time.

           This is important because TCP is a byte stream.
           We must not accidentally consume upload data
           while reading the command.
           ------------------------------------------------- */

        memset(
            buffer,
            0,
            sizeof(buffer)
        );


        int command_position = 0;


        while (command_position <
               BUFFER_SIZE - 1)
        {
            char command_byte;


            ssize_t bytes_received =
                read(
                    client_fd,
                    &command_byte,
                    1
                );


            if (bytes_received < 0)
            {
                perror("read");

                close(client_fd);

                break;
            }


            if (bytes_received == 0)
            {
                printf(
                    "Client closed the connection.\n"
                );

                close(client_fd);

                break;
            }


            if (command_byte == '\n')
            {
                break;
            }


            if (command_byte == '\r')
            {
                continue;
            }


            buffer[command_position] =
                command_byte;


            command_position++;
        }


        if (command_position ==
            BUFFER_SIZE - 1)
        {
            const char *error =
                "ERROR: Command too long.\n";


            write(
                client_fd,
                error,
                strlen(error)
            );


            close(client_fd);

            continue;
        }


        buffer[command_position] =
            '\0';


        printf(
            "Command received: %s\n",
            buffer
        );


        /* =================================================
           LIST
           ================================================= */

        if (strcmp(
                buffer,
                "LIST") == 0)
        {
            send_file_list(
                client_fd
            );
        }


        /* =================================================
           INFO
           ================================================= */

        else if (strncmp(
                     buffer,
                     "INFO ",
                     5) == 0)
        {
            const char *filename =
                buffer + 5;


            send_file_info(
                client_fd,
                filename
            );
        }


        /* =================================================
           SEARCH
           ================================================= */

        else if (strncmp(
                     buffer,
                     "SEARCH ",
                     7) == 0)
        {
            const char *filename =
                buffer + 7;


            search_file(
                client_fd,
                filename
            );
        }


        /* =================================================
           DOWNLOAD
           ================================================= */

        else if (strncmp(
                     buffer,
                     "DOWNLOAD ",
                     9) == 0)
        {
            const char *filename =
                buffer + 9;


            send_file(
                client_fd,
                filename
            );
        }


        /* =================================================
           UPLOAD
           ================================================= */

        else if (strncmp(
                     buffer,
                     "UPLOAD ",
                     7) == 0)
        {
            char filename[256];

            long file_size;


            if (sscanf(
                    buffer + 7,
                    "%255s %ld",
                    filename,
                    &file_size) != 2)
            {
                const char *error =
                    "ERROR: Invalid UPLOAD command.\n";


                write(
                    client_fd,
                    error,
                    strlen(error)
                );
            }


            else if (file_size < 0)
            {
                const char *error =
                    "ERROR: Invalid file size.\n";


                write(
                    client_fd,
                    error,
                    strlen(error)
                );
            }


            else
            {
                receive_file(
                    client_fd,
                    filename,
                    file_size
                );
            }
        }


        /* =================================================
           UNKNOWN COMMAND
           ================================================= */

        else
        {
            const char *response =
                "ERROR: Unknown command.\n";


            write(
                client_fd,
                response,
                strlen(response)
            );
        }


        close(client_fd);


        printf(
            "Client disconnected.\n"
        );
    }


    close(server_fd);

    return 0;
}
