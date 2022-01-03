#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define HRACIA_PLOCHA_VELKOST_X 25
#define HRACIA_PLOCHA_VELKOST_Y 60

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    bool koniec = false;
    while (!koniec) {
        bzero(buffer,256);
        fgets(buffer, 255, stdin);
        n = write(sockfd, buffer, strlen(buffer));
        if (buffer[0] == 'q') {
            koniec = true;
            printf("Koniec hry!\n");
        }
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }

        // VYPISANIE HRACEJ PLOCHY a
        char pole[HRACIA_PLOCHA_VELKOST_X][HRACIA_PLOCHA_VELKOST_Y];
        n = read(sockfd, pole, sizeof(pole));
        if (n < 0) {
            perror("Error reading from socket");
            return 6;
        }
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y + 2; ++j) {
            printf("-");
        }
        printf("\n");
        for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
            for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
                if (j == 0) {
                    printf("|");
                }
                printf("%c", pole[i][j]);
                if (j == HRACIA_PLOCHA_VELKOST_Y - 1) {
                    printf("|");
                }
            }
            printf("\n");
        }
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y + 2; ++j) {
            printf("-");
        }
        printf("\n");
    }

    close(sockfd);
    return 0;



    //ahojky
}
