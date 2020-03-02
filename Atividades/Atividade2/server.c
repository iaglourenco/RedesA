#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#define MAX_MSG 10


struct database{
    char usuario[19];
    char msg[79];
    int flagVazio; //1 sim ; -99 nao
};


/*
 * Servidor TCP
 */
int main(int argc, char **argv)
{
    unsigned short port;       
    char usuario[19],sendbuf[20];              
    char msg[79];              
    struct sockaddr_in client; 
    struct sockaddr_in server; 
    int s;                     /* Socket para aceitar conexoes       */
    int ns;                    /* Socket conectado ao cliente        */
    int namelen;
    char operacao[2];
    struct database banco[MAX_MSG],erased[MAX_MSG];
    int countMsg=0,i,countErase=0;
    /*
     * O primeiro argumento (argv[1]) e a porta
     * onde o servidor aguardara por conexoes
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexoes
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

   /*
    * Define a qual endereco IP e porta o servidor estara ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */
    server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor a porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
       perror("Bind()");
       exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexoes e
     * cria uma fila de conexoes pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    system("clear");
    printf("Servidor iniciado na porta %d\n\n",port);
do{
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }
    do{
        if(recv(ns,operacao,sizeof(operacao),0) == -1){
            perror("Recv()");
            exit(6);
        }
            if(strcmp(operacao,"1")==0){
                if(countMsg == MAX_MSG){
                    if(send(ns,"ERR1",sizeof("ERR1"),0) < 0){
                        perror("Send()");
                        exit(5);
                    }
                    }else{
                        if(send(ns,"OK",sizeof("OK"),0) < 0){
                            perror("Send()");
                            exit(5);
                        }
                        if (recv(ns, usuario, sizeof(usuario), 0) == -1){
                            perror("Recv()");
                            exit(6);
                        }
                        if (recv(ns, msg, sizeof(msg), 0) == -1){
                            perror("Recv()");
                            exit(6);
                        };
                        for(i=0;i<MAX_MSG;i++){

                            if(banco[i].flagVazio != -99){
                                usuario[strlen(usuario)-1]='\0';
                                msg[strlen(msg)-1]='\0';
                                memcpy(banco[i].usuario,usuario,sizeof(usuario));
                                memcpy(banco[i].msg,msg,sizeof(msg));
                                banco[i].flagVazio = -99;
                                countMsg++;
                                //printf("CADASTRADO-> Usuario: %s Mensagem: %s\n", usuario,msg);
                                break;
                            }
                        }
                        
                    }
            }
            else if(strcmp(operacao,"2")==0){
                sprintf(sendbuf,"%d",countMsg);
                if(send(ns,sendbuf,sizeof(sendbuf),0) < 0){
                    perror("Send()");
                    exit(5);
                }
    
                for(i=0;i<MAX_MSG;i++){
                    if(banco[i].flagVazio == -99){
                        if(send(ns,banco[i].usuario,sizeof(usuario),0) < 0){
                            perror("Send()");
                            exit(5);
                        }
                        if(send(ns,banco[i].msg,sizeof(msg),0) < 0){
                            perror("Send()");
                            exit(5);
                        }  
                    }
                }
                
            } 
            else if(strcmp(operacao,"3")==0){

                if(recv(ns,usuario,sizeof(usuario),0) == -1){
                    perror("Recv()");
                    exit(6);
                }
                usuario[strlen(usuario)-1]='\0';
                                

                countErase=0;
                for(i=0;i<MAX_MSG;i++){
                    if(strcmp(usuario,banco[i].usuario) == 0){
                        banco[i].flagVazio=1;
                        memcpy(erased[countErase].msg,banco[i].msg,sizeof(banco[i].msg));
                        memcpy(erased[countErase].usuario,banco[i].usuario,sizeof(banco[i].usuario));
                        countErase++;
                        countMsg--;
                        //printf("APAGADO -> Usuario: %s Msg: %s\n",banco[i].usuario,banco[i].msg);
                    }
                }
                sprintf(sendbuf,"%d",countErase);
                if(send(ns,sendbuf,sizeof(sendbuf),0) < 0){
                    perror("Send()");
                    exit(5);
                }
                for(i=0;i<countErase;i++){
                     
                     if(send(ns,erased[i].usuario,sizeof(usuario),0) < 0){
                        perror("Send()");
                        exit(5);
                    }

                    if(send(ns,erased[i].msg,sizeof(msg),0) < 0){
                        perror("Send()");
                        exit(5);
                    }
                }
                

            }
    
    }while(strcmp(operacao,"4") != 0);
    close(ns);

}while(1);
close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}


