// TENTANDO SIMULAR UM BANCO USANDO MULTITHREAD PARA REPRESENTAR CADA OPERAÇÃO   

// Comando para especificar um padrão de funcionalidades do SO que o código irá usar
// Necessário para a utilização de Barreiras
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Biblioteca para utilização do sleep
#include <pthread.h> // Biblioteca para uso de threads e mutexes
#include <semaphore.h> // Biblioteca para uso de semáforos

#define NUM_CLIENTS 15 // Número total de clientes (threads) que acessarão o sistema
#define QUANT_SISTEMA 3 // Número total de operações de sistema
#define QUANT_OPERACAO 5 // Número total de operações existentes

int saldo = 5000; // Valor inicial presente na conta

sem_t semaforo; // Cria o semaforo
pthread_mutex_t mutex; // Cria o mutex 
pthread_barrier_t barreira; // Cria a barreia

// Cria tipos de ponteiros referentes a uma função
typedef void *tipoSistema();
typedef void *tipoOperacao(void *arg);

// Cria vetores de ponteiros para funções dos tipos acima
tipoSistema *ponteiroSistema[QUANT_SISTEMA];
tipoOperacao *ponteiroOperacao[QUANT_OPERACAO];


////////////////// [ SISTEMA ]  //////////////////

// Rotina para inicializar os processos principais
// Recebe como argumento a thread responsável pela inicialização
void *conexao_dados() 
{
    printf("Carregando os dados da conta...\n\n");

    pthread_barrier_wait(&barreira); // A thread espera na barreira
    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para estabelecer a conexão com o banco de dados
// Recebe como argumento a thread responsável pela conexão
void *conexao_bd() 
{
    printf("Estabelecendo a conexão com o banco de dados...\n\n");

    pthread_barrier_wait(&barreira); // A thread espera na barreira
    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para carregar as configurações da conta
// Recebe como argumento a thread responsável pela conexão
void *conexao_config() 
{
    printf("Carregando as configurações da conta...\n\n");

    pthread_barrier_wait(&barreira); // A thread espera na barreira
    pthread_exit(NULL); // Finaliza a thread
}

////////////////// [ OPERAÇÕES ]  //////////////////

// Rotina para a operação de saque da conta
// Recebe como argumento o cliente que efetuará o saque
void *saque(void *arg) 
{
    int valor = rand() % 501; // Gera um valor aleatório entre 0 e 500 para ser sacado
    int cliente = *(int*)arg;

    pthread_mutex_lock(&mutex); // Efetua a trava no mutex

    // Verifica se o saldo é suficiente para o saque
    if (saldo >= valor) 
    {
        printf("Cliente %d está sacando R$%d\n", cliente, valor);
        saldo -= valor;
    } 
    else 
        printf("Cliente %d não pode sacar R$%d, saldo insuficiente\n", cliente, valor);
    
    printf("Saldo atual: R$%d\n\n", saldo);

    pthread_mutex_unlock(&mutex); // Libera a trava do mutex
    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para a operação de depósito na conta
// Recebe como argumento o cliente que efetuará o depósito
void *deposito(void *arg) 
{
    int valor = rand() % 501; // Gera um valor aleatório entre 0 e 500 para ser depositado
    int cliente = *(int*)arg;

    pthread_mutex_lock(&mutex); // Efetua a trava no mutex

    printf("Cliente %d está depositando R$%d\n", cliente, valor);
    saldo += valor;

    printf("Saldo atual: R$%d\n\n", saldo);

    pthread_mutex_unlock(&mutex); // Libera a trava do mutex
    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para a operação de consulta de saldo da conta
// Recebe como argumento o cliente que efetuará a consulta
void *consulta_saldo(void *arg) 
{
    int cliente = *(int*)arg;

    pthread_mutex_lock(&mutex); // Efetua a trava no mutex

    printf("Cliente %d consultou o saldo: R$%d\n\n", cliente, saldo);

    pthread_mutex_unlock(&mutex); // Libera a trava do mutex
    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para a operação de alteração de dados da conta (não atrapalha outras operações)
// Recebe como argumento o cliente que efetuará o pedido
void *atualizacao_dados(void *arg) 
{
    int cliente = *(int*)arg;

    printf("Cliente %d requisitou uma alteração de informações pessoais...\n\n", cliente);

    pthread_exit(NULL); // Finaliza a thread
}

// Rotina para a operação de suporte ao cliente (não atrapalha outras operações)
// Recebe como argumento o cliente que efetuará o pedido
void *suporte_cliente(void *arg) 
{
    int cliente = *(int*)arg;

    printf("Cliente %d requisitou um serviço de suporte ao cliente...\n\n", cliente);

    pthread_exit(NULL); // Finaliza a thread
}

////////////////// [ CLIENTE ]  //////////////////

// Função referente às operações possíveis do cliente
// Recebe como argumento o cliente que realizará uma ação
void *cliente(void *arg) 
{
    int operacao = rand() % QUANT_OPERACAO; // Seleciona aleatoriamente uma operação: 0 = saque, 1 = depósito, 2 = consulta de saldo, 3 = atualizacao de dados pessoais, 4 = suporte ao cliente
    pthread_t thread;  // Cria uma variável thread

    // Cria uma thread para realizar a rotina na posição sorteada
    pthread_create(&thread, NULL, ponteiroOperacao[operacao], arg);

    // Espera a thread terminar
    pthread_join(thread, NULL);

    printf("Cliente [%d] completou sua operação\n\n", *(int*)arg);
    sem_post(&semaforo); // Sinaliza ao semáforo que terminou o processo que estava realizando

    pthread_exit(NULL); // Finaliza a thread
}

// Função para gerenciar a "Fila de requisições" de acesso ao sistema do banco
// Recebe como argumento o cliente que está tentando acessar o banco
void *user_queue(void *arg)
{
    int user = *(int*)arg;

    printf("Cliente [%d] entrou na fila...\n\n", user);
    
    // Checa o valor do semaforo. Se for 0, a thread espera até que seja liberado 
    // Senão, permite a thread e decrementa o valor do semaforo
    sem_wait(&semaforo);

    printf("Cliente [%d] entrou no sistema\n\n", user);
    cliente(arg); // Efetua uma operação

    free(arg); // Libera o cliente que finalizou a operação
    pthread_exit(NULL); // Finaliza a thread
}

int main() 
{
    pthread_t operacoes[QUANT_SISTEMA]; // Cria múltiplas threads

    // Preenche o vetor de funções do sistema
    ponteiroSistema[0] = conexao_dados;
    ponteiroSistema[1] = conexao_bd;
    ponteiroSistema[2] = conexao_config;

    pthread_barrier_init(&barreira, NULL, 3); // Inicializa a barreira

    // Criação das threads de sistema
    for (int i = 0; i < QUANT_SISTEMA; i++)
        pthread_create(&operacoes[i], NULL, ponteiroSistema[i], NULL);
    
    sleep(3);
    printf("Sistema inicializado com sucesso!!\n\n");
    sleep(3);
    
    for (int i = 0; i < QUANT_SISTEMA; i++)
        pthread_join(operacoes[i], NULL); // Espera uma thread finalizar

    pthread_barrier_destroy(&barreira); // Desaloca a barreira

    // Preenche o vetor de funções de operação do cliente
    ponteiroOperacao[0] = saque;
    ponteiroOperacao[1] = deposito;
    ponteiroOperacao[2] = consulta_saldo;
    ponteiroOperacao[3] = atualizacao_dados;
    ponteiroOperacao[4] = suporte_cliente;

    pthread_t threads[NUM_CLIENTS]; // Cria múltiplas threads
    
    pthread_mutex_init(&mutex, NULL); // Inicializa o mutex

    // Inicializa o semáforo, passando como parâmetro a variável de semáforo, um valor que indica se são múltiplos processos ou threads e por fim a quantidade de threads que podem "executar ao mesmo tempo"
    sem_init(&semaforo, 0, 2);

    // Criação das threads de clientes
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        // É necessário criar endereços diferentes para cada cliente
        int *num_cliente = (int*)malloc(sizeof(int));
        *num_cliente = i;

        pthread_create(&threads[i], NULL, user_queue, num_cliente);
    }
    
    for (int i = 0; i < NUM_CLIENTS; i++)
        pthread_join(threads[i], NULL); // Espera uma thread finalizar

    pthread_mutex_destroy(&mutex); // Desaloca mutex
    sem_destroy(&semaforo); // Desaloca semaforo

    return 0;
}
