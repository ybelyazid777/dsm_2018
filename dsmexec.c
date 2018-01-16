#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>



/* variables globales */


/* un tableau gerant les infos d'identification */
/* des processusET,SOCK_S dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

struct Machine
{
    char name[64];
    char port[64];
};



void usage(void)
{
  fprintf(stdout, "Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchild_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */

   // a larrivee dun signal ==> fils mort==> liberer Pcb
   fprintf(stdout,"entrée du handler" );
    if (wait(NULL) == -1) // suppression du fils zombie
    {
         ERROR_EXIT("wait handler ");
    }

}


int main(int argc, char *argv[])
{
 char *local_name = malloc(SIZE);
struct sockaddr_in sin;
struct Machine tab_machine[20];
int sockfd;

  if (argc < 3){
    usage();
  } else {
     pid_t pid;
     int num_procs = 0;
     int i;
     char str[256];

     int *port_num = NULL;
     port_num = malloc(sizeof(int)*6);



     struct sockaddr_in csin;
     struct sigaction action1;//ajouter while 1 a la fin pour que le programme continue a tourner

     /* Mise en place d'un traitant pour recuperer les fils zombies*/
     /* XXX.sa_handler = sigchld_handler; */
     action1.sa_handler=sigchild_handler;
     memset(&action1,0,sizeof(struct sigaction));
     sigaction(SIGCHLD,&action1,NULL);
     /* lecture du fichier de machines */

     int fichier;
     fichier = open("machinefile.txt", O_RDONLY);

     /* 1- on recupere le nombre de processus a lancer */

     int v;
     char c;
     v = read(fichier,&c,1);
     while((c != EOF )&& (v == 1))
     {
        if(c == '\n')
          num_procs++;
        v = read(fichier,&c,1);
      }
      printf("%d\n",num_procs);


      int tab_sock[num_procs];
      proc_array = malloc(sizeof(dsm_proc_t)*num_procs);
     /* 2- on recupere les noms des machines : le nom de */

      static const char filename[] = "machinefile.txt";
      FILE *file = fopen(filename, "r");
      if(file != NULL)
      {
        char line [64]; /* or other suitable maximum line size */
        int compteur = 0;
        while(fgets(line, sizeof line, file) != NULL) /* read a line */
        {
          strcpy(tab_machine[compteur].name,line);
          printf("Nom de la %d ème machine : %s ",compteur +1, tab_machine[compteur].name); /* write the line */
          compteur++;
        }

      }
      else
      {
        perror(filename); /* why didn't the file open? */
      }

     /* la machine est un des elements d'identification */

     /* creation de la socket d'ecoute */

     sockfd = creer_socket(port_num, sin);
     sin.sin_port = *port_num;
     printf("%d\n",sin.sin_port);

     /* + ecoute effective */

     do_listen(sockfd, num_procs);
     printf("%d\n",sin.sin_port);

     /*On stock le numéro de port*/
     char port_str[64];
     sprintf(port_str,"%d",sin.sin_port);


     /* creation des fils */
     for(i = 0; i < num_procs ; i++) {


	/* creation du tube pour rediriger stdout */

    pipe(proc_array[i].pipefd1);
    printf("Création du tube pour rediriger stdout\n");


	/* creation du tube pour rediriger stderr */

  pipe(proc_array[i].pipefd2);
  printf("Création du tube pour rediriger stderr\n");

	int pid = fork();
	if(pid == -1)
  ERROR_EXIT("fork");

	if (pid == 0) { /* fils */

      fprintf(stdout,"======================= Coucou !\n");
      fflush(stdout);

      close(proc_array[i].pipefd1[0]);
      close(proc_array[i].pipefd2[0]);

	   /* redirection stdout */
      dup2(proc_array[i].pipefd1[1], STDOUT_FILENO);
      close(STDOUT_FILENO);


	   /* redirection stderr */
     dup2(proc_array[i].pipefd2[1], STDERR_FILENO);
     close(STDERR_FILENO);
	   /* Creation du tableau d'arguments pour le ssh */
     /*deja cree , faut juste le remplir*/




     gethostname(local_name, SIZE);
     local_name[strlen(local_name)] = '\0';

     char *newargv[9];

     newargv[0] = "ssh";

     newargv[1] = strtok(tab_machine[i].name,"\n");

     newargv[2] = "~/Images/Phase1/bin/dsmwrap";

     newargv[3] = port_str;

     newargv[4] = local_name;

     //strcpy(newargv[4],port_str);

     newargv[5] = argv[2];         //prog a executer

     newargv[6] = argv[3];

     newargv[7] = argv[4];

     newargv[8] = argv[5];

     newargv[9] = NULL;

     fprintf(stdout,"jump to new prog\n");
     fflush(stdout);

	   /* jump to new prog : */
	   /* execvp("ssh",newargv); */
     execvp("ssh",newargv);//ajout etoile
     printf("================== erreur dans execvp");
     fflush(stdout);

	} else  if(pid > 0) { /* pere */


     /* fermeture des extremites des tubes non utiles */
     close(proc_array[i].pipefd1[1]);
     close(proc_array[i].pipefd2[1]);

     num_procs_creat++;


	}
     }

     for(i = 0; i < num_procs ; i++){

       /* on accepte les connexions des processus dsm */

       socklen_t taille = sizeof(csin);

       tab_sock[i] = accept(sockfd, (struct sockaddr*) & csin, &taille);
       if (tab_sock[i] == INVALID_SOCKET)
          perror("accept");

          printf("ACCEPTE\n");



	/*  On recupere le nom de la machine distante */
	/* 1- d'abord la taille de la chaine */
	/* 2- puis la chaine elle-meme */
      char *buffer4 = malloc(SIZE);
      char *machine = malloc(SIZE);
       ssize_t l = read(tab_sock[i],buffer4, SIZE);
       strncpy(machine, buffer4, l- 1);
       printf("le nom de la machine recue %s\n", machine);



	/* On recupere le pid du processus distant  */
  char *buffer3 = malloc(SIZE);
  int pid_recu;
  read(tab_sock[i],buffer3,SIZE);
  pid_recu = atoi(buffer3);
  printf(" le pid recuperé est %d\n",pid_recu);
	/* On recupere le numero de port de la socket */
	/* d'ecoute des processus distants */
  char *buffer5 = malloc(SIZE);
  int nouv_port_recu;
  read(tab_sock[i],buffer5,SIZE);

  printf("le buffer du port 2 est : %s\n", buffer5);

  nouv_port_recu = atoi(buffer5);
  printf("le nouveau port 2  recu %d\n", nouv_port_recu);





     }

     for(i = 0; i < num_procs ; i++){

     /* envoi du nombre de processus aux processus dsm*/
      char * num_procs_string= malloc(10*sizeof(char));
      sprintf(num_procs_string, "%d\n", num_procs);
      printf("le num_procs envoyé :%s\n", num_procs_string);
      write(tab_sock[i],num_procs_string,sizeof(num_procs_string));

     /* envoi des rangs aux processus dsm */
     char * rank_procs_string= malloc(10*sizeof(char));
     sprintf(rank_procs_string, "%d\n", i+1);
     printf("le rang du processus envoyé:%s\n", rank_procs_string);
     write(tab_sock[i],rank_procs_string,sizeof(rank_procs_string));


     /* envoi des infos de connexion aux processus */
}
     /* gestion des E/S : on recupere les caracteres */
     /* sur les tubes de redirection de stdout/stderr */

     fd_set readfs;
   /* while(1)
         {

            je recupere les infos sur les tubes de redirection
            jusqu'à ce qu'ils soient inactifs (ie fermes par les
            processus dsm ecrivains de l'autre cote ...)





         };
      */

     /* on attend les processus fils */

     /* on ferme les descripteurs proprement */

     /* on ferme la socket d'ecoute */
     close(sockfd);
  }
   exit(EXIT_SUCCESS);
}
