#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Cliente TCP
 */
int main(int argc, char **argv)
{
    unsigned short port;       
    char usuario[19],msg[79],recvbuf[20];                          
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s,option,count,i;

                         

    /*
     * O primeiro argumento (argv[1]) e o hostname do servidor.
     * O segundo argumento (argv[2]) e a porta do servidor.
     */
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
do{
    printf("\n\n===========================\n");
    printf("Opcoes\n");
    printf("1 - Cadastrar mensagens\n");
    printf("2 - Ler mensagens\n");
    printf("3 - Apagar mensagens\n");
    printf("4 - Sair\n>");

    scanf("%1d",&option);
    system("clear");
    switch(option){
        case 1:
            printf("Usuario: ");
            scanf("%19s",usuario);
            printf("Mensagem: ");
            scanf("%79s",msg);
            
            if(send(s,"1",sizeof("1"),0) < 0){
                perror("Send()");
                exit(5);
            }

            if (recv(s, recvbuf, sizeof(recvbuf), 0) == -1){
                perror("Recv()");
                exit(6);
            }
            if(strcmp(recvbuf,"ERR1") == 0){
                printf("Maximo de mensagens cadastradas excedidas\n\n");
                break;
            }
            if (send(s, usuario, sizeof(usuario), 0) < 0  ){  
                perror("Send()");
                exit(5);
            }
            if (send(s, msg, sizeof(msg), 0) < 0){  
                perror("Send()");
                exit(5);
            }
            printf("Mensagem enviada ao servidor!\n\n");
   
            break;
        case 2:
        if(send(s,"2",sizeof("2"),0) < 0){
                perror("Send()");
                exit(5);
            }
            if (recv(s, recvbuf, sizeof(recvbuf), 0) == -1){
                perror("Recv()");
                exit(6);
            }
            printf("Mensagens cadastradas: %s\n",recvbuf);
            for(i=0;i<atoi(recvbuf);i++){
                
                if (recv(s, usuario, sizeof(usuario), 0) == -1){
                    perror("Recv()");
                    exit(6);
                }

                if (recv(s, msg, sizeof(msg), 0) == -1){
                    perror("Recv()");
                    exit(6);
                }
                 printf("Usuario: %s Mensagem: %s\n", usuario,msg);
            }
            break;
        case 3:
        if(send(s,"3",sizeof("3"),0) < 0){
                perror("Send()");
                exit(5);
            }
            break;
        case 4:
             /* Fecha o socket */
             if(send(s,"4",sizeof("4"),0) < 0){
                perror("Send()");
                exit(5);
            }
            close(s);
            printf("Cliente terminou com sucesso.\n");
            exit(0);
  
            break;
        default:
            printf("\nOpcao invalida!\n\n");
            break;
    }
}while(option != 4);


   
}


