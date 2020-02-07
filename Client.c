#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 
* Cliente UDP
 */
int main(int argc, char *argv[]){

   int s;
	char comando[200];
   unsigned short port;
   struct sockaddr_in server;
   char buf[200];
char answer[2000];

   /* 
    * O primeiro argumento (argv[1]) � o endere�o IP do servidor.
    * O segundo argumento (argv[2]) � a porta do servidor.
    */
   if(argc != 3)
   {
      printf("Use: %s IP porta\n",argv[0]);
      exit(1);
   }
  port = htons(atoi(argv[2]));

   /*
    * Cria um socket UDP (dgram).
    */
   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
       perror("socket()");
       exit(1);
   }

   /* Define o endere�o IP e a porta do servidor */
   server.sin_family      = AF_INET;            /* Tipo do endere�o         */
   server.sin_port        = port;               /* Porta do servidor        */
   server.sin_addr.s_addr = inet_addr(argv[1]); /* Endere�o IP do servidor  */
	
	
if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
       perror("bind()");
       exit(1);
   }

do{
	printf(">");
	__fpurge(stdin);
	scanf("%s",comando);
	
   strcpy(buf,comando);

   /* Envia a mensagem no buffer para o servidor */
   if (sendto(s, buf, (strlen(buf)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
       perror("sendto()");
       exit(2);
   }

   if(recvfrom(s, answer, sizeof(answer), 0, NULL,0) <0)
   {
       perror("recvfrom()");
       exit(1);
   }
	  printf("\n %s",answer);



	}while(strcmp(comando,"exit") != 0 );

   /* Fecha o socket */
   close(s);
	return 0;
}
