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

int main(int argc, char **argv)
{
    int s;  /* Socket para aceitar conexoes       */
    int ns; /* Socket conectado ao cliente        */
    int shmid;
    int namelen;
    int countMsg = 0, countErase = 0;

    database *banco;

    struct sockaddr_in client;
    struct sockaddr_in server;

    unsigned short port;
    char operacao[2];
    pid_t pid;

    shmid = shmget(IPC_PRIVATE, sizeof(database) * 10, 0666 | IPC_CREAT);
    if (shmid < 0)
    {
        perror("Erro no shmget");
        exit(1);
    }

    banco = (database *)shmat(shmid, NULL, 0);
    if (banco < 0)
    {
        perror("Shared memory attach");
        return 1;
    }

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

    ////////////////////////////////// SEMAFORO ////////////////////////////////

    struct sembuf g_sem_op1[1];
    struct sembuf g_sem_op2[1];

    g_sem_op1[0].sem_num = 0;
    g_sem_op1[0].sem_op = -1;
    g_sem_op1[0].sem_flg = 0;

    g_sem_op2[0].sem_num = 0;
    g_sem_op2[0].sem_op = 1;
    g_sem_op2[0].sem_flg = 0;

    int g_sem_id;

    if ((g_sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    printf("id do semaforo:  %i\n", g_sem_id);

    if (semop(g_sem_id, g_sem_op2, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    ////////////////////////////////// SEMAFORO ////////////////////////////////

    printf("Servidor iniciado na porta %d\n\n", port);

    do
    {
        namelen = sizeof(client);

        if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        pid = fork();

        if (pid == 0)
        {
            int pid_filho = getpid();
            printf("Filho Iniciado com pid: %d. \n", pid_filho);

            do
            {
                if (recv(ns, operacao, sizeof(operacao), 0) == -1)
                {
                    perror("Recv()");
                    exit(6);
                }
                if (strcmp(operacao, "1") == 0)
                {
                    if (semop(g_sem_id, g_sem_op1, 1) == -1)
                    {
                        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
                        exit(1);
                    }

                    countMsg = escreve(ns, countMsg, banco);

                    if (semop(g_sem_id, g_sem_op2, 1) == -1)
                    {
                        fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
                        exit(1);
                    }
                }
                else if (strcmp(operacao, "2") == 0)
                {
                    le(ns, banco);
                }
                else if (strcmp(operacao, "3") == 0)
                {
                    #ifdef PROTECT

                    if (semop(g_sem_id, g_sem_op1, 1) == -1)
                    {
                        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
                        exit(1);
                    }
                    #endif
                    
                    countMsg = exclui(ns, countMsg, banco);
                    
                    #ifdef PROTECT
                    if (semop(g_sem_id, g_sem_op2, 1) == -1)
                    {
                        fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
                        exit(1);
                    }
                    #endif
                }       

            } while (strcmp(operacao, "4") != 0);

            close(ns);
        }

    } while (1);
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}