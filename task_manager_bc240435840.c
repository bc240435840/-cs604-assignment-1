#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    char studentID[50];
    int tasks[3];
    int pipe1[2], pipe2[2]; // pipe1: parent -> child, pipe2: child -> parent

    // 1. Input from parent
    printf("Enter Student ID: ");
    scanf("%49s", studentID);

    printf("Enter Task 1 Execution Time: ");
    scanf("%d", &tasks[0]);
    printf("Enter Task 2 Execution Time: ");
    scanf("%d", &tasks[1]);
    printf("Enter Task 3 Execution Time: ");
    scanf("%d", &tasks[2]);

    // 2. Create two pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        return 1;
    }

    // 3. Create child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // ===== CHILD PROCESS =====
        close(pipe1[1]); // close write end of pipe1
        close(pipe2[0]); // close read end of pipe2

        int recvTasks[3];
        read(pipe1[0], recvTasks, sizeof(recvTasks));
        close(pipe1[0]);

        // Calculate total and longest task
        int total = recvTasks[0] + recvTasks[1] + recvTasks[2];
        int maxTime = recvTasks[0];
        int longestTask = 1;

        // Tie: smaller task number wins, so use > only, not >=
        if (recvTasks[1] > maxTime) {
            maxTime = recvTasks[1];
            longestTask = 2;
        }
        if (recvTasks[2] > maxTime) {
            maxTime = recvTasks[2];
            longestTask = 3;
        }

        int result[3] = {total, longestTask, maxTime};
        write(pipe2[1], result, sizeof(result));
        close(pipe2[1]);
        exit(0);
    } else {
        // ===== PARENT PROCESS =====
        close(pipe1[0]); // close read end of pipe1
        close(pipe2[1]); // close write end of pipe2

        write(pipe1[1], tasks, sizeof(tasks));
        close(pipe1[1]);

        int result[3];
        read(pipe2[0], result, sizeof(result));
        close(pipe2[0]);

        wait(NULL); // wait for child

        int total = result[0];
        int longestTask = result[1];
        int maxTime = result[2];

        // Display report
        printf("\n--- Task Processing Report ---\n");
        printf("Student ID: %s\n", studentID);
        printf("Task 1: %d units\n", tasks[0]);
        printf("Task 2: %d units\n", tasks[1]);
        printf("Task 3: %d units\n", tasks[2]);
        printf("Total Execution Time: %d units\n", total);
        printf("Longest Task: Task %d\n", longestTask);
        printf("Maximum Execution Time: %d units\n", maxTime);

        // Create report file name: bc240435840_report.txt
        char filename[70];
        snprintf(filename, sizeof(filename), "%s_report.txt", studentID);
        printf("Report saved to: %s\n", filename);

        // Save report using Linux file operations open() / write()
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("file open failed");
            return 1;
        }

        char buffer[500];
        int len = snprintf(buffer, sizeof(buffer),
            "--- Task Processing Report ---\n"
            "Student ID: %s\n"
            "Task 1: %d units\n"
            "Task 2: %d units\n"
            "Task 3: %d units\n"
            "Total Execution Time: %d units\n"
            "Longest Task: Task %d\n"
            "Maximum Execution Time: %d units\n",
            studentID, tasks[0], tasks[1], tasks[2], total, longestTask, maxTime);

        write(fd, buffer, len);
        close(fd);
    }

    return 0;
}
