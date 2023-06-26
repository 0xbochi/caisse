#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define SEM_KEY 1234
#define SHM_KEY 5678
#define MSG_KEY 9012

struct message {
    long mtype;
    int tickets_vendus;
    int caisse_id;
};

void lock(int semid) {
    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void unlock(int semid) {
    struct sembuf sb = {0, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int main() {

    int shmid = shmget(SHM_KEY, sizeof(int), 0644);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }


    int *nombre_places_restantes = shmat(shmid, NULL, 0);
    if (nombre_places_restantes == (int *)(-1)) {
        perror("shmat");
        exit(1);
    }


    int semid = semget(SEM_KEY, 1, 0644);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }


    int msgid = msgget(MSG_KEY, 0644);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    while (1) {
        struct message msg;

        if (msgrcv(msgid, &msg, sizeof(int)*2, 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        lock(semid);

        printf("Afficheur: Caisse %d a vendu %d billets, %d places restantes\n", msg.caisse_id, msg.tickets_vendus, *nombre_places_restantes);

        unlock(semid);
    }


    if (shmdt(nombre_places_restantes) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
