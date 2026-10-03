# Execution Guide

## 1. System Requirements

The project requires:

- Ubuntu Linux
- GCC compiler
- GNU Make
- Linux kernel headers
- Git
- Root/sudo access for loading the Linux kernel module

Check the installed tools:

gcc --version
make --version
uname -r

## 2. Project Directory

Move to the project directory:

cd ~/capstone

The project is organized into separate components:

capstone/
├── client/
├── server/
├── driver/
├── driver_test/
├── docs/
├── shared_files/
├── .gitignore
└── README.md

client/ — TCP client application
server/ — TCP server application
driver/ — Linux character device driver
driver_test/ — Driver testing program
docs/ — Project documentation
shared_files/ — Files shared by the server
README.md — Main project documentation

## 3. Build the Linux Character Device Driver

Navigate to the driver directory:

cd ~/capstone/driver

Build the kernel module: make

A successful build creates the kernel module: capstone_driver.ko

The driver is built using the Linux kernel build system through the provided Makefile.

## 4. Load the Linux Character Device Driver

Load the kernel module:

sudo insmod capstone_driver.ko

Verify that the module is loaded: lsmod | grep capstone_driver

Check that the device file was created: ls -l /dev/capstone_device

Give the current user read and write permission: sudo chmod 666 /dev/capstone_device

Verify the device permissions: ls -l /dev/capstone_device

The character device is used by the user-space applications to communicate with the Linux kernel driver.

## 5. Test the Linux Character Device Driver

Open a new terminal and navigate to the driver test directory:

cd ~/capstone/driver_test

Compile the driver test program: gcc driver_test.c -o driver_test

Run the test program: ./driver_test

Expected output:

Device opened successfully.
Data written to driver: Hello from user space!
Data read from driver: Hello from user space!
Device closed successfully.

This test verifies:

Opening the character device
Writing data from user space to the driver
Reading data from the driver back to user space
Closing the device

## 6. Build the Server

Open a new terminal and navigate to the server directory:

cd ~/capstone/server

Compile the server: gcc server.c -o server

Start the server: ./server

The server starts and listens for TCP client connections on port 8080.

Keep the server terminal running while testing the client.

## 7. Build the Client

Open another terminal and navigate to the client directory:

cd ~/capstone/client

Compile the client: gcc client.c -o client

Run the client: ./client

The client connects to the server using: 127.0.0.1:8080

Make sure the server is already running before starting the client.

## 8. Client Operations

After starting the client, the available operations are:

- **LIST** — Displays the files available in the shared directory.
- **INFO** — Displays information about a selected file.
- **SEARCH** — Searches for a file by filename.
- **DOWNLOAD** — Downloads a file from the server.
- **UPLOAD** — Uploads a local file to the server.
- **EXIT** — Closes the client connection.

These operations are performed through the TCP connection between the client and server.

## 9. Test File Download

Start the server:

cd ~/capstone/server
./server

In another terminal, start the client:

cd ~/capstone/client
./client

Select the DOWNLOAD operation and enter: document.txt

After the download completes, verify the downloaded file: cat ~/capstone/client/downloaded_document.txt

The downloaded content should match the original file stored in: ~/capstone/shared_files/document.txt

## 10. Test File Upload

A sample file is available in the client directory:

~/capstone/client/upload_test.txt

Start the server:

cd ~/capstone/server
./server

In another terminal, start the client:

cd ~/capstone/client
./client

Select the UPLOAD operation and enter: upload_test.txt

After the upload completes, verify that the file exists on the server: ls -l ~/capstone/shared_files/upload_test.txt

Verify the uploaded file contents: cat ~/capstone/shared_files/upload_test.txt

The uploaded file should contain: This file was uploaded through the TCP file sharing system.

## 11. Verify Driver Integration

The server communicates with the Linux character device driver after successful file upload and download operations.

After performing a file transfer, check the kernel messages:

sudo dmesg | tail -20

You should see messages similar to:

Capstone Driver: device opened
Capstone Driver: data written
Capstone Driver: device closed

These messages confirm that the user-space server successfully interacted with the Linux character device driver.

The communication flow is:

Client
   │
   │ TCP
   ▼
Server
   │
   │ write()
   ▼
/dev/capstone_device
   │
   ▼
Linux Character Device Driver
   │
   ▼
Linux Kernel

## 12. Stop the Linux Character Device Driver

After completing the demonstration, the kernel module can be unloaded using: sudo rmmod capstone_driver

Verify that the module has been unloaded: lsmod | grep capstone_driver

No output indicates that the module is no longer loaded.

## 13. Clean Build Files

To clean the Linux kernel module build files:

cd ~/capstone/driver
make clean

Remove compiled user-space executables:

rm -f ~/capstone/client/client
rm -f ~/capstone/server/server
rm -f ~/capstone/driver_test/driver_test

The source files are not affected by these commands.

## 14. Complete Demonstration Procedure

For a complete project demonstration, use the following sequence.

Step 1 — Build and load the driver
cd ~/capstone/driver
make
sudo insmod capstone_driver.ko
sudo chmod 666 /dev/capstone_device

Verify: 
lsmod | grep capstone_driver
ls -l /dev/capstone_device

Step 2 — Build and start the server
In a new terminal:
cd ~/capstone/server
gcc server.c -o server
./server

Keep this terminal running.

Step 3 — Build and start the client
In another terminal:
cd ~/capstone/client
gcc client.c -o client
./client

Step 4 — Demonstrate client operations
Demonstrate the available operations:
LIST
INFO
SEARCH
DOWNLOAD
UPLOAD
EXIT

Step 5 — Verify driver activity
After performing file transfers:
sudo dmesg | tail -20
Verify that the driver reports device activity.

Step 6 — Test the driver independently
In another terminal:
cd ~/capstone/driver_test
gcc driver_test.c -o driver_test
./driver_test

Expected output:

Device opened successfully.
Data written to driver: Hello from user space!
Data read from driver: Hello from user space!
Device closed successfully.

## 15. Important Notes

The project is designed for Linux.
The Linux character device driver requires sudo privileges to load and unload.
The server must be running before starting the client.
The /dev/capstone_device device must exist before testing driver integration.
The client and server communicate through TCP on port 8080.
Generated executable files and kernel module build artifacts are excluded from Git using .gitignore.
The project source code is written in C.

Then save: **Ctrl + O → Enter → Ctrl + X**

After that, run: cat docs/EXECUTION.md
