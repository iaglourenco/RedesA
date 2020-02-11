#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Servidor UDP
 */
int main(int argc, char *argv[])
{
    int sockint, s, namelen, client_address_size;
    struct sockaddr_in client, server;
    char buf[200], answer[2000];

    if (argc != 2)
    {
        printf("Use %s porta", argv[0]);
        exit(1);
    }

    /*
    * Cria um socket UDP (dgram). 
    */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }

    /*
    * Define a qual endere�o IP e porta o servidor estar� ligado.
    * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endere�os IP
    */
    server.sin_family = AF_INET;            /* Tipo do endere�o             */
    server.sin_port = htons(atoi(argv[1])); /* Escolhe uma porta dispon�vel */
    server.sin_addr.s_addr = INADDR_ANY;    /* Endere�o IP do servidor      */

    /*
    * Liga o servidor � porta definida anteriormente.
    */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind()");
        exit(1);
    }

    /* Consulta qual porta foi utilizada. */
    namelen = sizeof(server);
    if (getsockname(s, (struct sockaddr *)&server, &namelen) < 0)
    {
        perror("getsockname()");
        exit(1);
    }

    printf("Server rodando na porta %d\n", ntohs(server.sin_port));

    do
    {
        /*
    * Recebe uma mensagem do cliente.
    * O endere�o do cliente ser� armazenado em "client".
    */
        client_address_size = sizeof(client);
        if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_address_size) < 0)
        {
            perror("recvfrom()");
            exit(1);
        }

        /*
    * Imprime a mensagem recebida, o endere�o IP do cliente
    * e a porta do cliente 
    */
	buf[strlen(buf)-1] = '\0';
        printf("================================================== \n");
        printf("Recebida o comando '%s' do endereco IP %s da porta %d\n\n", buf, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        FILE *terminal_comando = popen(buf, "r");
	fgets(answer, sizeof(answer), terminal_comando); 
	if(answer == NULL){
	 
	   
            if (sendto(s, "Erro comando não encontrado\n", strlen("Erro comando não encontrado\n"), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(2);
            }
	}
	
	else{
		while (1)
        	{
            		if (sendto(s, answer, (strlen(answer) + 1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            		{
                		perror("sendto()");
                		exit(2);
            		}
			if(fgets(answer, sizeof(answer), terminal_comando) == NULL) break;        	
		}

	}        


	
        pclose(terminal_comando);
        
        strcpy(answer, "EOM");
        if (sendto(s, answer, (strlen(answer) + 1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
        {
            perror("sendto()");
            exit(2);
        }

    } while (1);
    /*
    * Fecha o socket.
    */
    close(s);
    return 0;
}
