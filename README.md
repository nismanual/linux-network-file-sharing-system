## Linux Network File Sharing System

## 1. Project Overview

The Linux Network File Sharing System is a client-server application developed in C for Linux.

The system allows clients to connect to a centralized server over TCP and perform file management operations such as:

Listing available files

Viewing file information

Searching for files

Uploading files

Downloading files

A Linux character device driver is also integrated into the system to demonstrate Linux device-driver concepts and communication between user space and kernel space.

## 2. Objectives

The main objectives of this project are:

Implement a TCP-based client-server architecture in C.

Provide reliable file upload and download functionality.

Implement file listing, searching, and information retrieval.

Apply Linux system programming concepts.

Implement a Linux character device driver.

Demonstrate communication between user-space applications and a Linux kernel module.

Integrate Linux device-driver concepts into a practical software architecture.

Maintain a structured and reproducible Linux development environment.

## 3. Technologies Used

Technology	                   Purpose
C	                           Application, networking, and driver test implementation
Linux	                           Operating system
TCP/IP	                           Client-server communication
POSIX Sockets	                   Network programming
Linux Character Device Driver      Kernel-space integration
GCC	                           C compilation
Make	                           Kernel module build system
Git/GitHub	                   Version control and project hosting
VirtualBox	                   Linux development environment


## 4. System Architecture

                 Linux Client
                      |
                      | TCP
                      |
                      v
              +---------------+
              | Linux Server  |
              +-------+-------+
                      |
             +--------+--------+
             |                 |
             v                 v
      shared_files/     Character Device
                         Driver
                            |
                            v
                    /dev/capstone_device

The client communicates with the server using TCP sockets.

The server manages files stored in the:

shared_files/
directory.

The server also communicates with the Linux character device driver through:

/dev/capstone_device
The character device driver provides a kernel-level interface that can receive and return data between user space and kernel space.

The server uses this interface to record file-transfer events.

## 5. Project Structure

capstone/
│
├── client/
│   ├── client.c
│   ├── document.txt
│   ├── downloaded_document.txt
│   └── upload_test.txt
│
├── driver/
│   ├── capstone_driver.c
│   └── Makefile
│
├── driver_test/
│   └── driver_test.c
│
├── server/
│   └── server.c
│
├── shared_files/
│   ├── document.txt
│   ├── readme.txt
│   └── upload_test.txt
│
├── docs/
├── logs/
├── .gitignore
└── README.md

## 6. Client Features

The client provides an interactive menu:

=================================
       FILE SHARING CLIENT
=================================
1. List Files
2. File Information
3. Search File
4. Download File
5. Upload File
6. Exit
=================================
LIST
Displays the files available on the server.

INFO
Displays information about a specified file on the server.

SEARCH
Searches for a specified file on the server.

DOWNLOAD
Downloads a file from the server and stores it in the client directory.

UPLOAD
Uploads a file from the client directory to the server.

EXIT
Terminates the client application.

## 7. Linux Character Device Driver

The project contains a Linux character device driver implemented in:

driver/capstone_driver.c
The driver creates the device:

/dev/capstone_device
The driver implements:

Character device registration

Device creation

Device opening

Device reading

Device writing

Device release

User-space/kernel-space data transfer

The driver uses:

copy_to_user()
and:

copy_from_user()
to safely transfer data between user space and kernel space.

The driver also generates kernel log messages for important device operations.

## 8. Driver Integration

The server communicates with the character device driver using:

/dev/capstone_device
When a file transfer is successfully completed, the server sends an event to the driver.

For example:

UPLOAD:upload_test.txt
or:

DOWNLOAD:document.txt
The server opens the device, writes the event, and closes the device.

This demonstrates interaction between:

User Space
    |
    v
TCP Server
    |
    v
/dev/capstone_device
    |
    v
Linux Character Device Driver
    |
    v
Kernel Space
Driver activity can be inspected using:

sudo dmesg | tail -15
Example output:

Capstone Driver: device opened
Capstone Driver: data written
Capstone Driver: device closed

## 9. Building the Server

Navigate to the server directory:

cd ~/capstone/server
Compile the server:

gcc server.c -o server
Run the server:

./server
The server listens on:

Port: 8080
The server binds to:

0.0.0.0

## 10. Building the Client

Navigate to the client directory:

cd ~/capstone/client
Compile the client:

gcc client.c -o client
Run the client:

./client
For the current local demonstration environment, the client connects to:

127.0.0.1:8080

## 11. Building the Linux Driver

Navigate to the driver directory:

cd ~/capstone/driver
Build the kernel module:

make
Load the module:

sudo insmod capstone_driver.ko
Verify that it is loaded:

lsmod | grep capstone_driver
Check the device:

ls -l /dev/capstone_device
If necessary for the demonstration environment, allow user-space access:

sudo chmod 666 /dev/capstone_device

## 12. Driver Testing

A separate user-space test program is provided in:

driver_test/driver_test.c
Build the test program:

cd ~/capstone/driver_test
gcc driver_test.c -o driver_test
Run it:

sudo ./driver_test
Expected output:

Device opened successfully.
Data written to driver: Hello from user space!
Data read from driver: Hello from user space!
Device closed successfully.
The test verifies:

Opening the character device.

Writing data to the driver.

Reading data from the driver.

Closing the device.

The corresponding kernel messages can be checked using:

sudo dmesg | tail -15

## 13. Testing File Upload

A test file is provided in the client directory:

client/upload_test.txt
The file contains:

This file was uploaded through the TCP file sharing system.
Its size is:

60 bytes
Start the server:

cd ~/capstone/server
./server
In another terminal, start the client:

cd ~/capstone/client
./client
Select:

 Upload File
Enter:

upload_test.txt
Expected output:

File selected: upload_test.txt
File size: 60 bytes
Connecting to server...
Connected to server.
Command sent: UPLOAD upload_test.txt 60
Uploading file...
Upload data sent successfully.
Bytes sent: 60

Server response:
UPLOAD_SUCCESS:upload_test.txt
Verify the uploaded file:

cat ~/capstone/shared_files/upload_test.txt
Expected content:

This file was uploaded through the TCP file sharing system.
The server also logs the upload event through the Linux character device driver.

## 14. Testing File Download

A test file is available on the server:

shared_files/document.txt
Start the server:

cd ~/capstone/server
./server
Start the client in another terminal:

cd ~/capstone/client
./client
Select:

Download File
Enter:

document.txt
The client downloads the file and stores it in the client directory.

Verify the downloaded file:

cat ~/capstone/client/document.txt
The downloaded file should contain the same content as the corresponding server file.

The server also records the download event through the Linux character device driver.

## 15. Testing Driver Integration

After performing a file upload or download, inspect the kernel messages:

sudo dmesg | tail -15
Example:

Capstone Driver: device opened
Capstone Driver: data written
Capstone Driver: device closed
This confirms that the server successfully interacted with the Linux character device driver.

The complete interaction is:

Client
   |
   | TCP
   v
Server
   |
   +--------------------+
   |                    |
   v                    v
File System       Character Device
                       |
                       v
              /dev/capstone_device
                       |
                       v
              Linux Kernel Driver

## 16. Security Considerations

The server validates filenames before accessing files.

The following path traversal patterns are rejected:

..
/
This prevents a client from attempting to access files outside the intended file-sharing directory.

The server also limits command length and handles invalid commands.

The current implementation is designed for a controlled Linux demonstration environment.

## 17. Error Handling

The system handles common errors including:

Socket creation failures

Connection failures

File opening failures

File-not-found conditions

Invalid filenames

Invalid commands

Invalid file sizes

Partial network writes

Incomplete file transfers

Device-driver access failures

User-space/kernel-space copy failures

Error messages are reported through standard Linux error handling mechanisms such as:

perror()
and return-value checks.

## 18. Linux Concepts Demonstrated

This project demonstrates practical Linux system programming concepts including:

System Calls
open()
read()
write()
close()
Networking
socket()
bind()
listen()
accept()
connect()
send()
recv()
File Operations
open()
read()
write()
stat()
fstat()
Kernel Module Concepts
Linux kernel modules

Character devices

Device registration

Device creation

File operations

Kernel logging

Module initialization

Module cleanup

User/Kernel Communication
copy_to_user()
copy_from_user()
Build and Development Tools
GCC
Make
Git
Linux terminal

## 19. Limitations

The current implementation is intended primarily for a controlled Linux environment and capstone demonstration.

Current limitations include:

The current demonstration uses a local TCP connection.

Authentication is not implemented.

File encryption is not implemented.

Multiple-client concurrency is limited by the current server architecture.

File permissions are not exposed through the client interface.

Transfer progress is not displayed.

The character driver currently provides a simple data buffer rather than persistent event storage.

Device permissions may need to be configured when the module is loaded.

## 20. Conclusion

The Linux Network File Sharing System demonstrates a complete client-server file-sharing workflow using C and Linux system programming.

The project combines:

TCP socket programming

File system operations

Client-server architecture

Linux system calls

Linux character device driver development

User-space/kernel-space communication

Kernel logging

Secure filename handling

The system supports file listing, file information retrieval, file searching, file uploading, and file downloading.

The integration of the Linux character device driver provides practical exposure to Linux kernel concepts while the TCP client-server system demonstrates application-level networking.

The project therefore provides a practical implementation of both software architecture and Linux device-driver concepts in a single system.

## 21. Author

Nisigandha Mallick

Linux Capstone Project — 2026

Developed using:

C
Linux
TCP/IP
POSIX Sockets
Linux Character Device Driver
GCC
Make
Git
