#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>

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

//Dato para sigsetjmp
sigjmp_buf jmpbuf;

//Funciones
void calcular_unidad_trabajo(int thread_ganador);
void read_parameters();
int obtenerThread(int boleto_ganador);
int todos_los_threads_terminaron();
void lottery_scheduler();
void trabajar(int ganador);
void calcular_unidad_trabajo(int thread_ganador);

//Inicializa un thread
void new_thread(struct Thread *thread){
    thread->total_boletos = 0;
    thread->resultado_parcial_de_pi = 0;
    thread->total_unidades_trabajo = 0;
    thread->unidades_de_trabajo_pendientes = 0;
}

//Lee los parámetros desde el archivo de entrada input.txt
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
    
    threads = malloc(sizeof(*threads) * TOTAL_THREADS);
    
    //Total de boletos
    getline(&line, &len, file);
    element = strtok(line, " ");
    for(int i=0; i < TOTAL_THREADS; i++)
    {
    	new_thread(&threads[i]);
    	threads[i].total_boletos = atoi(element);
    	element = strtok(NULL, " ");
    }
    
    //Cantidad de trabajo
    getline(&line, &len, file);
    element = strtok(line, " ");
    for(int i=0; i < TOTAL_THREADS; i++)
    {
    	threads[i].total_unidades_trabajo =  atoi(element);
    	threads[i].unidades_de_trabajo_pendientes = atoi(element);
    	element = strtok(NULL, " ");
    }
        
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

//Selecciona al thread ganador de la lotería
int obtenerThread(int boleto_ganador)
{
    //utilizado por scheduler
	//busca el thread que tiene el boleto ganador utilizando rangos
	//retorna el thread o el Ìndice del thread o algo asÌ
	//IMPORTANTE: No retornar un thread que ya terminÛ

    int cont = 0;

    for (int i = 0; i < TOTAL_THREADS; i++) {        
        cont += threads[i].total_boletos;
        if(cont > boleto_ganador)
        {
            return i;
        } 
    }
}

//Retorna 1 si todos los threads ya terminaron su trabajo
int todos_los_threads_terminaron()
{
    for( int i=0; i < TOTAL_THREADS; i++)
    {
    	if(threads[i].unidades_de_trabajo_pendientes > 0) return 0;
    
    }
    return 1;
}

//Función llamada por la señal de alarma
void sig_alarm_handler(int sigo)
{
    siglongjmp(jmpbuf, 2);
}

//Scheduler por lotería, selecciona siguiente thread
void lottery_scheduler()
{
    sigsetjmp(jmpbuf, 1); //Punto de regreso de threads
    
    if(todos_los_threads_terminaron()){
        free(threads);
    	printf("Todos los threads han terminado.\n");
    	printf("Resultado final de PI: %f\n",pi_Calculado);
    }
    
    else
    {
        int total_boletos = 0;
        for( int i=0; i < TOTAL_THREADS; i++){
            total_boletos +=  threads[i].total_boletos;
        }
            
        int boleto_ganador = rand() % total_boletos;
        int thread_ganador = obtenerThread(boleto_ganador);
        
        //PRINTS DE PRUEBA
        printf("\n----HACIENDO LOTERÍA--------------\n");
        printf("Total de boletos: %d\n", total_boletos);
        printf("Boleto ganador: %d\n", boleto_ganador);
        printf("Thread con boleto ganador: %d\n", thread_ganador);        
        
    }
	
}

//Función que ejecutan los threads al ser seleccionados
void trabajar(int ganador){
   //Modo Expropiativo
   if(ES_EXPROPIATIVO){
   
   	//Activa alarma de interrupción
        alarm(QUANTUM);
        
        //Calcula elementos de serie
        while(threads[ganador].unidades_de_trabajo_pendientes > 0)
        {
            calcular_unidad_trabajo(ganador);
        }
        
   //Modo No Expropiativo            
   }else{
   	int trabajo_hecho = 0;
   	int trabajo_pendiente = threads[ganador].total_unidades_trabajo * PORCENTAJE_A_REALIZAR;
   	
   	while( trabajo_hecho < trabajo_pendiente && threads[ganador].unidades_de_trabajo_pendientes > 0)
   	{
            calcular_unidad_trabajo(ganador);
            trabajo_hecho++;
        }       
        
   }
   
   //Quita tiquetes si thread terminó trabajo
   if(threads[ganador].unidades_de_trabajo_pendientes == 0)
       threads[ganador].total_boletos = 0;
       
   //Regresa a scheduler
   siglongjmp(jmpbuf, 1);
}

//Calcular los 50 siguientes elementos de la serie
void calcular_unidad_trabajo(int thread_ganador)
{
    //Variables útiles: macro UNIDAD_TRABAJO (vale 50)
    //			pi_Calculado, indice_serie_actual
    // Acceder a thread usando: threads[thread_ganador].propiedad			


}


int main(int argc, char **argv)
{
    
    read_parameters();  
    
    //Inicializa random
    time_t t;
    srand((unsigned) time(&t));
    
    //Asigna función a alarma
    if(signal(SIGALRM, sig_alarm_handler) == SIG_ERR){
    	printf("Error de la señal");
    	exit(1);
    }
    
    lottery_scheduler();
        
    return 0;
}
