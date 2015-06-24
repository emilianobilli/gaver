#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    char sun_path[104];

/*    srand(time()); */
    sprintf (&sun_path[0], "%d-%d-%d.sock", time(), getpid(), rand()%100);

    printf("%s\n", sun_path);
    
/*    sprintf (&sun_path[0], "%d-%d-%d.sock", time(), getpid(), rand()%100);*/

    printf("%s\n", sun_path);
    return 0;
}