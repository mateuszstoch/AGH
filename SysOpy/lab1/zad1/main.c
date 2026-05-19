#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define M 5
int global_int = 0;

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        int count = atoi(argv[1]);

        for (int j = 0; j < count; j++)
        {

            int pid = vfork();
            switch (pid)
            {
            case -1:
                return 0;
            case 0:
                for (int i = 0; i < M; i++)
                {
                    global_int++;
                    printf("Potomek (PID: %d)\n", getpid());
                    sleep(0.25);
                }
                exit(0);
                break;
            default:
                break;
            }
        }
        for (int i = 0; i < count; i++)
        {
            wait(NULL);
        }
        printf("Rodzic (PID: %d), global_value=%d\n", getpid(), global_int);
    }
    return 0;
}
