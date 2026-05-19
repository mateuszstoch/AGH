#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define M 5
int global_int = 0;

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        int n = atoi(argv[1]);
        for (int j = 0; j < n; j++)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                execvp("./child", &argv[2]);
            }
        }
        for (int i = 0; i < n; i++)
        {
            wait(NULL);
        }
        printf("Rodzic (PID: %d)\n", getpid());
    }
    return 0;
}
