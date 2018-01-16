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
#include <assert.h>

int main(int argc, char **argv)
{
    char *pid_string = malloc(10*sizeof(char));
    char *port_string= malloc(10*sizeof(char));
    char *machine_name = malloc(SIZE);
    int i = 0;
    int new_argc;
    struct sockaddr_in csin;
    struct sockaddr_in sin;
    int port2 = 0;

    //char buffer[SIZE];
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */



   new_argc = argc - 3;
    char *new_argv[new_argc + 1];
    while (i<new_argc)
    {
      new_argv[i] = argv[i+3];
      i++;
    }
    new_argv[new_argc+1] = NULL;

    printf("%s %s %s %s\n",new_argv[0], new_argv[1], new_argv[2], new_argv[3]);




   fprintf(stdout,"======================= coucou\n");
   fflush(stdout);

   /* creation d'une socket pour se connecter au */
   memset(&csin, '\0',sizeof(csin));
   csin.sin_family = AF_INET;
   csin.sin_port = htons(atoi(argv[1]));
   int port = csin.sin_port ;

   /*csin.sin_addr = argv[2];*/
   struct hostent* res;
   //struct in_addr addr;


   fprintf(stdout,"================Name  %s\n",argv[2]);

   res=gethostbyname(argv[2]);

   fprintf(stdout,"================ res : %p\n",res);
   fflush(stdout);

   //addr=(struct in_addr *) res->h_addr_list[0];
   //csin.sin_addr=*addr;

   //fprintf(stdout,"================== ADDR %s : %p\n",res->h_addr_list[0],&res->h_addr_list[0]);

    assert(res->h_addr_list[0]);

   memcpy(&csin.sin_addr,res->h_addr_list[0],res->h_length);
   //fprintf(stdout,"================== addr : %s\n",csin.sin_addr);
   //fflush(stdout);

   int sockc = creer_socket(&port,csin);

   //printf("Ancien numéro de port : %d\n", port);
   //fflush(stdout);
   //Connexion de a la socket
   do_connect(sockc, csin);
   printf("Connected to the server ! \n");
   fflush(stdout);



   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

   /* Envoi du nom de machine au lanceur */
   gethostname(machine_name, SIZE);
   machine_name[strlen(machine_name)] = '\n';

   write(sockc,machine_name,strlen(machine_name));
   printf("le nom de la machine envoyé est %s\n",machine_name);


   /* Envoi du pid au lanceur */

    int pid = getpid();
    sprintf(pid_string, "%d\n", pid);
    printf("le pid envoyé:%d\n", pid);
    write(sockc,pid_string,sizeof(pid_string));

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   memset(&sin, '\0',sizeof(sin));
   sin.sin_family = AF_INET;
   memcpy(&sin.sin_addr,res->h_addr_list[0],res->h_length);


   int sockfd = creer_socket(&port2,sin);

   printf(" Nouveau numéro de port : %d\n",port2);
   fflush(stdout);
   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
  sprintf(port_string, "%d\n", port2);
  write(sockc, port_string, sizeof(port_string));



  printf("execution de la bonne commande\n");

   /* on execute la bonne commande */

   execvp(new_argv[0], new_argv);
   ERROR_EXIT("execvp");

   return 0;
}
