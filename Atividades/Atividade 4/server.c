//
//
//Usar Parametro -pthread para compilar
//Ex. gcc -pthread -o server server.c
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <signal.h>

#define MAX_MSG 10
#define shm_key 1024
#define SEM_KEY 0x1451

typedef struct
{
    char usuario[19];
    char msg[79];
    int flagVazio; //1 sim ; -99 nao
} database;

//Struct para passar argunmentos para as funcoes das threads--------------------------------------------------
typedef struct
{
    int ns;
    char operacao[2];

} thread_arg, *ptr_thread_arg;
//--------------------------------------------------------------------------------------------------------------

// Variaveis globais para as Threads ---------------------------------------------------------------------------
int countMsg;
database *banco[MAX_MSG];
//---------------------------------------------------------------------------------------------------------------

int escreve(int ns, int countMsg, database banco[])
{
    char usuario[19];
    char msg[79];

    if (countMsg == MAX_MSG)
    {
        if (send(ns, "ERR1", sizeof("ERR1"), 0) < 0)
        {
            perror("Send()");
            exit(5);
        }
    }
    else
    {
        if (send(ns, "OK", sizeof("OK"), 0) < 0)
        {
            perror("Send()");
            exit(5);
        }
        if (recv(ns, usuario, sizeof(usuario), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }
        if (recv(ns, msg, sizeof(msg), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        };

        for (int i = 0; i < MAX_MSG; i++)
        {
            if (banco[i].flagVazio != -99)
            {
                usuario[strlen(usuario) - 1] = '\0';
                msg[strlen(msg) - 1] = '\0';
                memcpy(banco[i].usuario, usuario, sizeof(usuario));
                memcpy(banco[i].msg, msg, sizeof(msg));
                banco[i].flagVazio = -99;
                countMsg++;
                break;
            }
        }
    }

    return countMsg;
}

void le(int ns, database banco[])
{
    char sendbuf[20];
    char usuario[19];
    char msg[79];

    int countMsg = 0;

    for (int i = 0; i < MAX_MSG; i++)
    {
        if (banco[i].flagVazio == -99)
        {
            countMsg++;
        }
    }

    sprintf(sendbuf, "%d", countMsg);
    if (send(ns, sendbuf, sizeof(sendbuf), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    for (int i = 0; i < MAX_MSG; i++)
    {
        if (banco[i].flagVazio == -99)
        {
            if (send(ns, banco[i].usuario, sizeof(usuario), 0) < 0)
            {
                perror("Send()");
                exit(5);
            }
            if (send(ns, banco[i].msg, sizeof(msg), 0) < 0)
            {
                perror("Send()");
                exit(5);
            }
        }
    }
}

int exclui(int ns, int countMsg, database banco[])
{
    char sendbuf[20];
    char usuario[19];
    char msg[79];
    int countErase;
    database erased[MAX_MSG];

    if (recv(ns, usuario, sizeof(usuario), 0) == -1)
    {
        perror("Recv()");
        exit(6);
    }
    usuario[strlen(usuario) - 1] = '\0'; //-1 por causa do enter

    countErase = 0;

    for (int i = 0; i < MAX_MSG; i++)
    {
        if (strcmp(usuario, banco[i].usuario) == 0)
        {
            banco[i].flagVazio = 1;
            memcpy(erased[countErase].msg, banco[i].msg, sizeof(banco[i].msg));
            memcpy(erased[countErase].usuario, banco[i].usuario, sizeof(banco[i].usuario));
            countErase++;
            countMsg--;
        }
    }
    sprintf(sendbuf, "%d", countErase);
    if (send(ns, sendbuf, sizeof(sendbuf), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    for (int i = 0; i < countErase; i++)
    {

        if (send(ns, erased[i].usuario, sizeof(usuario), 0) < 0)
        {
            perror("Send()");
            exit(5);
        }

        if (send(ns, erased[i].msg, sizeof(msg), 0) < 0)
        {
            perror("Send()");
            exit(5);
        }
    }

    return countMsg;
}

void thread_func(void *arg)
{
    ptr_thread_arg thread_arg = (ptr_thread_arg)arg;

    int pid_thread = pthread_self();
    printf("Thread Iniciada com Id: %d. \n", pid_thread);
    printf("Socket: %d \n", thread_arg->ns);

    do
    {
        if (recv(thread_arg->ns, thread_arg->operacao, sizeof(thread_arg->operacao), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }
        if (strcmp(thread_arg->operacao, "1") == 0)
        {
            countMsg = escreve(thread_arg->ns, countMsg, banco);
        }
        else if (strcmp(thread_arg->operacao, "2") == 0)
        {
            le(thread_arg->ns, banco);
        }
        else if (strcmp(thread_arg->operacao, "3") == 0)
        {
            countMsg = exclui(thread_arg->ns, countMsg, banco);
        }

    } while (strcmp(thread_arg->operacao, "4") != 0);

    close(thread_arg->ns);
}

int main(int argc, char **argv)
{
    int s; /* Socket para aceitar conexoes       */
    int namelen;
    int countErase = 0;
    int ns_local;

    struct sockaddr_in client;
    struct sockaddr_in server;

    unsigned short port;
    char operacao[2];

    int thread_create_result;
    pthread_t ptid;
    thread_arg t_arg;

    countMsg = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short)atoi(argv[1]);

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }

    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }

    printf("Servidor iniciado na porta %d\n\n", port);

    do
    {
        namelen = sizeof(client);

        if ((ns_local = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        t_arg.ns = ns_local;
        strcpy(t_arg.operacao, operacao);

        thread_create_result = pthread_create(&ptid, NULL, &thread_func, &t_arg);

        if (thread_create_result != 0)
        {
            perror("Nao foi possivel criar a Thread!");
            exit(thread_create_result);
        }

    } while (1);
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}