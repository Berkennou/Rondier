#include "tcp.h"
#include "utils.h"
#include <asm-generic/errno-base.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define NB_STATION 2
#define PORT 4455

static int sockfd;
static pthread_mutex_t station[NB_STATION]; // + machine mobile
static pthread_mutex_t cheked[NB_STATION - 1];
static int sock_connection[NB_STATION];
int nb_connected;
/*fonction qui est  appelé quand il y'a un exit pour liberer la memoire allouée
 * et ferme la socket*/
void monExit(void) { close(sockfd); }

/* fonction qui contient le déroulement du jeu */
void *rondier(void *args) {
  int index = NB_STATION - 1;
  int rc,fd = sock_connection[index];
  char buffer[BUF_SIZE];

  pthread_mutex_lock(station+index);
  buffer[0] = 'D';
  rc = write_all(fd,buffer,1);
  if(rc < 0)
  {
    log_error(rc,"write_all() ");
    exit(-1);
  }

  rc = read(fd,buffer,BUF_SIZE);
  if(rc < 0)
  {
    log_error(rc,"write_all() ");
    exit(-1);
  }

  fprintf(stdout,"%s\n",buffer);

  for(int i = 0;i < NB_STATION-1;++i)
  {
    pthread_mutex_unlock(station+i);
    pthread_mutex_lock(cheked+i);
    buffer[0] = 'R';
    rc = write(fd,buffer,1);
    if(rc < 0)
    {
      log_error(rc,"write_all() ");
      exit(-1);
    }

    rc = read(fd,buffer,BUF_SIZE);
    if(rc < 0)
    {
      log_error(rc,"read()");
      exit(-1);
    }
    //traitement raport
    fprintf(stdout,"Rapport : \n%s\n",buffer);
    buffer[0] = (i < NB_STATION-2)?'C':'E';
    rc = write(fd,buffer,1);
    if(rc < 0)
    {
      log_error(rc,"write_all() ");
      exit(-1);
    }
  }

  return NULL;
}
void *checkpoint(void *args) {
  int index = *(int *)args;
  int rc,fd = sock_connection[index];
  char buffer[BUF_SIZE];
  fprintf(stdout,"idex == %d\n",index);
  pthread_mutex_lock(station+index);
  buffer[0] = 'D';
  rc = write(fd,buffer,1);
  if(rc < 0)
  {
    log_error(rc,"write_all() ");
    exit(-1);
  }

  rc = read(fd,buffer,1);
  if(rc < 0)
  {
    log_error(rc,"read() ");
    exit(-1);
  }
  pthread_mutex_unlock(cheked+index);
  fprintf(stdout,"cheked\n");



  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 1 + 1) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  int rc;
  pthread_t th[NB_STATION];

  //  Gestion du signal sigpipe pour eviter le crash du serveur
  signal(SIGPIPE, SIG_IGN);
  atexit(monExit);
  const in_port_t port = atoi(argv[1]);
  sockfd = install_server(port);
  handle_error(sockfd, "install_server()");
  printf("Ecoute sur le port %d\n", port);
  
  

  while (1) {
    //  accepter une p connexion
    int fd_sock_conn = accept(sockfd,NULL,NULL);
    if (fd_sock_conn < 0) {
      log_error(fd_sock_conn, "accept()");
      continue;
    } else {
      fprintf(stdout,"CONNECTED!!\n");
      // identiffication de la station
      char c;
      rc = read_all(fd_sock_conn, &c, 1);
      if (rc < 0) {
        log_error(fd_sock_conn, "accept()");
        continue;
      } else {
        if (c == '1') {
          // chekpoint
          // Lire le numero de chekpoint
          fprintf(stdout,"CHECK POINT :)\n");
          rc = read_all(fd_sock_conn, &c, 1);
          if (rc < 0) {
            log_error(fd_sock_conn, "read_all()");
            continue;
          } else {
             fprintf(stdout,"NUMMM ==  %c \n",c);
            nb_connected++;
            sock_connection[((int)c - 49)] = fd_sock_conn;
            rc = pthread_mutex_init(station + ((int)c - 49), NULL);
            if (rc < 0) {
              perror("pthread_mutex_init");
              exit(-1);
            }
            pthread_mutex_lock(station + ((int)c - 49));
            rc = pthread_mutex_init(cheked + ((int)c - 49), NULL);
            if (rc < 0) {
              perror("pthread_mutex_init");
              exit(-1);
            }
            pthread_mutex_lock(cheked + ((int)c - 49));

            int *arg = malloc(sizeof(*arg));
            *arg = ((int)c - 49);
            rc = pthread_create(th+(*arg), NULL, checkpoint, (void *)arg);
            if (rc < 0) {
              perror("pthread create");
              exit(0);
            }
          }

        } else {
          if (c == '2') {
            nb_connected++;
            sock_connection[NB_STATION - 1] = fd_sock_conn;
            rc = pthread_mutex_init(station + NB_STATION - 1, NULL);
            if (rc < 0) {
              perror("pthread_mutex_init");
              exit(-1);
            }
            pthread_mutex_lock(station + NB_STATION - 1);

            rc = pthread_create(th+NB_STATION-1, NULL, rondier, NULL);
            if (rc < 0) {
              perror("pthread create");
              exit(0);
            }
            
          } else {
            fprintf(stderr, "Type de station non reconnu !");
          }
        }

        if (nb_connected == NB_STATION) {
          // lancer le rondier
          pthread_mutex_unlock(station + NB_STATION - 1);
          break;
        }
      }
    }
  }

  for(int i = 0 ; i < NB_STATION;++i)
    pthread_join(th[i], NULL);

  


  for(int i = 0 ; i < NB_STATION;++i)
    pthread_mutex_destroy(station+i);

  for(int i = 0 ; i< NB_STATION -1 ;++i)
    pthread_mutex_destroy(cheked+i);

  handle_error(close(sockfd), "close()");
  return 0;
}