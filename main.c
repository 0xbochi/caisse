#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int nombre_places_restantes;

void* caisse(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        if (nombre_places_restantes <= 0) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }

        int tickets_vendus = (rand() % 7) + 1;

        if (nombre_places_restantes - tickets_vendus >= 0) {
            nombre_places_restantes -= tickets_vendus;
            printf("Caisse %d: %d billets vendus, %d places restantes\n", *((int*) arg), tickets_vendus, nombre_places_restantes);
        }

        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3 + 1);
    }
}

void* afficheur(void* arg) {
    while (1) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        printf("Afficheur: %d places restantes\n", nombre_places_restantes);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <nombre de caisses> <titre du film> <nombre de places>\n", argv[0]);
        return 1;
    }

    int nombre_caisses = atoi(argv[1]);
    char* titre_film = argv[2];
    nombre_places_restantes = atoi(argv[3]);

    printf("Film: %s, %d places disponibles\n", titre_film, nombre_places_restantes);

    pthread_t threads[nombre_caisses + 1];
    int caisse_ids[nombre_caisses];

    srand(time(NULL));

    for (int i = 0; i < nombre_caisses; ++i) {
        caisse_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, caisse, &caisse_ids[i]);
    }

    pthread_create(&threads[nombre_caisses], NULL, afficheur, NULL);

    for (int i = 0; i < nombre_caisses; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Toutes les caisses sont fermées. Afficheur actif\n");

    return 0;
}
