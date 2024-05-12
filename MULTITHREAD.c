// TENTANDO SIMULAR UM BANCO USANDO MULTITHREAD PARA REPRESENTAR CADA OPERAÇÃO   

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_CLIENTS 4

int saldo = 5000;

sem_t semaforo; // cria o semaforo
pthread_mutex_t mutex; // cria o mutex 

// DIFERENÇAS entre mutex e semaforos: 
// mutex: Apenas a thread que bloqueou o mutex pode libera-lo (conceito de propietario)
// semaforo: Qualquer thread pode decrementar ou incremetar um semaforo

// mutex: Possui apenas dois estados (bloqueado / desbloqueado)
// semaforo: Pode variar de 0 ate um numero N especificado na inicializacao

// mutex: Protege seções criticas e evita condições de corrida
// semaforo: Controla o acesso de um recurso particular ou uma determinada ação


void *saque(void *arg) 
{
    int valor = rand() % 501; // Saque aleatório entre 0 e 500
    int cliente = *(int*)arg;

    pthread_mutex_lock(&mutex);

    if (saldo >= valor) 
    {
        printf("Cliente %d está sacando R$ %d\n", cliente, valor);
        saldo -= valor;
    } 
    else 
        printf("Cliente %d não pode sacar R$ %d, saldo insuficiente\n", cliente, valor);
    
    printf("Saldo atual: R$ %d\n", saldo);

    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *deposito(void *arg) 
{
    int cliente = *(int*)arg;
    int valor = rand() % 501; 

    pthread_mutex_lock(&mutex);

    printf("Cliente %d está depositando R$ %d\n", cliente, valor);
    saldo += valor;
    printf("Saldo atual: R$ %d\n", saldo);

    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *consulta_saldo(void *arg) 
{
    int cliente = *(int*)arg;

    pthread_mutex_lock(&mutex);

    printf("Cliente %d consultou o saldo: R$ %d\n", cliente, saldo);

    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *cliente(void *arg) 
{
    int operacao = rand() % 3; // Seleciona aleatoriamente uma operação: 0 = saque, 1 = depósito, 2 = consulta de saldo
    pthread_t thread; 

    if (operacao == 0) 
        pthread_create(&thread, NULL, saque, arg);
    else if (operacao == 1) 
        pthread_create(&thread, NULL, deposito, arg);
    else 
        pthread_create(&thread, NULL, consulta_saldo, arg);

    pthread_join(thread, NULL);

    printf("USER [%d] completes his operation\n", *(int*)arg);
    sem_post(&semaforo);

    pthread_exit(NULL);
}


void *user_queue(void *arg)
{
    int user = *(int*)arg;

    printf("USER [%d] is joining the queue\n", user);
    // checa o valor do semaforo, se for 0, a thread espera até que seja liberado 
    // senão, permite a thread e decremata o valor do semaforo
    sem_wait(&semaforo); // --

    printf("USER [%d] is performing an operation\n", user);
    cliente(arg);

    // sem_post(&semaforo); // incremeta o valor do semaforo ++

    free(arg);
    pthread_exit(NULL);
}

//alef 
int main() 
{
    int i;
    pthread_t threads[NUM_CLIENTS]; // cria multiplas threads
    pthread_mutex_init(&mutex, NULL); // inicializa o mutex 

    // Como estamos usando multiplas threads e nao processos, o 2° paremetro deve ser zero, porque estamos usando apenas 1 processo principal 
    // o Terceiro parametro e o valor incial do semaforo, que sera pode variar dependedno da quantidade de threads que vão poder atravessar o semaforo
    sem_init(&semaforo, 0, 2);

    for (i = 0; i < NUM_CLIENTS; i++)
    {
        // essa alocação é feita, porque é possivel que ao passar i como parametro, uma thread possa acabar acessando o mesmo endereço de uma thread anterior, causando problemas, agora meio que cada thread meio que vai ter sua propia varivel
        int *num_cliente = (int*)malloc(sizeof(int));
        *num_cliente = i;

        // printf("LOOP 1: Thread %d começou a execução!\n", *num_cliente);
    
        pthread_create(&threads[i], NULL, user_queue, num_cliente);
    }
    
    for (i = 0; i < NUM_CLIENTS; i++)
    {
        pthread_join(threads[i], NULL); // termina a execução de uma thread
        // printf("LOOP 2: Thread %d terminou execução!\n", i);
    }

    pthread_mutex_destroy(&mutex); // desaloca mutex
    sem_destroy(&semaforo); // desaloca semaforo

    return 0;
}
