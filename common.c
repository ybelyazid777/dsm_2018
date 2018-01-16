#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<netdb.h>
#include <arpa/inet.h>
#include <errno.h>


#define SOCKET_ERROR -1
#define BIND_ERROR -1
#define INVALID_SOCKET -1

int creer_socket( int* port_num, struct sockaddr_in sin)
{
   int fd = 0;
   socklen_t size = sizeof(sin);

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   fd = socket( AF_INET, SOCK_STREAM, 0 );             //Créé la socket
    if (fd == INVALID_SOCKET){                          //Verification de la validité de la socket
      perror( "socket" );
      exit(EXIT_FAILURE);
    }

    memset(&sin, '\0',sizeof(sin));
    sin.sin_addr.s_addr = htonl(INADDR_ANY);              //Contexte d'adressage serveur
    sin.sin_family = AF_INET;


    do_bind(fd, sin);

    /*Modifie le numéro de port*/
    getsockname(fd,(struct sockaddr * )&sin, &size);
    //sin.sin_port = ntohs(sin.sin_port);
    *port_num = ntohs(sin.sin_port);

    

     /* renvoie le numero de descripteur */
     return fd;
  }

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */

void do_bind(int fd, struct sockaddr_in sin)
{
  int bind_err = bind(fd , (struct sockaddr *)&sin, sizeof(sin));     //Liaison de la socket serveur avec la structure sin
  if (bind_err == BIND_ERROR)
    {                                                                 //Verification du bind
      perror ("bind");
      exit (EXIT_FAILURE);
    }
}

void do_listen(int fd, int nbr_ecoute)
{
  int listen_err = listen(fd, nbr_ecoute);
  if (listen_err == SOCKET_ERROR)
    {
      perror("listen");
      exit (EXIT_FAILURE);
    }
}

void do_connect(int fd, struct sockaddr_in csin)
{

  fprintf(stdout,"================= fd : %i\n",fd);

  int connect_err = connect(fd, (struct sockaddr *) & csin, sizeof(csin));
  if (connect_err == -1)
  {
    fprintf(stdout,"=============== %i\n",errno);

    perror("ERROR address");
    exit(EXIT_FAILURE);
  }
}
 ssize_t do_write(int sockfd, const void* buffer, size_t len)
{
    ssize_t msg_sent = write(sockfd, buffer,len);

    if (msg_sent < 0)
    {
        perror("ERROR : write failed\n");
        return 1;
    }

    return msg_sent;
}
