#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Fonction appelée par un nouveau thread
void* f(void *i) {
	printf("hello\n");
	printf("%d\n", *(int*)i);
	return NULL;
}

int main() {
	int i = 3;
	//f(&i);

	 // créer un nouveau thread en lui passant une fonction à exécuter
	pthread_t id;
	int rc = pthread_create(&id, NULL, &f, &i);
	if(rc < 0) {
		perror("pthread_create");
		exit(1);
	}

	// NOTE : si le programme se termine immédiatement :
	// Pas le temps de créer le thread. 
	// Il faut au moins une instruction qui suit
	// NOTE : pour notre programme : pas de pb puisque tourne en continu

	//sleep(1); 
	printf("Task is done.\n");

}