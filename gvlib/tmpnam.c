#include <stdio.h>

int main(void)
{
    char path[108];

    tmpnam(path);

    printf("%s\n", path);

}	