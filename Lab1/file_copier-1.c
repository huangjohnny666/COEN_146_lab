//
//  Created by Behnam Dezfouli
//  CSEN, Santa Clara University
//
//
// This program offers two options to the user:
// -- Option 1: enables the user to copy a file to another file. 
// The user needs to enter the name of the source and destination files.
// -- Option 2: enables the user to measure the performance of file copy. 
// In addition to the name of source and destination files, the user needs to enter the maximum source file size as well as the step size to increment the source file size.
//
//
// double copier(FILE *s_file, FILE *d_file)
// Precondition: s_file is a pointer to a file opened in read mode, d_file is a file pointer opened in write mode
// Postcondition: Copies the contents of the file pointed to by s_file to the file pointed to by d_file
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 1
#define DUM_CHAR 'A'

// Function to copy a file using function calls
double function_call_copier(const char *sourceFile, const char *destinationFile) {
    clock_t start, end;
    double time_used;

    // Start time measurement
    start = clock();

    // Open source file for reading
    FILE *sourceFilePtr = fopen(sourceFile, "rb");
    if (sourceFilePtr == NULL) {
        perror("Cannot access source file");
        return -1.0;
    }

    // Open destination file for writing
    FILE *destinationFilePtr = fopen(destinationFile, "wb");
    if (destinationFilePtr == NULL) {
        perror("Cannot access destination file");
        fclose(sourceFilePtr);
        return -1.0;
    }

    size_t bytesRead;
    char buffer[100];

    // Read from sourceFile and write into destinationFile
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFilePtr)) > 0) {
        fwrite(buffer, 1, bytesRead, destinationFilePtr);
    }

    fclose(sourceFilePtr);
    fclose(destinationFilePtr);

    // Stop time measurement
    end = clock();

    // Calculate the time used
    time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    return time_used;
}

// Function to copy a file using system calls
double system_call_copier(const char *sourceFile, const char *destinationFile) {
    clock_t start, end;
    double time_used;

    // Start time measurement
    start = clock();

    int sourceFd = open(sourceFile, O_RDONLY);
    if (sourceFd == -1) {
        perror("Cannot access source file");
        return -1.0;
    }

    int destinationFd = open(destinationFile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (destinationFd == -1) {
        perror("Cannot access destination file");
        close(sourceFd);
        return -1.0;
    }

    char buffer[100];
    ssize_t bytesRead;

    while ((bytesRead = read(sourceFd, buffer, sizeof(buffer))) > 0) {
        if (write(destinationFd, buffer, bytesRead) != bytesRead) {
            perror("Error writing file");
            close(sourceFd);
            close(destinationFd);
            return -1.0;
        }
    }

    close(sourceFd);
    close(destinationFd);

    // Stop time measurement
    end = clock();

    // Calculate the time used
    time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    return time_used;
}

int main() {
    // ask for option
    int option;
    printf("Enter 1 for option 1 or 2 for option 2: ");
    scanf("%d", &option);

    if (option == 1) // File copy
    {
        char sourceFile[100], destinationFile[100];

        double time_used_option1;
        // Receive the name of source file from the user
        printf("Enter your source file name: ");
        scanf("%s", sourceFile);

        // Receive the name of destination file from the user
        printf("Enter your destination file name: ");
        scanf("%s", destinationFile);

        // Ask which type of call
        int call_option;
        printf("Enter 1 for function_call or 2 for system_call: ");
        scanf("%d", &call_option);

        if (call_option == 1) {
            // Using function call from above
            time_used_option1 = function_call_copier(sourceFile, destinationFile);
        } else if (call_option == 2) {
            // Using system call from above
            time_used_option1 = system_call_copier(sourceFile, destinationFile);
        } else {
            printf("Invalid function call\n");
            return 1;
        }

        printf("Total time for option 1: %f seconds\n", time_used_option1);
    }
    else if (option == 2) // File copy with performance measurement
    {
        char sourceFile[100], destinationFile[100];
        int maximum_file_size, step_size;



        // Receive the name of source file from the user
        printf("Enter your source file name: ");
        scanf("%s", sourceFile);

        // Receive maximum_file_size (in bytes) from the user
        printf("Enter maximum_file_size (in bytes): ");
        scanf("%d", &maximum_file_size);

        // Receive step_size (in bytes) from the user
        printf("Enter step_size (in bytes): ");
        scanf("%d", &step_size);

        int current_size = 0;
        double time_used_option2;

        while (current_size <= maximum_file_size) {


            // Start time measurement
            clock_t start = clock();

            // Create a file of current_size
            FILE *filePtr = fopen(sourceFile, "wb");
            
            // Write dummy data to the file
            int i;
            for(i=0; i < current_size; i++) {
                fputc(DUM_CHAR, filePtr);
            }
            fclose(filePtr);



            // Copy source file to destination file
            if (system_call_copier(sourceFile, destinationFile) == -1.0) {
                printf("Error during file copy\n");
                return 1;
            }

            // Stop time measurement
            clock_t end = clock();

            // Calculate the time used
            time_used_option2 = ((double)(end - start)) / CLOCKS_PER_SEC;

            printf("Time taken for current file size %d: %f seconds\n", current_size, time_used_option2);

            // Increment the current size
            current_size += step_size;
        }
    }
    else {
        printf("Invalid option\n");
        return 1;
    }

    return 0;
}
