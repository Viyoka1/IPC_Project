#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "msg.h" // Include the message header file

using namespace std;

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* Global variables for cleanup */
int shmid, msqid;
void *sharedMemPtr = NULL;

/**
 * Signal handler to clean up resources when SIGINT is caught
 */
void signalHandlerFunc(int signal)
{
    fprintf(stderr, "\nCaught SIGINT! Cleaning up resources...\n");

    /* Detach from shared memory */
    if (shmdt(sharedMemPtr) == -1)
        perror("shmdt");

    /* Deallocate shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        perror("shmctl");

    /* Deallocate message queue */
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
        perror("msgctl");

    exit(0);
}

/**
 * Sets up the shared memory segment and message queue
 */
void init(int &shmid, int &msqid, void *&sharedMemPtr)
{
    /* Generate a key using ftok() */
    key_t key = ftok("keyfile.txt", 'a');
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    printf("Key generated successfully: %d\n", key);

    /* Allocate shared memory */
    shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    /* Attach to shared memory */
    sharedMemPtr = shmat(shmid, (void *)0, 0);
    if (sharedMemPtr == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    /* Create message queue */
    msqid = msgget(key, 0644 | IPC_CREAT);
    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    fprintf(stderr, "Shared memory and message queue initialized successfully.\n");
}

/**
 * Receive the file name from the sender
 * @return - the name of the file received from the sender
 */
string recvFileName()
{
    fileNameMsg fileNameMsgBuffer;

    /* Receive the file name */
    if (msgrcv(msqid, &fileNameMsgBuffer, sizeof(fileNameMsgBuffer) - sizeof(long), FILE_NAME_TRANSFER_TYPE, 0) == -1)
    {
        perror("msgrcv");
        exit(1);
    }

    /* Extract the file name */
    return string(fileNameMsgBuffer.fileName);
}

/**
 * Main loop to receive file data
 * @param fileName - the name of the file received from the sender
 * @return - the total number of bytes received
 */
unsigned long mainLoop(const string &fileName)
{
    int msgSize = -1; // Message size received from the sender
    unsigned long totalBytesReceived = 0;

    /* Open the file for writing */
    string recvFileName = fileName + "__recv";
    FILE *fp = fopen(recvFileName.c_str(), "w");
    if (!fp)
    {
        perror("fopen");
        exit(1);
    }

    fprintf(stderr, "Receiving file: %s\n", recvFileName.c_str());

    /* Keep receiving messages until the sender sets the size to 0 */
    while (msgSize != 0)
    {
        message msg;

        /* Receive the message from the sender */
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SENDER_DATA_TYPE, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }

        msgSize = msg.size;

        if (msgSize > 0)
        {
            /* Write the shared memory contents to the file */
            if (fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < (size_t)msgSize)
            {
                perror("fwrite");
                exit(1);
            }

            totalBytesReceived += msgSize;

            /* Send acknowledgment to the sender */
            ackMessage ackMsg;
            ackMsg.mtype = RECV_DONE_TYPE;
            if (msgsnd(msqid, &ackMsg, sizeof(ackMsg) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "File transfer complete.\n");
            fclose(fp);
        }
    }

    return totalBytesReceived;
}

/**
 * Clean up resources before exiting
 */
void cleanUp(const int &shmid, const int &msqid, void *sharedMemPtr)
{
    /* Detach from shared memory */
    if (shmdt(sharedMemPtr) == -1)
        perror("shmdt");

    /* Deallocate shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        perror("shmctl");

    /* Deallocate message queue */
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
        perror("msgctl");

    fprintf(stderr, "Cleaned up resources successfully.\n");
}

int main()
{
    /* Set up signal handler */
    signal(SIGINT, signalHandlerFunc);

    /* Initialize shared memory and message queue */
    init(shmid, msqid, sharedMemPtr);

    /* Receive the file name */
    string fileName = recvFileName();
    fprintf(stderr, "Received file name: %s\n", fileName.c_str());

    /* Start the main loop to receive file data */
    unsigned long bytesReceived = mainLoop(fileName);
    fprintf(stderr, "Total bytes received: %lu\n", bytesReceived);

    /* Clean up resources */
    cleanUp(shmid, msqid, sharedMemPtr);

    return 0;
}
