# Testing Documentation

## 1. Testing Overview

The project was tested on Ubuntu Linux to verify the functionality of:

- Linux character device driver
- TCP server
- TCP client
- File upload
- File download
- File listing
- File information
- File search
- Driver integration
- Filename validation
- Error handling

All major components were tested independently and together.

---

## 2. Driver Compilation Test

Navigate to the driver directory:


cd ~/capstone/driver

Build the kernel module:

make
Expected Result

The build should complete successfully and generate:

capstone_driver.ko

## 3. Driver Loading Test

Load the kernel module:

sudo insmod capstone_driver.ko

Verify that the module is loaded:

lsmod | grep capstone_driver
Expected Result

The module name should appear in the output:

capstone_driver

## 4. Device File Test

Check whether the device file exists:

ls -l /dev/capstone_device
Expected Result

A character device named:

/dev/capstone_device

should be present.

Set permissions for testing:

sudo chmod 666 /dev/capstone_device

## 5. Character Device Driver Functional Test

Compile the driver test program:

cd ~/capstone/driver_test
gcc driver_test.c -o driver_test

Run:

./driver_test
Expected Result
Device opened successfully.
Data written to driver: Hello from user space!
Data read from driver: Hello from user space!
Device closed successfully.
Result

PASS

The test confirms that:

The device can be opened.
User-space data can be written to the driver.
Data can be read back from the driver.
The device can be closed successfully.

## 6. Kernel Log Test

Check kernel messages:

sudo dmesg | tail -20

The driver should generate messages such as:

Capstone Driver: initializing
Capstone Driver: registered with major number
Capstone Driver: device opened
Capstone Driver: data written
Capstone Driver: data read
Capstone Driver: device closed
Result

PASS

The kernel messages confirm that the driver functions were invoked successfully.

## 7. Server Compilation Test

Navigate to the server directory:

cd ~/capstone/server

Compile:

gcc server.c -o server
Result

PASS

The server compiles successfully without compilation errors.

## 8. Client Compilation Test

Navigate to the client directory:

cd ~/capstone/client

Compile:

gcc client.c -o client
Result

PASS

The client compiles successfully without compilation errors.

## 9. TCP Connection Test

Start the server:

cd ~/capstone/server
./server

Start the client in another terminal:

cd ~/capstone/client
./client
Expected Result

The client should establish a TCP connection with:

127.0.0.1:8080
Result

PASS

The client successfully connects to the server.

## 10. LIST Operation Test

Select the LIST operation from the client.

Expected Result

The server returns the files available in:

shared_files/

Example files include:

document.txt
readme.txt
upload_test.txt
Result

PASS

The client successfully retrieves the available file list.

## 11. INFO Operation Test

Select the INFO operation and provide a valid filename.

Example:

document.txt
Expected Result

The server returns information about the requested file.

Result

PASS

The file information operation works correctly.

## 12. SEARCH Operation Test

Select the SEARCH operation and search for:

document.txt
Expected Result

The system reports whether the file exists in the shared directory.

Result

PASS

The search operation works correctly.

## 13. File Download Test

Select the DOWNLOAD operation.

Download:

document.txt
Expected Result

The file is transferred from:

server/shared_files/

to the client.

Verify:

cat ~/capstone/client/downloaded_document.txt
Result

PASS

The downloaded file content matches the original file.

## 14. File Upload Test

A sample file is available at:

~/capstone/client/upload_test.txt

Select the UPLOAD operation and upload:

upload_test.txt

Verify on the server:

ls -l ~/capstone/shared_files/upload_test.txt

Check the content:

cat ~/capstone/shared_files/upload_test.txt
Expected Content
This file was uploaded through the TCP file sharing system.
Result

PASS

The file is successfully transferred from the client to the server.

## 15. Driver Integration Test

After a successful upload or download, check:

sudo dmesg | tail -20

The driver should show activity such as:

Capstone Driver: device opened
Capstone Driver: data written
Capstone Driver: device closed
Result

PASS

The server successfully communicates with the Linux character device driver.

## 16. Filename Validation Test

The server validates filenames before accessing files.

Test invalid filename patterns containing:

..
/
\
Expected Result

The server rejects the request and returns an error.

Example:

ERROR: Invalid filename.
Result

PASS

The validation prevents basic path traversal attempts.

## 17. Invalid File Test

Request a file that does not exist.

Example:

nonexistent.txt
Expected Result

The server should return an appropriate error instead of crashing.

Result

PASS

Invalid file requests are handled safely.

## 18. Connection Handling Test

Close the client connection while the server is running.

Expected Result

The server detects the closed connection and continues operating without crashing.

Result

PASS

The server handles client disconnections correctly.

## 19. Test Summary
Test	                Result
Driver compilation	PASS
Driver loading	        PASS
Device creation	        PASS
Driver read/write	PASS
Kernel logging	        PASS
Server compilation	PASS
Client compilation	PASS
TCP connection	        PASS
LIST operation	        PASS
INFO operation	        PASS
SEARCH operation	PASS
File download	        PASS
File upload	        PASS
Driver integration	PASS
Filename validation	PASS
Invalid file handling	PASS
Connection handling	PASS

## 20. Testing Conclusion

Testing confirms that the major components of the Linux Network File Sharing System work together correctly.

The project successfully demonstrates:

TCP client-server communication
File upload and download
File management operations
Linux character device driver functionality
User-space to kernel-space interaction
Basic filename security validation
Error handling and connection management

Save it:

**Ctrl + O → Enter → Ctrl + X**

Then verify the three documentation files exist:

cd ~/capstone
find docs -maxdepth 1 -type f | sort
