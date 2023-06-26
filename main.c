#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>

#define SEM_KEY 1234
#define SHM_KEY 5678
#define MSG_KEY 9012

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <nombre de caisses> <nombre de places>\n", argv[0]);
        return 1;
    }

    int nombre_caisses = atoi(argv[1]);
    int nombre_places = atoi(argv[2]);


    int shmid = shmget(SHM_KEY, sizeof(int), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }


    int *nombre_places_restantes = shmat(shmid, NULL, 0);
    if (nombre_places_restantes == (int *)(-1)) {
        perror("shmat");
        exit(1);
    }

    *nombre_places_restantes = nombre_places;


    int semid = semget(SEM_KEY, 1, 0644 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }


    union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } argument;
    argument.val = 1;

    if (semctl(semid, 0, SETVAL, argument) == -1) {
        perror("semctl");
        exit(1);
    }


    int msgid = msgget(MSG_KEY, 0644 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }


    for (int i = 0; i < nombre_caisses; ++i) {
        if (fork() == 0) {
            execl("./caisse", "caisse", NULL);
            exit(0);
        }
    }


    if (fork() == 0) {
        execl("./afficheur", "afficheur", NULL);
        exit(0);
    }

    for (int i = 0; i < nombre_caisses + 1; ++i) {
        wait(NULL);
    }

    printf("Toutes les caisses sont fermées. Afficheur arrêté\n");

    if (shmdt(nombre_places_restantes) == -1) {
        perror("shmdt");
        exit(1);
    }


    if (shmctl(shmid, IPC_RMID, NULL) == -1 || semctl(semid, 0, IPC_RMID, argument) == -1 || msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("destroy");
        exit(1);
    }

    return 0;
}
