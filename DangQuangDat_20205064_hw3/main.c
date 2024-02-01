#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex.h>
#include <stdlib.h>

bool checkValidIpAddress(char ipAddress[]);
bool checkValidDomain(char domain[]);
void getDomainFromIpAddress(char ipAddress[]);
void getIpFromDomainAddress(char domain[]);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Input invalid");
        return -1;
    }

    char mode[2];
    char param[255];
    strncpy(mode, argv[1], sizeof(mode));
    strncpy(param, argv[2], sizeof(param));

    if (strcmp(mode, "1") == 0)
    {
        if (checkValidDomain(param))
        {
            printf("Wrong parameter\n");
            return 0;
        }

        if (!checkValidIpAddress(param))
        {
            printf("IP Address invalid\n");
            return 0;
        }
        getDomainFromIpAddress(param);
    }

    if (strcmp(mode, "2") == 0)
    {
        if (checkValidIpAddress(param))
        {
            printf("Wrong parameter\n");
            return 0;
        }

        if (!checkValidDomain(param))
        {
            printf("Domain invalid\n");
            return 0;
        }
        getIpFromDomainAddress(param);
    }
    return 0;
}

bool checkValidIpAddress(char ipAddress[])
{
    struct in_addr addr;
    return inet_pton(AF_INET, ipAddress, &addr) == 1;
}

bool checkValidDomain(char domain[])
{
    regex_t regexT;
    int isValid;
    const char *pattern = "^([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6}$";
    isValid = regcomp(&regexT, pattern, REG_EXTENDED);
    if (isValid != 0)
    {
        return false;
    }
    isValid = regexec(&regexT, domain, 0, NULL, 0);
    regfree(&regexT);
    return isValid == 0;
}

void getDomainFromIpAddress(char ipAddress[])
{
    struct in_addr addr;
    inet_pton(AF_INET, ipAddress, &addr);

    struct hostent *host = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);

    if (host != NULL)
    {
        printf("Official name: %s\n", host->h_name);
        if (host->h_aliases[0] != NULL)
        {
            printf("Alias name: \n");
            int i;
            for (i = 0; host->h_aliases[i] != NULL; i++)
            {
                printf("%s\n", host->h_aliases[i]);
            }
        }
        int alias_index = 0;

        char **alias;
        for (alias = host->h_aliases; *alias != NULL; alias++)
        {
            printf("Alias %d: %s\n", ++alias_index, *alias);
        }
    }
    else
    {
        printf("Not found information\n");
    }
}

void getIpFromDomainAddress(char domain[])
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result;
    int status = getaddrinfo(domain, NULL, &hints, &result);
    if (status != 0)
    {
        printf("Not found information\n");
        return;
    }

    struct addrinfo *current;
    int index = 0;

    for (current = result; current != NULL; current = current->ai_next)
    {
        if (current->ai_family != AF_INET)
        {
            continue;
        }
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)current->ai_addr;
        char ip4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip4, INET_ADDRSTRLEN);
        if (index == 0)
        {
            printf("Official IP: %s\n", ip4);
        }
        else if (index == 1)
        {
            printf("Alias IP:\n");
            printf("%s\n", ip4);
        }
        else
        {
            printf("%s\n", ip4);
        }

        index++;
    }

    freeaddrinfo(result);
    return;
}