README
Team members: Viyoka Lim
Section Number: Section 01 - Class Nbr 10154 - Discussion
Email: Vlim7@csufullerton.edu
Programming Language: C++
Introduction
This project demonstrates interprocess communication (IPC) in a Linux environment using shared memory, message queues, and signal handling. The sender program transfers a file to the receiver program by writing data to a shared memory segment, and the receiver reads the data and writes it to an output file. Signal handling ensures proper cleanup of shared resources in the event of unexpected termination.
________________


Design Overview
Sender Program
* Open the input file and read it in chunks.
* Sends the file name and data to the receiver using a message queue.
* Writes data to shared memory and waits for acknowledgment.
* Signals the end of the transfer by sending a message with size = 0.
Receiver Program
* Sets up shared memory and message queues.
* Receives the file name and creates the output file.
* Read data from shared memory and write it to the file.
* Send acknowledgement back to the sender.
* Cleans up resources upon termination.
Signal Handling
* Handles SIGINT (Ctrl+C) to deallocate shared memory and message queues gracefully and ensure no resources are left hanging.
________________


Implementation Details
Key Components
1. Message Queue (msg.h):
   * Contains message structures for:
      * File name transfer.
      * Data transfer.
      * Acknowledgment messages.
2. Sender Program (sender.cpp):
   * Reads input files and sends data to the receiver using shared memory and message queues.
3. Receiver Program (recv.cpp):
   * Reads from shared memory and writes received data to the output file.
4. Signal Handling (signaldemo.cpp):
   * Demonstrates handling of signals like SIGINT (Ctrl+C) to ensure graceful cleanup of resources
________________


Challenges
During the project setup:
* Directory Structure:
   * The inclusion of nested folders (e.g., samplefiles) caused minor confusion, especially with file paths like keyfile.txt.
   * This was resolved by ensuring consistent working directories and paths in the code.
* Environment:
   * Some headers like sys/shm.h and sys/msg.h required a properly configured Linux environment (e.g., WSL or native Linux).
   * Testing on MSYS2 highlighted differences between Windows and Linux IPC features, requiring adjustments for proper execution.
________________


Special Notes
1. The program assumes the presence of a file named keyfile.txt in the working directory (samplefiles) for generating unique IPC keys.
2. Tested primarily in a Linux-like environment using WSL; behavior on native Linux should be identical.
3. Ensure proper cleanup of shared resources by terminating the sender and receiver programs cleanly or by sending SIGINT (Ctrl+C).