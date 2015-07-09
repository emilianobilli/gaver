#include <netdb.h>
#include <stdio.h>


int main(void)
{
    struct servent *ent;

    ent = getservbyname("gaver", NULL);

    if (ent != NULL)
    {
	printf ("%d\n", ntohs( (unsigned short)ent->s_port));
    }

    return 0;
}