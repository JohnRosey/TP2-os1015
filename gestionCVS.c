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

#include "gestionListeChaineeCVS.h"

//Pointeur de tête de liste
extern struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
extern struct noeud* queue;

extern int nbThreadAMLSO;

extern sem_t semH;
extern sem_t semQ;
extern sem_t semConsole;
extern sem_t semNBThreadAMLSO;

//#######################################
//#
//# Affiche une série de retour de ligne pour "nettoyer" la console
//#
void cls(void){
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

//#######################################
//#
//# Affiche un messgae et quitte le programme
//#
void error(const int exitcode, const char * message){
	printf("\n-------------------------\n%s\n",message);
	exit(exitcode);
	}

//#######################################
//#
//# Chargement de la BD binaire
//#
void loadFich(const char* sourcefname){
	
	//Initialisation des pointeurs
	head = NULL;
	queue = NULL;


	const char tligne[100];
	int noligne=0;

	//Ouverture du fichier source en mode "rt" : [r]ead [t]ext
	FILE *f;
	f = fopen(sourcefname, "rt");
	if (f==NULL)
		return;

	//Ajout des éléments extraits du fichier source
	
	while(!feof(f)){
		fgets(tligne, 100, f);
		addItemNONCONCURRENT(noligne++, tligne);
		}

 }

//#######################################
//#
//# Enregistre le contenu de la liste chaînée dans un fichier texte
//#
void saveItems(struct infoSAVE* param){
	
	char sourcefname[100];
	struct noeud * ptr = NULL;
	
	strcpy(sourcefname,(const char*)param->nomFichier);
	free(param);
	
	sem_wait(&semNBThreadAMLSO);
	    nbThreadAMLSO++;
	sem_post(&semNBThreadAMLSO);
	
	sem_wait(&semH);
	sem_wait(&semQ);

	if(head != NULL)
		ptr = head;	// premier element
	else{ // liste vide 
		sem_wait(&semNBThreadAMLSO);
		    nbThreadAMLSO--;
		sem_post(&semNBThreadAMLSO);
		sem_post(&semQ);
		sem_post(&semH);
		return;	
	}
	FILE *f;
	char nomC[255];
	char nomTXT[255];
	char command[255];
	char *ptrC;

	strcpy(nomC,sourcefname);
	strcpy(nomTXT, sourcefname);
	ptrC = strrchr(nomTXT,'.');
	strcpy(ptrC,".txt");
	ptrC = strrchr(nomC,'.');
	strcpy(ptrC,".c");

	//Ouverture du fichier en mode "wt" : [w]rite [t]ext

 	f= fopen(nomTXT, "wt");

	
	if (f==NULL)
		error(2, "saveItems: Erreur lors de l'ouverture du fichier pour écriture en mode texte.");

	while (ptr!=NULL){

		//Écriture des données
		fprintf(f,"%s\n",ptr->ligne.ptrligne);
		
		//Déplacement du pointeur
		ptr = ptr->suivant;
		}
	
	//Fermeture du fichier
	fclose(f);

	// Passage de .txt a .c
	sprintf(command,"mv %s %s",nomTXT,nomC);
	system(command);
	
	sem_wait(&semNBThreadAMLSO);
	    nbThreadAMLSO--;
	sem_post(&semNBThreadAMLSO);
	sem_post(&semQ);
	sem_post(&semH);
	
}

//#######################################
//#
//# Execute le fichier source .c , et envoit le retour au client.
//#
void executeFile(struct infoEXE* param){

	char nomFichier[100];
	int pid_client;

	strcpy(nomFichier,(const char*)param->nomFichier);
	pid_client = param->pid_client;

	free(param);

	char command[100];
	char nameC[100];
	char buffer[128];

	FILE *f, *commandOutput;

	//Ouverture du fichier MakeCVS en mode "wt" : [w]rite [t]ext
	f = fopen("MakeCVS", "wt");
	if (f==NULL){
		error(2, "ExecuteFile: Erreur lors de l'ouverture du fichier pour écriture en mode texte.");
	}
	
	strcpy(nameC,nomFichier);
	fprintf(f,"fichCVSEXE: %s\n",nomFichier);
	fprintf(f,"\tgcc -o fichCVSEXE %s\n",nomFichier);

	//Fermeture du fichier
	fclose(f);

	// make du fichier MakeCVS
	sprintf(command, "make -f MakeCVS");
	system(command);

	// execution du fichier compilé fichCVSEXE
	sprintf(command, "./fichCVSEXE");
	
	// on va récupérer l'output de la commande et le renvoyer au client
	commandOutput = popen(command,"r");
    if (!commandOutput) {
        fprintf(stderr, "Erreur lors de la récupération du résultat d'exécution.\n");
        //exit(EXIT_FAILURE);
    }
	while (fgets(buffer, sizeof buffer, commandOutput) != NULL) {
        writeOnClientFifo(pid_client, buffer);
    }

	system(command);
	


}


//#######################################
//#
//# fonction utilisée par les threads de transactions
//#
//void readTrans(char* nomFichier){
void readTrans(){
	pthread_t tid[1000];
	int i, nbThread = 0;
	char *tok, *sp;

	int server_fifo_fd;
    struct Info_FIFO_Transaction transactionClient;
    int read_res;
    char client_fifo[256];


    // création de la fifo ...
    int serverFifo = mkfifo(FIFO_TRANSACTIONS, 0777);
    if(serverFifo !=0){
        fprintf(stderr, "Une erreur est survenue lors de la création de la fifo. Si elle existe déjà, supprimez-la et réessayez. \nSinon, Merci de vérifier vos droits d'accès.\n");
        exit(EXIT_FAILURE);
    }

	// ouverture de la fifo
    server_fifo_fd = open(FIFO_TRANSACTIONS, O_RDONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Une erreur est survenue lors de l'ouverture de la fifo. Merci de vérifier vos droits d'accès.\n");
        exit(EXIT_FAILURE);
    }


	do {
		read_res = read(server_fifo_fd, &transactionClient, sizeof(transactionClient));
        if (read_res > 0) {

            //  lecture dune structure de type info_FIFO_Transaction
            printf("PID Client : %d \n ", transactionClient.pid_client);
            printf("Transaction du client : %s \n\n ", transactionClient.transaction);
            sprintf(client_fifo, CLIENT_FIFO_NAME, transactionClient.pid_client);
			
			//Extraction du type de transaction
			tok = strtok_r(transactionClient.transaction, " ", &sp);


            //Branchement selon le type de transaction
			switch(tok[0]){
				case 'A':
				case 'a':{
					//Extraction des paramètres
					int noligne = atoi(strtok_r(NULL, " ", &sp));
					char *tligne = strtok_r(NULL, "\n", &sp);
					
					struct infoADD *ptr = (struct infoADD*) malloc(sizeof(struct infoADD));
					ptr->noligne = noligne;
					strcpy(ptr->tligne,(const char *)tligne);
					
					//Appel de la fonction associée
					//printf("\nDANS READTRANS nl = %d tl = %s ", noligne, tligne);
					pthread_create(&tid[nbThread++], NULL, (void *)addItem, ptr);
					break;
					}
				case 'E':
				case 'e':{
					//Extraction du paramètre
					int noligne = atoi(strtok_r(NULL, " ", &sp));
					
					struct infoREMOVE *ptr = (struct infoREMOVE*) malloc(sizeof(struct infoREMOVE));
					ptr->noligne = noligne;
					
					//Appel de la fonction associee
					pthread_create(&tid[nbThread++], NULL, (void *)removeItem, ptr);
					break;
					}
				case 'M':
				case 'm':{
					//Extraction des paramètres
					int noligne = atoi(strtok_r(NULL, " ", &sp));
					char *tligne = strtok_r(NULL, "\n", &sp);
					
					struct infoMODIFY *ptr = (struct infoMODIFY*) malloc(sizeof(struct infoMODIFY));
					ptr->noligne = noligne;
					strcpy(ptr->tligne,(const char *)tligne);
					
					//Appel de la fonction associee
					pthread_create(&tid[nbThread++], NULL, (void *)modifyItem, ptr);
					break;
					}
				case 'L':
				case 'l':{
					//Extraction des paramètres
					int nstart = atoi(strtok_r(NULL, "-", &sp));
					int nend = atoi(strtok_r(NULL, " ", &sp));
					
					struct infoLIST *ptr = (struct infoLIST*) malloc(sizeof(struct infoLIST));
					ptr->start = nstart;
					ptr->end = nend;
					ptr->pid_client = transactionClient.pid_client;
					//Appel de la fonction associee
					pthread_create(&tid[nbThread++], NULL, (void *)listItems, ptr);
					break;
					}
				case 'S':
				case 's':{
					//Appel de la fonction associée
					char *nomfich = strtok_r(NULL, " ", &sp);
					
					struct infoSAVE *ptr = (struct infoSAVE*) malloc(sizeof(struct infoSAVE));
					strcpy(ptr->nomFichier,(const char *)nomfich);

					//Appel de la fonction associee
					pthread_create(&tid[nbThread++], NULL, (void *)saveItems, ptr);
					break;
					}
				case 'X':
				case 'x':{
					//Appel de la fonction associée
					char *nomfich = strtok_r(NULL, " ", &sp);
					
					struct infoEXE *ptr = (struct infoEXE*) malloc(sizeof(struct infoEXE));
					strcpy(ptr->nomFichier,(const char *)nomfich);
					ptr->pid_client = transactionClient.pid_client;

					//Appel de la fonction associee
					pthread_create(&tid[nbThread++], NULL, (void *)executeFile, ptr);
					break;
					}
				default:{
					//Au cas où la commande ne fonctionne pas
					writeOnClientFifo(transactionClient.pid_client,"Commande inconnue.");
					}
			}
        }
	} while(read_res > 0);
	
	for(i=0; i<nbThread;i++){
		pthread_join(tid[i], NULL);
	}
}


