#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>

/********** Exemple d'utilisation de pselect avec le signal SIGALRM **********/

//variable globale pour notifier la capture d'un signal
volatile sig_atomic_t print_flag = false;


//Gestionnaire de signal, est-ce qu'on met ici la gestion de la table de voisin ? 
void handle_alarm(int sig) {
  print_flag = true;
  char *message = "signal recu\n";
  write(STDOUT_FILENO, message, strlen(message));
  printf("\n");
}



void affichage(int sig){
  char *message = "signal recu\n";
  write(STDOUT_FILENO, message, strlen(message));
}

int main(void){


  sigset_t zeromask, sigset;

  
  struct sigaction action;
  
  memset(&action, 0, sizeof(struct sigaction));

  //sa_handler = pointeur vers la fonction traitant le signal
  action.sa_handler = handle_alarm;
  //Sigaction spécifie le traitement effectué ou à effectuer à réception d'un signal donné. 
  //renvoie 0 si l'appel s'est bien déroulé. 
  sigaction(SIGALRM , &action, NULL);

  //Initialisation d'un signal (retourne toujours 0)
  sigemptyset(&sigset);   
  sigemptyset(&zeromask);         
  
  sigaddset(&sigset, SIGALRM);
  //On bloque le signal SIGALRM
  sigprocmask(SIG_BLOCK, &sigset, NULL);


  alarm(5);

  while(1){

   if(print_flag){
    print_flag = false;
    alarm(5);
   }

  /**

  La raison pour laquelle pselect() est nécessaire est que si l'on veut attendre soit un signal, 
  soit qu'un descripteur de fichier soit prêt, alors un test atomique est nécessaire 
  pour éviter les situations de concurrence. (Pas vraiment compris ??)
  (Supposons que le gestionnaire de signaux active un drapeau global et revienne. 
  Alors un test de ce drapeau, suivi d'un appel select() peut bloquer indéfiniment 
  si le signal arrive juste après le test mais avant l'appel. 
  À l'inverse, pselect() permet de bloquer le signal d'abord, 
  traiter les signaux déjà reçus, puis invoquer pselect() avec le sigmask, désiré, 
  en évitant la situation de blocage.)  

  **/

   // pselect = unblock signal, then wait for signal or ready file descriptor 
   //Quand on appelle pselect, le signal SIGALRM va être débloqué, zeromask est un signal vide
   //si le signal débloqué était en attente,pselect retourne immédiatement -1 avec errno valant EINTR.

   //zeromask is a pointer to a signal mask (see sigprocmask(2)); if it is not NULL, then pselect() first replaces the current signal mask by the one pointed to by zeromask, 
   //then does the "select" function, and then restores the original signal mask.

   //when pselect is called, it replaces the signal mask of the process with an empty set (zeromask)
   //and then check the descriptors, possibly going to sleep. But when pselect returns, the signal
   //mask of the process is reset to its value before pselect was called (SIGALRM blockedEINTR) 

   /** 
    When pselect return [EINTR] :
    The function was interrupted before any of the selected events occurred and before the timeout interval expired. 
   **/

   // pselect with no file descriptors and no timeout
  int psel=pselect(0, NULL, NULL, NULL, NULL, &zeromask);
     

    //pause();
  }
}
