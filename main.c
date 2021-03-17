#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#define UNIDAD_TRABAJO 50

//Parametros
short int ES_EXPROPIATIVO;
int TOTAL_THREADS;
int QUANTUM;
float PORCENTAJE_A_REALIZAR;

//Resultado
double pi_Calculado = 0;

//Último índice de la serie que fue calculado por un thread
int indice_serie_actual = 0;

struct Thread{
    int total_boletos;
    int resultado_parcial_de_pi;
    int total_unidades_trabajo;
    int unidades_de_trabajo_pendientes;
};
//Puntero a arreglo de Threads de tamaño TOTAL_THREADS
struct Thread *threads;

sigjmp_buf jmpbuf;

void new_thread(struct Thread *thread, int boletos){
    thread->total_boletos = boletos;
    thread->resultado_parcial_de_pi = 0;
    thread->total_unidades_trabajo = 0;
    thread->unidades_de_trabajo_pendientes = 0;
}

//
void read_parameters()
{
    FILE *file;
    char *line;
    size_t len = 0;
    char *element;
    
    //Abre el archivo
    if((file = fopen("input.txt","r")) == NULL){
    	printf("Error al abrir archivo");
    	exit(1);
    }
    
    //Lee modo de operación
    getline(&line, &len, file);
    ES_EXPROPIATIVO = atoi(line);
    
    //Lee total de threads
    getline(&line, &len, file);
    TOTAL_THREADS = atoi(line);
    
    struct Thread threadArray[TOTAL_THREADS];
    
    //Total de boletos
    getline(&line, &len, file);
    element = strtok(line, " ");
    for(int i=0; i < TOTAL_THREADS; i++)
    {
    	new_thread(&threadArray[i], atoi(element));
    	element = strtok(NULL, " ");
    }
    
    //Cantidad de trabajo
    getline(&line, &len, file);
    element = strtok(line, " ");
    for(int i=0; i < TOTAL_THREADS; i++)
    {
    	threadArray[i].total_unidades_trabajo = atoi(element);
    	element = strtok(NULL, " ");
    }
    
    threads = threadArray;
    
    //Quantum o Porcentaje
    getline(&line, &len, file);    
    if(ES_EXPROPIATIVO)
    {
    	QUANTUM = atoi(line);   
    }else{
    	PORCENTAJE_A_REALIZAR = atof(line);
    }
    
    fclose(file);
    
}

int todos_los_threads_terminaron()
{
    return 0;
}

void sig_alarm_handler(int sigo)
{
    siglongjmp(jmpbuf, 2);
}

void lottery_scheduler()
{
    sigsetjmp(jmpbuf, 1);
    if(todos_los_threads_terminaron()) exit(0);
    
    else
    {
        int total_boletos = 0;
        
    }
	
}



int main(int argc, char **argv)
{
    
    read_parameters();
    
    //Asigna función a alarma
    if(signal(SIGALRM, sig_alarm_handler) == SIG_ERR){
    	printf("Error de la señal");
    	exit(1);
    }
    
    lottery_scheduler();
        
    return 0;
}
