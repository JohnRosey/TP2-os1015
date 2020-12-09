gestionCVS_MAIN: gestionCVS_MAIN.o gestionListeChaineeCVS.o gestionCVS.o
	gcc -o gestionCVS_MAIN gestionCVS_MAIN.o gestionListeChaineeCVS.o gestionCVS.o -lpthread
	gcc -o client client.c -lncurses -lpthread
gestionCVS_MAIN.o: gestionCVS_MAIN.c gestionListeChaineeCVS.h 
	gcc -c gestionCVS_MAIN.c -Wall -I. -lpthread
gestionListeChaineeCVS.o: gestionListeChaineeCVS.c gestionListeChaineeCVS.h
	gcc -c gestionListeChaineeCVS.c -Wall -I. -lpthread
gestionCVS.o: gestionCVS.c gestionListeChaineeCVS.h
	gcc -c gestionCVS.c -Wall -I. -lpthread

