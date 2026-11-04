#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // Para sleep() en segundos

// ==========================================
// 1. DECLARACIÓN DE RECURSOS GLOBALES
// ==========================================
sem_t semAB;
sem_t semBC;
sem_t semAC; // Añadido para la nueva sincronización con Hilo C
sem_t semCB; // Añadido para la nueva sincronización con Hilo C

pthread_mutex_t mutex; // Declaración del Mutex
int variable_compartida = 0; // Variable compartida para las cifras

// Hilo A: Desarrollado por Ismael
void* HiloA(void* arg) {
    printf("[Hilo A] Paso 1: Iniciando preparación de datos...\n");
    sleep(1); // Simula tiempo de procesamiento (1 segundo)
    printf("[Hilo A] Paso 1: Datos listos. Liberando Hilo B.\n");
    
    // Incrementar semáforo semAB (V / Signal)
    sem_post(&semAB);
    pthread_exit(NULL);
}

// ==========================================
// 2. IMPLEMENTACIÓN DE HILOS B Y C 
// ==========================================
void* HiloB(void* arg) {
    int cifra2 = 25; // Número asignado por Luis Alfaro

    // Esperar notificación de Hilo C (Parte 1)
    sem_wait(&semCB);

    printf("[Hilo B Dev: Luis Alfaro] Paso 3: Generando Cifra 2 (%d)...\n", cifra2);

    // Exclusión Mutua al escribir en variable_compartida
    pthread_mutex_lock(&mutex);
    variable_compartida = cifra2;
    pthread_mutex_unlock(&mutex);

    printf("[Hilo B] Cifra 2 guardada. Notificando a Hilo C...\n");
    sem_post(&semBC); // Notifica al Hilo C (Parte 2)

    pthread_exit(NULL);
}

// Hilo C: Desarrollado por LuisD 
    int local_cifra1 = 0;
    int local_cifra2 = 0;

    // --- PARTE 1: Procesar Cifra 1 ---
    sem_wait(&semAC); // Espera a Hilo A

    printf("[Hilo C Dev: LuisD] Paso 2: Leyendo Cifra 1...\n");
    pthread_mutex_lock(&mutex);
    local_cifra1 = variable_compartida;
    variable_compartida = 0; // Limpia la variable compartida
    pthread_mutex_unlock(&mutex);

    printf("[Hilo C] Cifra 1 recibida: %d. Limpiada variable compartida. Notificando a Hilo B...\n", local_cifra1);
    sem_post(&semCB); // Notifica al Hilo B

    // --- PARTE 2: Procesar Cifra 2 y Sumar ---
    sem_wait(&semBC); // Espera a Hilo B

    printf("[Hilo C Dev: LuisD] Paso 4: Leyendo Cifra 2...\n");
    pthread_mutex_lock(&mutex);
    local_cifra2 = variable_compartida;
    pthread_mutex_unlock(&mutex);

    // Cálculo e impresión de resultados
    int suma = local_cifra1 + local_cifra2;
    printf("\n==========================================");
    printf("\n[Hilo C] DESGLOSE FINAL Y SUMA:");
    printf("\n -> Cifra 1 (Hilo A): %d", local_cifra1);
    printf("\n -> Cifra 2 (Hilo B): %d", local_cifra2);
    printf("\n -> SUMA TOTAL: %d", suma);
    printf("\n==========================================\n");

    pthread_exit(NULL);
}

// ==========================================
// 3. FUNCIÓN PRINCIPAL (MAIN CORREGIDO)
// ==========================================
int main(int argc, char *argv[]) { 
    pthread_t hA, hB, hC;

    // Inicializar Mutex
    pthread_mutex_init(&mutex, NULL);

    // Inicializar Semáforos
    sem_init(&semAB, 0, 0);
    sem_init(&semBC, 0, 0);
    sem_init(&semAC, 0, 0);
    sem_init(&semCB, 0, 0);

    printf("=== INICIO DE LA SUMA SINCRONIZADA ===\n\n");

    // Creación de hilos
    pthread_create(&hC, NULL, HiloC, NULL);
    pthread_create(&hB, NULL, HiloB, NULL);
    pthread_create(&hA, NULL, HiloA, NULL);

    // Esperar terminación de hilos
    pthread_join(hA, NULL);
    pthread_join(hB, NULL);
    pthread_join(hC, NULL);

    // Destruir recursos y semáforos
    sem_destroy(&semAB);
    sem_destroy(&semBC);
    sem_destroy(&semAC);
    sem_destroy(&semCB);
    pthread_mutex_destroy(&mutex);

    printf("\n=== FIN DE LA EJECUCIÓN ===\n");
    return 0;
}
