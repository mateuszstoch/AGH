#include "definitions.h"
#include <sys/file.h>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        int count = atoi(argv[0]);

        for (int j = 0; j < count; j++)
        {
            FILE *f = fopen(OUTPUT_FILE, "a");
            if (f == NULL)
            {
                perror("fopen");
                exit(1);
            }

            flock(fileno(f), LOCK_EX);
            fprintf(f, "Potomek (PID: %d)\n", getpid());

            fflush(f);

            flock(fileno(f), LOCK_UN);

            fclose(f);
            sleep(0.25);
        }
    }
    exit(0);
}