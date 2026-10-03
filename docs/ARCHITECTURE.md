# System Architecture

## 1. Overview

The Linux Network File Sharing System is a C-based client-server application designed to provide file sharing over a TCP connection.

The project also incorporates a Linux character device driver to demonstrate interaction between user-space applications and the Linux kernel.

The system consists of four major components:

1. TCP Client
2. TCP Server
3. Shared File Storage
4. Linux Character Device Driver

---

## 2. High-Level Architecture


                         Linux System
                              │
                ┌─────────────┴─────────────┐
                │                           │
          User Space                    Kernel Space
                │                           │
        ┌───────┴────────┐                  │
        │                │                  │
     Client            Server              │
        │                │                  │
        │      TCP       │                  │
        └───────────────►│                  │
                         │                  │
                         │ file operations │
                         ▼                  │
                   Shared Files            │
                         │                  │
                         │ driver logging   │
                         ▼                  │
                  /dev/capstone_device ◄────┘
                         │
                         ▼
                Character Device Driver

## 3. Client Architecture

The client is a user-space C application.

Its main responsibilities are:

Establishing a TCP connection with the server
Sending commands to the server
Receiving server responses
Listing available files
Searching for files
Requesting file information
Downloading files
Uploading files
Closing the connection

The client communicates with the server using TCP sockets.

The server address used during local testing is:

127.0.0.1

The server port is:

8080

## 4. Server Architecture

The server is a user-space C application responsible for handling client requests.

Its main responsibilities are:

Creating a TCP socket
Binding the socket to port 8080
Listening for client connections
Accepting client connections
Receiving commands
Processing file operations
Sending responses to the client
Receiving uploaded files
Sending requested files
Validating filenames
Communicating with the Linux character device driver

The server operates on files stored in:

~/capstone/shared_files/

## 5. TCP Communication

The client and server communicate using the TCP protocol.

The basic communication flow is:

Client
   │
   │ TCP connection
   ▼
Server
   │
   │ command
   ▼
Command Processing
   │
   ├── LIST
   ├── INFO
   ├── SEARCH
   ├── DOWNLOAD
   └── UPLOAD

TCP provides reliable and ordered delivery of data between the client and server.

Commands are terminated using a newline character so that the server can correctly identify the end of each command.

## 6. Supported Operations

LIST

The server reads the shared directory and returns the available files.

Client → LIST
Server → File list
INFO

The client requests information about a specific file.

Client → INFO <filename>
Server → File information
SEARCH

The client searches for a specific filename.

Client → SEARCH <filename>
Server → Search result
DOWNLOAD

The client requests a file from the server.

Client → DOWNLOAD <filename>
Server → File data
UPLOAD

The client sends a file to the server.

Client → UPLOAD <filename> <filesize>
Server → Receives file data
Server → Upload result

## 7. File Storage Architecture

The server stores shared files inside:

shared_files/

Example:

shared_files/
├── document.txt
├── readme.txt
└── upload_test.txt

Files uploaded by the client are stored in this directory after successful transfer.

The client maintains its own local files inside:

client/

## 8. Linux Character Device Driver

The project includes a Linux character device driver implemented in:

driver/capstone_driver.c

The driver is compiled as a Linux kernel module:

capstone_driver.ko

After loading the module, the device is available as:

/dev/capstone_device

The driver implements standard character-device operations:

open
read
write
release

## 9. Driver Data Flow

The driver maintains a small kernel-space buffer.

The data flow is:

User Space
    │
    │ write()
    ▼
/dev/capstone_device
    │
    ▼
Kernel Driver Buffer
    │
    │ read()
    ▼
User Space

The driver uses:

copy_from_user()

to safely copy data from user space into kernel space.

It uses:

copy_to_user()

to safely copy data from kernel space back to user space.

## 10. Driver Registration

During module initialization, the driver:

Registers a character device.
Creates a device class.
Creates the device node.
Makes the device available through /dev/capstone_device.

During module removal, the driver:

Destroys the device.
Destroys the device class.
Unregisters the character device.

## 11. Server and Driver Integration

The server integrates with the character device driver using:

/dev/capstone_device

After a successful file operation, the server opens the device and writes an event message.

For example:

UPLOAD:upload_test.txt

or:

DOWNLOAD:document.txt

The communication path is:

TCP Client
     │
     ▼
TCP Server
     │
     │ open()
     │ write()
     ▼
/dev/capstone_device
     │
     ▼
Character Device Driver
     │
     ▼
Linux Kernel

This integration demonstrates interaction between a normal user-space application and a Linux kernel module.


## 12. Security and File Validation

The server validates filenames before performing file operations.

The system rejects filenames containing:

..
/
\

This prevents clients from attempting to access files outside the intended shared directory using path traversal techniques.

The server also validates file transfer information such as the filename and file size.

## 13. Error Handling

The project checks for errors during important system operations.

Examples include:

Socket creation failures
Connection failures
File opening failures
File reading failures
File writing failures
Driver access failures
Invalid filenames
Invalid commands
Connection termination

When an error occurs, the application reports the problem and performs appropriate cleanup.

## 14. User Space and Kernel Space

The architecture demonstrates the distinction between user space and kernel space.

User Space

The following components execute in user space:

Client
Server
Driver Test Program
Kernel Space

The Linux character device driver executes in kernel space:

capstone_driver.ko

The device interface connects the two:

User Space
    │
    │ system calls
    ▼
/dev/capstone_device
    │
    ▼
Kernel Space

## 15. Complete System Flow

The complete system operates as follows:

1. Linux kernel loads the character device driver
                    │
                    ▼
2. /dev/capstone_device is created
                    │
                    ▼
3. Server starts and listens on TCP port 8080
                    │
                    ▼
4. Client connects to the server
                    │
                    ▼
5. Client sends a file operation
                    │
                    ▼
6. Server processes the request
                    │
             ┌──────┴──────┐
             │             │
             ▼             ▼
        File Operation   Driver Event
             │             │
             ▼             ▼
      Shared Files   /dev/capstone_device
                           │
                           ▼
                    Character Driver
                           │
                           ▼
                     Linux Kernel

## 16. Technologies Used

Component	                            Technology
Programming Language	                    C
Operating System	                    Ubuntu Linux
Networking                                  TCP/IP Sockets
File System	                            Linux File System
Kernel Component	                    Linux Character Device Driver
Kernel Module	                            .ko
Compiler	                            GCC
Build System	                            GNU Make
Version Control	                            Git

## 17. Architecture Concepts Demonstrated

The project demonstrates the following concepts:

Client-server architecture
TCP socket programming
File transfer
Linux file system operations
Linux system calls
User-space and kernel-space interaction
Character device drivers
Kernel module development
Device file management
Inter-process communication through networking
Error handling
Basic file access security
Modular software architecture

## 18. Design Summary

The system separates networking, file management, and kernel-level functionality into different components.

The client handles user interaction and requests.

The server handles network communication and file operations.

The shared directory provides centralized file storage.

The Linux character device driver provides the kernel-level component required to demonstrate Linux device-driver concepts.

This modular structure makes the project easier to understand, test, maintain, and demonstrate.


Then save:

**Ctrl + O → Enter → Ctrl + X**

After saving, run: cat docs/ARCHITECTURE.md
