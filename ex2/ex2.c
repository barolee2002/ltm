#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void ipToDNS(char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    struct in_addr address;
    inet_aton(ip, &address);
    if ((he = gethostbyaddr(&address, sizeof(struct in_addr), AF_INET)) == NULL)
    {
        printf("Not found information\n");
        return;
    }

    printf("Official name: %s\n", he->h_name);
    printf("Alias name:\n");

    for (int i = 0; he->h_aliases[i] != NULL; i++)
    {
        printf("%s\n", he->h_aliases[i]);
    }

}
void dnsToIP(char *domain)
{
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(domain)) == NULL)
    {
        printf("Not found information\n");
        return;
    }

    printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr_list[0]));
    printf("Alias IP:\n");

    for (int i = 1; he->h_addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*(struct in_addr *)he->h_addr_list[i]));
    }
}
int checkArgv2(char *argv)
{
    struct in_addr address;
    if (inet_pton(AF_INET, argv, &address) == 0)
    {
        return 1;
    }
    return 0;
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Erorr Input\n");
        return 1;
    }

    char *argv1 = argv[1];
    char *argv2 = argv[2];
    if (strcmp(argv1, "1") == 0 && !checkArgv2(argv2))
    {
        ipToDNS(argv2);
    }
    else if (strcmp(argv1, "2") == 0 && checkArgv2(argv2))
    {
        dnsToIP(argv2);
    }
    else
    {
        printf("Wrong parameter\n");
        return 1;
    }

    return 0;
}
