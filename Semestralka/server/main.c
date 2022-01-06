#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define HRACIA_PLOCHA_VELKOST_X 25
#define HRACIA_PLOCHA_VELKOST_Y 60
#define MAX_VELKOST_HADA 100
#define MAX_POCET_HRIBOV 20

typedef struct clanok{
    int poziciaX;
    int poziciaY;
} CLANOK_DATA;

typedef struct hrac{
    char id;
    CLANOK_DATA clanky_hada[MAX_VELKOST_HADA];
    int velkostHada;
    char smer;
    int socket;
} HRAC_DATA;

typedef struct hraciePole{
    char pole[HRACIA_PLOCHA_VELKOST_X][HRACIA_PLOCHA_VELKOST_Y];
    bool hraSkoncila;
    int pocetHribov;
    pthread_mutex_t * mutexPocetHribov;
    pthread_cond_t * pridajHrib;
    HRAC_DATA * hrac1;
    HRAC_DATA * hrac2;
    int stavHry;
} HRACIE_POLE_DATA;

void posunClankov(HRAC_DATA * hracData, HRACIE_POLE_DATA * hraciePoleData){
    int predX = 0;
    int predY = 0;
    bool zobralHrib = false;
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
            //Kontrola zobral hrib
            if (hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'H'){
                pthread_mutex_lock(hraciePoleData->mutexPocetHribov);
                hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] = ' ';
                hraciePoleData->pocetHribov--;
                pthread_cond_signal(hraciePoleData->pridajHrib);
                pthread_mutex_unlock(hraciePoleData->mutexPocetHribov);
                if (hracData->velkostHada < MAX_VELKOST_HADA){
                    zobralHrib = true;
                }
            }
            //Kontrola ci nenarazil do steny
            if (hracData->clanky_hada[j].poziciaX < 0 || hracData->clanky_hada[j].poziciaX >= HRACIA_PLOCHA_VELKOST_X) {
                hraciePoleData->hraSkoncila = true;
                if ( hracData->id == 'X') {
                    hraciePoleData->stavHry = 2;
                }
                if ( hracData->id == 'O') {
                    if (hraciePoleData->stavHry == 2) {
                        hraciePoleData->stavHry = 3;
                    } else {
                        hraciePoleData->stavHry = 1;
                    }
                }
                return;
            }
            if (hracData->clanky_hada[j].poziciaY < 0 || hracData->clanky_hada[j].poziciaY >= HRACIA_PLOCHA_VELKOST_Y) {
                hraciePoleData->hraSkoncila = true;
                if ( hracData->id == 'X') {
                    hraciePoleData->stavHry = 2;
                }
                if ( hracData->id == 'O') {
                    if (hraciePoleData->stavHry == 2) {
                        hraciePoleData->stavHry = 3;
                    } else {
                        hraciePoleData->stavHry = 1;
                    }
                }
                return;
            }
            //Kontrola ci nenarazil sam do seba alebo do 2. hraca
            if (hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'O' ||
                    hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'X'){
                hraciePoleData->hraSkoncila = true;
                if ((hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'O' ||
                    hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'X') &&
                    hracData->id == 'O') {
                    if (hraciePoleData->stavHry == 2) {
                        hraciePoleData->stavHry = 3;
                    } else {
                        hraciePoleData->stavHry = 1;
                    }
                    return;
                }
                if ((hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'O' ||
                     hraciePoleData->pole[hracData->clanky_hada[j].poziciaX][hracData->clanky_hada[j].poziciaY] == 'X') &&
                    hracData->id == 'X') {
                    hraciePoleData->stavHry = 2;
                    return;
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
    //Kontrola zvacsenia
    if (zobralHrib){
        hracData->velkostHada++;
        hracData->clanky_hada[hracData->velkostHada-1].poziciaX = predX;
        hracData->clanky_hada[hracData->velkostHada-1].poziciaY = predY;
    }
    // PREPISANIE HRACEJ PLOCHY
    for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
            if (hraciePoleData->pole[i][j] != 'H') {
                hraciePoleData->pole[i][j] = ' ';
            }
        }
    }
    for (int j = 0; j < hraciePoleData->hrac1->velkostHada; ++j) {
        hraciePoleData->pole[hraciePoleData->hrac1->clanky_hada[j].poziciaX]
        [hraciePoleData->hrac1->clanky_hada[j].poziciaY] = 'X';
    }
    for (int j = 0; j < hraciePoleData->hrac2->velkostHada; ++j) {
        hraciePoleData->pole[hraciePoleData->hrac2->clanky_hada[j].poziciaX]
        [hraciePoleData->hrac2->clanky_hada[j].poziciaY] = 'O';
    }
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

void * generovanieHribovF(void * hraciePole){
    HRACIE_POLE_DATA * dataP = hraciePole;
    printf("Zacalo sa generovanie kolacov\n");
    while (!dataP->hraSkoncila){
        sleep(3);
        bool vygeneroval = false;
        pthread_mutex_lock(dataP->mutexPocetHribov);
        while (dataP->pocetHribov >= MAX_POCET_HRIBOV){
            pthread_cond_wait(dataP->pridajHrib,dataP->mutexPocetHribov);
        }
        while (!vygeneroval) {
            int x = rand() % HRACIA_PLOCHA_VELKOST_X;
            int y = rand() % HRACIA_PLOCHA_VELKOST_Y;
            if (dataP->pole[x][y] == ' ') {
                dataP->pole[x][y] = 'H';
                vygeneroval = true;
                dataP->pocetHribov++;
            }
        }
        pthread_mutex_unlock(dataP->mutexPocetHribov);
    }
    printf("Skoncilo sa generovanie kolacov\n");
    pthread_exit(NULL);
}

void zmenaSmeru(HRAC_DATA * pHrac, char smer) {
    switch (smer) {
        case 'w':
            if (pHrac->smer != 'd') {
                pHrac->smer = 'u';
            }
            break;
        case 's':
            if (pHrac->smer != 'u') {
                pHrac->smer = 'd';
            }
            break;
        case 'a':
            if (pHrac->smer != 'r') {
                pHrac->smer = 'l';
            }
            break;
        case 'd':
            if (pHrac->smer != 'l') {
                pHrac->smer = 'r';
            }
            break;
        default:
            break;
    }
}

void * nacitanieSmeruHracF1(void * data){
    HRACIE_POLE_DATA * dataPole = data;
    printf("Vlakno pre nacitanie smeru hraca X zacalo\n");
    char buffer[256];
    while (!dataPole->hraSkoncila) {
        bzero(buffer,256);
        read(dataPole->hrac1->socket, buffer, 255);
        printf("Hrac X zadal smer:%s\n", buffer);
        zmenaSmeru(dataPole->hrac1,buffer[0]);
    }
    printf("Vlakno pre nacitanie smeru hraca X skoncilo\n");
    pthread_exit(NULL);
}

void * nacitanieSmeruHracF2(void * data){
    HRACIE_POLE_DATA * dataPole = data;
    printf("Vlakno pre nacitanie smeru hraca O zacalo\n");
    char buffer[256];
    while (!dataPole->hraSkoncila) {
        bzero(buffer,256);
        read(dataPole->hrac2->socket, buffer, 255);
        printf("Hrac O zadal smer:%s\n", buffer);
        zmenaSmeru(dataPole->hrac2,buffer[0]);
    }
    printf("Vlakno pre nacitanie smeru hraca O skoncilo\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int sockfd, newsockfd, newsockfd2;
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
    //Hrac1
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }
    printf("Hrac X sa napojil, caka sa na napojenie druheho hraca\n");
    bzero(buffer,256);
    sprintf(buffer,"Tvoj had je X, pockaj kym sa napoji hrac O\n");
    n = write(newsockfd,buffer, 255);
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }
    //Hrac2
    newsockfd2 = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }
    printf("Hrac O sa napojil hra zacala\n");
    bzero(buffer,256);
    sprintf(buffer,"Tvoj had je O\n");
    n = write(newsockfd2,buffer, 255);
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }
    //Poslenie info o zacati hry
    bzero(buffer,256);
    sprintf(buffer,"Hra zacne o 10 sekund, priprav sa!\n");
    n = write(newsockfd,buffer, 255);
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }
    n = write(newsockfd2,buffer, 255);
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }
    sleep(10);
    // VYTVORENIE HRACEJ PLOCHY
    HRACIE_POLE_DATA hraciePoleData;
    hraciePoleData.stavHry = 0;
    hraciePoleData.hraSkoncila = false;
    hraciePoleData.pocetHribov = 0;
    pthread_mutex_t mutexPocetHribov = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t pridajHrib = PTHREAD_COND_INITIALIZER;
    hraciePoleData.mutexPocetHribov = &mutexPocetHribov;
    hraciePoleData.pridajHrib = &pridajHrib;
    for (int i = 0; i < HRACIA_PLOCHA_VELKOST_X; ++i) {
        for (int j = 0; j < HRACIA_PLOCHA_VELKOST_Y; ++j) {
            hraciePoleData.pole[i][j] = ' ';
        }
    }
    // VYTVORENIE HRACA1
    HRAC_DATA hrac1;
    hrac1.velkostHada = 10;
    hrac1.smer = 'r';
    hrac1.id = 'X';
    hrac1.socket = newsockfd;
    int i = 0;
    for (int y= hrac1.velkostHada - 1; y >= 0; --y) {
        hrac1.clanky_hada[i].poziciaX = 0;
        hrac1.clanky_hada[i].poziciaY = y;
        i++;
    }
    for (int j = 0; j < hrac1.velkostHada; ++j) {
        hraciePoleData.pole[hrac1.clanky_hada[j].poziciaX]
            [hrac1.clanky_hada[j].poziciaY] = 'X';
    }
    // VYTVORENIE HRACA2
    HRAC_DATA hrac2;
    hrac2.velkostHada = 10;
    hrac2.smer = 'l';
    hrac2.id = 'O';
    hrac2.socket = newsockfd2;
    i = 0;
    for (int y= hrac2.velkostHada - 1; y >= 0; --y) {
        hrac2.clanky_hada[i].poziciaX = HRACIA_PLOCHA_VELKOST_X - 1;
        hrac2.clanky_hada[i].poziciaY = HRACIA_PLOCHA_VELKOST_Y - y - 1;
        i++;
    }
    for (int j = 0; j < hrac1.velkostHada; ++j) {
        hraciePoleData.pole[hrac2.clanky_hada[j].poziciaX]
        [hrac2.clanky_hada[j].poziciaY] = 'O';
    }
    //Pridanie hracov do hracej plochy
    hraciePoleData.hrac1 = &hrac1;
    hraciePoleData.hrac2 = &hrac2;
    //Vytvorenie vlakna na generovanie hribov
    pthread_t generovanieHribov;
    pthread_create(&generovanieHribov,NULL,generovanieHribovF,&hraciePoleData);

    pthread_t nacitanieSmeruHrac1;
    pthread_create(&nacitanieSmeruHrac1, NULL, nacitanieSmeruHracF1, &hraciePoleData);
    pthread_t nacitanieSmeruHrac2;
    pthread_create(&nacitanieSmeruHrac2,NULL,nacitanieSmeruHracF2,&hraciePoleData);
    // ZACIATOK CYKLU
    while (!hraciePoleData.hraSkoncila) {
        //posuvanie hada
        if (hraciePoleData.stavHry == 0){
            posunClankov(hraciePoleData.hrac1, &hraciePoleData);
        }
        if (hraciePoleData.stavHry == 0){
            posunClankov(hraciePoleData.hrac2, &hraciePoleData);
        }
        //Kontrola ci hra skoncila
        if (!hraciePoleData.hraSkoncila) {
            bzero(buffer,256);
            buffer[0] = 'f';
            n = write(newsockfd,buffer, 255);
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
            n = write(newsockfd2,buffer, 255);
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
            n = write(newsockfd2,hraciePoleData.pole,sizeof (hraciePoleData.pole));
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
            n = write(newsockfd2,buffer, 255);
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
        }
        usleep(125000);
        //sleep(5);
    }
    // VYHODNOTENIE HRY
    if ( hraciePoleData.stavHry == 1) {
        bzero(buffer,256);
        sprintf(buffer,"Vyhral hrac X!\nDlzka hada hraca X:%d\nDlzka hada hraca O:%d\nPre skoncenie hry stlacte q\n",hrac1.velkostHada,hrac2.velkostHada);
        n = write(newsockfd,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
        n = write(newsockfd2,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
    }
    if ( hraciePoleData.stavHry == 2) {
        bzero(buffer,256);
        sprintf(buffer,"Vyhral hrac O!\nDlzka hada hraca X:%d\nDlzka hada hraca O:%d\nPre skoncenie hry stlacte q\n",hrac1.velkostHada,hrac2.velkostHada);
        n = write(newsockfd,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
        n = write(newsockfd2,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
    }

    if ( hraciePoleData.stavHry == 3) {
        bzero(buffer,256);
        sprintf(buffer,"Nevyhral nikto, remiza!\nDlzka hada hraca X:%d\nDlzka hada hraca O:%d\nPre skoncenie hry stlacte q\n",hrac1.velkostHada,hrac2.velkostHada);
        n = write(newsockfd,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
        n = write(newsockfd2,buffer, 255);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
    }

    pthread_mutex_destroy(&mutexPocetHribov);
    pthread_cond_destroy(&pridajHrib);
    pthread_join(generovanieHribov,NULL);
    pthread_join(nacitanieSmeruHrac1,NULL);
    pthread_join(nacitanieSmeruHrac2,NULL);
    close(newsockfd);
    close(newsockfd2);
    close(sockfd);

    return 0;
}
