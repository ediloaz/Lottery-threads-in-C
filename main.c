#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <gtk/gtk.h>

#define UNIDAD_TRABAJO 50


// Usen para compilar y correr:
//
// gcc -o gladewin main.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic ; ./gladewin



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

//Inicialización de los componentes en Interfaz
int maxThreadsEnInterfaz = 17;

GtkBuilder *builder; 
GtkWidget *window;
GtkWidget *g_lbl_mode;
GtkWidget *g_lbl_quantum;
GtkWidget *g_lbl_pi_general;

struct VisualThread{
    GtkWidget *progress_bar;
    GtkWidget *percentage;
    GtkWidget *result;
    GtkWidget *spinner;
};

struct VisualThread *visual_threads;

// Usar random, se usa con int r = rand(); 
// srand(time(NULL));

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


//
//
// Funciones para la Interfaz de usuario

float getPorcentajeTrabajo(int positionThread){
    int totalUnidadesActuales = threads[positionThread].total_unidades_trabajo - threads[positionThread].unidades_de_trabajo_pendientes;
    return (totalUnidadesActuales * 100 / threads[positionThread].total_unidades_trabajo);
}

// Actualiza la interfaz
void actualizarInterfaz(int threadActual){
    printf("actualizarInterfaz");
    
    // Actualizamos todos los hilos en pantalla
    for (int i = 0; i < TOTAL_THREADS+1; i++) {
        
        char value_percentage[100];
        char value_result[100];
        sprintf(value_percentage, "%i%c", (int)getPorcentajeTrabajo(i), '%'); 
        sprintf(value_result, "%i", threads[i].resultado_parcial_de_pi); 

        gtk_label_set_text(GTK_LABEL(visual_threads[i].percentage), value_percentage);
        gtk_progress_bar_set_fraction(visual_threads[i].progress_bar, (getPorcentajeTrabajo(i)/100));
        gtk_label_set_text(GTK_LABEL(visual_threads[i].result), value_result);
        
        // Si es el thread actual le aplica un estilo único
        // TODO
        if (i==threadActual){
            gtk_spinner_start(visual_threads[i].spinner);
            gtk_style_context_remove_class( gtk_widget_get_style_context(visual_threads[i].progress_bar), "progressBar" );
            gtk_style_context_add_class( gtk_widget_get_style_context(visual_threads[i].progress_bar), "currentProgressBar" );
            
        }else{
            gtk_spinner_stop(visual_threads[i].spinner);
            gtk_style_context_remove_class( gtk_widget_get_style_context(visual_threads[i].progress_bar), "currentProgressBar" );
            gtk_style_context_add_class( gtk_widget_get_style_context(visual_threads[i].progress_bar), "progressBar" );
            // gtk_style_context_add_class ( gtk_widget_get_style_context(visual_threads[i].progress_bar), "currentProgressBar" );
        }
    }

    // Revisa si algún evento está pendiente de actualizar y lo actualiza.
    // Éste se usa para actualizar cambios en el UI e invocar timeouts en interfaz.
    // Mientras o luego de hacer algún cambio de la interfaz (como el set_text).
    while (gtk_events_pending ())
        gtk_main_iteration ();
        
}



// TODO para probar la interfaz
void testeandoLaInterfaz(){

    while(1){
        int randomThread = rand()%TOTAL_THREADS; 
        int repeticiones = rand()%20; 
        for (int i = 0; i < repeticiones; i++) {
            sleep(1);
            threads[randomThread].resultado_parcial_de_pi = rand()%100000;
            threads[randomThread].unidades_de_trabajo_pendientes = threads[randomThread].unidades_de_trabajo_pendientes-1;
            actualizarInterfaz(randomThread);
        }
    }
}

// Configuración constantes de la interfaz
void configurarConstantesDeInterfaz()
{
    char quatumString[15]; 
    sprintf(quatumString, "%s%i", "Quantum: ", QUANTUM); 
    char piGeneralString[250]; 
    sprintf(piGeneralString, "%s%f", "Pi general calculado: ", pi_Calculado); 
    gtk_label_set_text(GTK_LABEL(g_lbl_mode), ES_EXPROPIATIVO ? "Expropiativo" : "No expropiativo");
    gtk_label_set_text(GTK_LABEL(g_lbl_quantum), quatumString);
    gtk_label_set_text(GTK_LABEL(g_lbl_pi_general), piGeneralString);
}

// Creación de la interfaz
void iniciarInterfaz(int argc, char *argv[])
{
    
    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "interface.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "interface"));
    gtk_builder_connect_signals(builder, NULL);

    // Estilos
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    // Referencia de los componentes en interfaz que se ocupan manejar con código
    g_lbl_mode = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_mode"));
    g_lbl_quantum = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_quantum"));
    g_lbl_pi_general = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_pi_general"));
    visual_threads = malloc(sizeof(*visual_threads) * maxThreadsEnInterfaz);

    // Inicializamos todos los threads
    for (int i = 0; i < maxThreadsEnInterfaz; i++) {
        char id_progress_bar[100];
        char id_result[100];
        char id_percentage[100];
        char id_spinner[100];

        sprintf(id_progress_bar, "%s%i%s", i>8 ? "th_" : "th_0", i+1, "_prg_bar"); 
        sprintf(id_result, "%s%i%s", i>8 ? "th_" : "th_0", i+1, "_lbl_result"); 
        sprintf(id_percentage, "%s%i%s", i>8 ? "th_" : "th_0", i+1, "_lbl_percentage"); 
        sprintf(id_spinner, "%s%i%s", i>8 ? "th_" : "th_0", i+1, "_spinner");

        if (i<TOTAL_THREADS){
            // Los agrega al array para administrarlos luego
            visual_threads[i].progress_bar = GTK_WIDGET(gtk_builder_get_object(builder, id_progress_bar));
            visual_threads[i].result = GTK_WIDGET(gtk_builder_get_object(builder, id_result));
            visual_threads[i].percentage = GTK_WIDGET(gtk_builder_get_object(builder, id_percentage));
            visual_threads[i].spinner = GTK_WIDGET(gtk_builder_get_object(builder, id_spinner));
        }else{
            // Simplemente los oculta de interfaz
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, id_progress_bar)));
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, id_result)));
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, id_percentage)));
            gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, id_spinner)));
        }
    }

    configurarConstantesDeInterfaz();
    g_object_unref(builder);
    gtk_widget_show(window);
    gtk_main();
}

// Se llama cuando la interfaz es cerrada
void on_window_main_destroy()
{
    gtk_main_quit();
}



// Esta función es llamada desde Glade una vez que inicia la interfaz
void algorithm(){

    testeandoLaInterfaz();

    //Inicializa random
    time_t t;
    srand((unsigned) time(&t));
    
    //Asigna función a alarma
    if(signal(SIGALRM, sig_alarm_handler) == SIG_ERR){
    	printf("Error de la señal");
    	exit(1);
    }
    
    lottery_scheduler();
}

int main(int argc, char **argv)
{
    read_parameters();  
    iniciarInterfaz(argc, argv);

    // Se está llamando desde interfaz la inicialización del programa, MIENTRAS se encuentra
    // la forma de hacerlo automáticamente luego de inicializar la interfaz.
    // algorithm();
        
    return 0;
}
