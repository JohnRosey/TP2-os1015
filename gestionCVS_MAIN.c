//#########################################################
//#
//# Titre : 	UTILITAIRES (MAIN) TP1 LINUX Automne 19
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

//Pointeur de tête de liste
struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
struct noeud* queue;

int nbThreadAMLSO;

sem_t semH;
sem_t semQ;
sem_t semConsole;
sem_t semNBThreadAMLSO;


int main(int argc, char* argv[]){

	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;
	nbThreadAMLSO = 0;
	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	sem_init(&semConsole, 0, 1);
	sem_init(&semNBThreadAMLSO, 0, 1);


	// lancement du serveur, on affiche un message d'acceuil à la console
	printf("\n\n###############################################################################\n");
	printf(" Serveur lancé avec le pid numéro %d. En attente de connexion entrante...\n", getpid());
	printf("###############################################################################\n\n");

	// on apelle la fonction métier du programme
	readTrans();
	
	sem_destroy(&semH);
	sem_destroy(&semQ);
	sem_destroy(&semConsole);
	sem_destroy(&semNBThreadAMLSO);


	//Fin du programme
	exit( 0);
}

