
//#########################################################
//#
//# Titre : 	CODE DU CLIENT (MAIN) TP1 LINUX Automne 19
//#				VERSION CONCURRENTE 
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Ulrich YOUGBARE
//#	Date :		Decembre 2020
//#
//# Langage : 	ANSI C on LINUX (Debian)
//#
//#######################################


#include "gestionListeChaineeCVS.h"
#include <ctype.h>

#define CMD_LEN 512
#define WIN_WIDTH 50
#define WIN_HEIGHT 40

sem_t semConsole;

void openReceptionWindow(){
    WINDOW *recep_window_ptr;
    int lastLine = 3;

    int client_fifo_fd;
    int read_res;
    struct Info_FIFO_Transaction my_data;
    struct Info_FIFO_Transaction reponseServer;
    char client_fifo[256];

    // on crée notre propre fifo sur laquelle on va écouter !
    sprintf(client_fifo, CLIENT_FIFO_NAME, getpid());
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "une erreur est survenue lors de la création du fifo. %s\n", client_fifo);
        exit(EXIT_FAILURE);
    }
    client_fifo_fd = open(client_fifo, O_RDONLY);
    if (client_fifo_fd == -1) {
        fprintf(stderr, "une erreur est survenue lors de l'ouverture du fifo. n");
        exit(EXIT_FAILURE);
    }

    // on peut donc commencer a dessiner la fenetre ... 
    sem_wait(&semConsole);
    recep_window_ptr = newwin(WIN_HEIGHT, WIN_WIDTH*2, 0, WIN_WIDTH+1);
    box(recep_window_ptr, '|', '-');
    attrset(COLOR_PAIR(2) | A_BOLD);
    mvwprintw(recep_window_ptr, 1, 1, "%s", "RECEPTION");
    attrset(COLOR_PAIR(1));
    attroff(A_BOLD);
    sem_post(&semConsole);

    do {
        read_res = read(client_fifo_fd, &reponseServer, sizeof(reponseServer));
        if (read_res > 0) {
            // On bloque le terminal pour y écrire ce que repond le serveur
            sem_wait(&semConsole);
            attrset(COLOR_PAIR(3));
            mvwprintw(recep_window_ptr, lastLine, 1, "> : %s", reponseServer.transaction);
            attrset(COLOR_PAIR(1));
            wrefresh(recep_window_ptr);
            sem_post(&semConsole);
            lastLine ++;

        }
    } while (read_res > 0);

    endwin();
    close(client_fifo_fd);
    unlink(client_fifo);
}

void openTransmissionWindow(){
    WINDOW *trans_window_ptr;
    char clientCmd[CMD_LEN];
    char message[CMD_LEN];
    int lastLine = 3;

    int server_fifo_fd;
    struct Info_FIFO_Transaction my_data;
    char client_fifo[256];

    server_fifo_fd = open(FIFO_TRANSACTIONS, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Veuillez d'abord lancer le serveur (gestionCVS_MAIN) !\n");
        exit(EXIT_FAILURE);
    }

    my_data.pid_client = getpid();

    sem_wait(&semConsole);

    trans_window_ptr = newwin(WIN_HEIGHT, WIN_WIDTH, 0, 0);
    box(trans_window_ptr, '|', '-');

    attrset(COLOR_PAIR(2) | A_BOLD);
    mvwprintw(trans_window_ptr, 1, 1, "%s", "TRANSMISSION");
    attrset(COLOR_PAIR(1));
    attroff(A_BOLD);
    mvwprintw(trans_window_ptr, lastLine, 1, "%s", "$ : ");
    wrefresh(trans_window_ptr);
    wgetstr(trans_window_ptr,clientCmd);
    wrefresh(trans_window_ptr);
    sem_post(&semConsole);

    while(strcmp(clientCmd,"quit")!=0){

        // On envoit le message au serveur
        sprintf(my_data.transaction, "%s", clientCmd); 
        write(server_fifo_fd, &my_data, sizeof(my_data));
        
        //une fois le message envoyé, on re-affiche le "prompt" client
        lastLine +=2;
        sem_wait(&semConsole);
        mvwprintw(trans_window_ptr, lastLine, 1, "%s", "$ : ");
        wgetstr(trans_window_ptr,clientCmd);
        wrefresh(trans_window_ptr);
        sem_post(&semConsole);
    }
    
    close(server_fifo_fd);
    unlink(client_fifo);
    endwin();




}

int main() {
    pthread_t tid[2];
    sem_init(&semConsole, 0, 1);
    initscr();

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Erreur - Votre terminal ne supporte pas l'affichage des couleurs.\n");
        exit(1);
    }

    start_color();

    init_pair (0, COLOR_WHITE, COLOR_BLACK);
  init_pair (1, COLOR_CYAN, COLOR_BLACK);
  init_pair (2, COLOR_WHITE, COLOR_BLUE);
  init_pair (3, COLOR_WHITE, COLOR_BLACK);
  init_pair (4, COLOR_WHITE, COLOR_RED);
  init_pair (5, COLOR_BLACK, COLOR_GREEN);

    pthread_create(&tid[0], NULL, (void *)openReceptionWindow, NULL);
    pthread_create(&tid[1], NULL, (void *)openTransmissionWindow, NULL);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    endwin();

    sem_destroy(&semConsole);

    exit(EXIT_SUCCESS);
}
