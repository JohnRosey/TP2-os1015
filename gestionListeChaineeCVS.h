//#########################################################
//#
//# Titre : 	Utilitaires Liste Chainee et CVS LINUX Automne 11
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#
//# Auteur : 	Ulrich YOUGBARE
//#	Date :		Decembre 2020
//#
//# Langage : 	ANSI C on LINUX (Debian)
//#
//#######################################




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CLIENT_FIFO_NAME "./tmp/FIFO%d"
#define FIFO_TRANSACTIONS "./tmp/FIFO_TRANSACTIONS"

struct Info_FIFO_Transaction {
    int pid_client;
    char transaction[400];
};

struct infoligne{						
	int		noligne;				
	char	ptrligne[100];							
	};								 

struct noeud{			
	struct infoligne	ligne;
	sem_t 			sem;
	struct noeud		*suivant;	
	};

struct infoADD{	
    int noligne;
	char tligne[100];
	};

struct infoMODIFY{
	int noligne;
	char tligne[100];
	};
	
struct infoREMOVE{						
	int noligne;
	};

struct infoLIST{						
	int start;
	int end;
	int pid_client;
	};
	
struct infoSAVE{
	char nomFichier[100];
	};
struct infoLOAD{
	char nomFichier[100];
	};
struct infoEXE{
	char nomFichier[100];
	int pid_client;
	};
		
	
void cls(void);
void error(const int exitcode, const char * message);

struct noeud * findItem(const int no);
struct noeud * findPrev(const int no);

void addItemNONCONCURRENT(const int nl, const char* tl);
void writeOnClientFifo(int pid_client, char message[400]);
void addItem(struct infoADD* param);
void removeItem(struct infoREMOVE* param);
void modifyItem(struct infoMODIFY* param);
void listItems(struct infoLIST* param);
void saveItems(struct infoSAVE* param);
void executeFile(struct infoEXE* param);

void loadFich(const char* sourcefname);
void readTrans();
