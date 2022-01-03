#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define HRACIA_PLOCHA_VELKOST_X 25
#define HRACIA_PLOCHA_VELKOST_Y 60
#define MAX_VELKOST_HADA 100

typedef struct clanok{
    int poziciaX;
    int poziciaY;
} CLANOK_DATA;

typedef struct hrac{
    CLANOK_DATA clanky_hada[MAX_VELKOST_HADA];
    int velkostHada;
    char smer;
} HRAC_DATA;

typedef struct hraciePole{
    char pole[HRACIA_PLOCHA_VELKOST_X][HRACIA_PLOCHA_VELKOST_Y];
} HRACIE_POLE_DATA;

bool posunHada(HRAC_DATA * hracData, HRACIE_POLE_DATA * hraciePoleData){
    int predX = 0;
    int predY = 0;
    for (int j = 0; j < hracData->velkostHada; ++j) {
        if (j == 0) {
            predX = hracData->clanky_hada[j].poziciaX;
            predY = hracData->clanky_hada[j].poziciaY;
            switch (hracData->smer) {
                case 'u':
                    hracData->clanky_hada[j].poziciaX = hracData->clanky_hada[j].poziciaX - 1;
                    hracData->clanky_hada[j].poziciaY = hracData->clanky_hada[j].poziciaY;
                    break;
                case 'd':
                    hracData->clanky_hada[j].poziciaX = hracData->clanky_hada[j].poziciaX + 1;
                    hracData->clanky_hada[j].poziciaY = hracData->clanky_hada[j].poziciaY;
                    break;
                case 'l':
                    hracData->clanky_hada[j].poziciaX = hracData->clanky_hada[j].poziciaX;
                    hracData->clanky_hada[j].poziciaY = hracData->clanky_hada[j].poziciaY - 1;
                    break;
                case 'r':
                    hracData->clanky_hada[j].poziciaX = hracData->clanky_hada[j].poziciaX;
                    hracData->clanky_hada[j].poziciaY = hracData->clanky_hada[j].poziciaY + 1;
                    break;
            }
            //Kontrola ci nenarazil do steny
            if (hracData->clanky_hada[j].poziciaX < 0 || hracData->clanky_hada[j].poziciaX >= HRACIA_PLOCHA_VELKOST_X) {
                return false;
            }
            if (hracData->clanky_hada[j].poziciaY < 0 || hracData->clanky_hada[j].poziciaY >= HRACIA_PLOCHA_VELKOST_Y) {
                return false;
            }
            //Kontrola ci nenarazil sam do seba
            for (int i = 1; i < hracData->velkostHada; ++i) {
                if (hracData->clanky_hada[i].poziciaX == hracData->clanky_hada[0].poziciaX && hracData->clanky_hada[i].poziciaY == hracData->clanky_hada[0].poziciaY){
                    return  false;
                }
            }
        } else {
            int x = hracData->clanky_hada[j].poziciaX;
            int y = hracData->clanky_hada[j].poziciaY;
            hracData->clanky_hada[j].poziciaX = predX;
            hracData->clanky_hada[j].poziciaY = predY;
            predX = x;
            predY = y;
        }
    }
    // PREPISANIE HRACEJ PLOCHY
    for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
            hraciePoleData->pole[i][j] = ' ';
        }
    }
    for (int j = 0; j < hracData->velkostHada; ++j) {
        hraciePoleData->pole[hracData->clanky_hada[j].poziciaX]
        [hracData->clanky_hada[j].poziciaY] = 'x';
    }
    return true;
}

void vykreslenieHracejPlochy(HRACIE_POLE_DATA * hraciePoleData){
    for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y + 2; ++j) {
        printf("-");
    }
    printf("\n");
    for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
            if (j == 0 ) {
                printf("|");
            }
            printf("%c",(*hraciePoleData).pole[i][j]);
            if (j == HRACIA_PLOCHA_VELKOST_Y - 1 ) {
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


int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2)
    {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }

    // VYTVORENIE HRACEJ PLOCHY
    HRACIE_POLE_DATA hraciePoleData;
    for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
            hraciePoleData.pole[i][j] = ' ';
        }
    }

    // VYTVORENIE HRACA
    HRAC_DATA hrac1;
    hrac1.velkostHada = 10;
    hrac1.smer = 'r';
    int i = 0;
    for (int y= hrac1.velkostHada - 1; y >= 0; --y) {
        hrac1.clanky_hada[i].poziciaX = 0;
        hrac1.clanky_hada[i].poziciaY = y;
        i++;
    }
    for (int j = 0; j < hrac1.velkostHada; ++j) {
        hraciePoleData.pole[hrac1.clanky_hada[j].poziciaX]
            [hrac1.clanky_hada[j].poziciaY] = 'x';
    }


    // ZACIATOK CYKLU
    bool koniec = false;
    while (!koniec) {
       //Nacitanie zmeny smeru
        bzero(buffer,256);
        n = read(newsockfd, buffer, 255);
        printf("Here is the message: %s\n", buffer);
        if (n < 0)
        {
            perror("Error reading from socket");
            return 4;
        }
        switch (buffer[0]) {
            case 'q':
                koniec = true;
                printf("Koniec hry!");
                break;
            case 'w':
                if (hrac1.smer != 'd') {
                    hrac1.smer = 'u';
                }
                break;
            case 's':
                if (hrac1.smer != 'u') {
                    hrac1.smer = 'd';
                }
                break;
            case 'a':
                if (hrac1.smer != 'r') {
                    hrac1.smer = 'l';
                }
                break;
            case 'd':
                if (hrac1.smer != 'l') {
                    hrac1.smer = 'r';
                }
                break;
            default:
                break;
        }
        //Posun hada
        if (posunHada(&hrac1,&hraciePoleData)) {
            bzero(buffer,256);
            buffer[0] = 'f';
            n = write(newsockfd,buffer, 255);
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
            n = write(newsockfd,hraciePoleData.pole,sizeof (hraciePoleData.pole));
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
        } else {
            bzero(buffer,256);
            buffer[0] = 't';
            n = write(newsockfd,buffer, 255);
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
            koniec = true;
        }


    }

    close(newsockfd);
    close(sockfd);

    return 0;
}
