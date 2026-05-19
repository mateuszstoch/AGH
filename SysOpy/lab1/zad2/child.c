#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        int count = atoi(argv[0]);

        for (int j = 0; j < count; j++)
        {
            printf("Potomek (PID: %d)\n", getpid());
            sleep(0.25);
        }
    }
    exit(0);
}