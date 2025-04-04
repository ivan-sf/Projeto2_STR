#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Definição das filas
QueueHandle_t queueDepositoM1;        // Entrada de M1 (1 item máximo)
QueueHandle_t queueM1Processado;      // Saída de M1 (1 item máximo)
QueueHandle_t queueDepositoM2;        // Entrada de M2 (1 item máximo)
QueueHandle_t queueDepositoM3;        // Entrada de M3 (1 item máximo)
QueueHandle_t queueDepositoSaida;     // Saída final (1 item máximo)

// Semáforos para controlar a capacidade dos depósitos
SemaphoreHandle_t semDepositoM1;
SemaphoreHandle_t semDepositoM1Processado;
SemaphoreHandle_t semDepositoM2;
SemaphoreHandle_t semDepositoM3;
SemaphoreHandle_t semDepositoSaida;

SemaphoreHandle_t semaforoM1;

int contadorItensSaida = 0; // Apenas para debug

// Ajuste da Tarefa de R1 (coloca itens no depósito de M1)
void tarefaR1(void *pvParameters) {
    int item = 0;
    while (1) {
        if (xSemaphoreTake(semDepositoM1, portMAX_DELAY)) { // Espera até que o depósito tenha espaço
            xQueueSend(queueDepositoM1, &item, portMAX_DELAY);
            printf("R1 colocou um item no deposito da M1.\n");
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Simula tempo de transporte
    }
}

// Ajuste da Tarefa de R2 (pega item processado de M1 e leva para M2)
void tarefaR2(void *pvParameters) {
    int item;
    while (1) {
        if (uxQueueSpacesAvailable(queueDepositoM2) > 0) {
            if (xQueueReceive(queueM1Processado, &item, portMAX_DELAY)) {
                printf("R2 pegou um item processado de M1.\n");
                xSemaphoreGive(semDepositoM1Processado);
                vTaskDelay(pdMS_TO_TICKS(600));
                xQueueSend(queueDepositoM2, &item, portMAX_DELAY);
                printf("R2 colocou um item no deposito de M2.\n");
            }
        }
    }
}

void tarefaR3(void *pvParameters) {
    int item;
    while (1) {
        if (xSemaphoreTake(semaforoM1, pdMS_TO_TICKS(100))) { // Controle de acesso
            if (xQueueReceive(queueM1Processado, &item, pdMS_TO_TICKS(100))) {
                printf("R3 pegou um item de M1 processado.\n");
                vTaskDelay(pdMS_TO_TICKS(800)); // Tempo de transporte
                xQueueSend(queueDepositoM3, &item, portMAX_DELAY);
                printf("R3 colocou um item no deposito M3.\n");
            }
            xSemaphoreGive(semaforoM1);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Funcao do R4: transporta itens ou de M2 ou de M3 para a saida
void tarefaR4(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueDepositoM2, &item, pdMS_TO_TICKS(50))) { // Tenta pegar itens de M2 primeiro
            printf("R4 pegou um item da M2.\n");
        } else if (xQueueReceive(queueDepositoM3, &item, pdMS_TO_TICKS(50))) { // Se não tem itens em M2, pega de M3
            printf("R4 pegou um item da M3.\n");
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); // Espera um pouco antes de tentar de novo
            continue;
        }
        //vTaskDelay(pdMS_TO_TICKS(500));
        vTaskDelay(pdMS_TO_TICKS(700));
        xQueueSend(queueDepositoSaida, &item, portMAX_DELAY);
        contadorItensSaida++;
        printf("R4 colocou um item na saida. Total: %d\n", contadorItensSaida);
    }
}

// Ajuste da Tarefa de M1 (pega o item e processa)
void tarefaM1(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueDepositoM1, &item, portMAX_DELAY)) {
            printf("M1 comecou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(1500)); // Tempo de processamento
            printf("M1 finalizou o processamento de um item.\n");

            xSemaphoreGive(semDepositoM1); // Libera o espaço do depósito de entrada
            xSemaphoreTake(semDepositoM1Processado, portMAX_DELAY); // Espera espaço na saída

            xQueueSend(queueM1Processado, &item, portMAX_DELAY);
            printf("M1 colocou um item processado no deposito de saida.\n");
        }
    }
}

// Ajuste da Tarefa de M2 (processa itens)
void tarefaM2(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueDepositoM2, &item, portMAX_DELAY)) {
            printf("M2 começou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(2000)); // Tempo de processamento
            printf("M2 finalizou o processamento de um item.\n");

            xSemaphoreGive(semDepositoM2); // Libera espaço na entrada de M2
            xSemaphoreTake(semDepositoSaida, portMAX_DELAY); // Espera espaço na saída final

            xQueueSend(queueDepositoSaida, &item, portMAX_DELAY);
            printf("M2 colocou um item na saida. Total: %d\n", item);
        }
    }
}

// Funcao da M3: processa itens antes de R4 pegar
void tarefaM3(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueDepositoM3, &item, portMAX_DELAY)) {
            printf("M3 comecou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(2000)); // Tempo de processamento
            printf("M3 processou um item.\n");
            xQueueSend(queueDepositoSaida, &item, portMAX_DELAY);
        }
    }
}

// Função principal (inicializa filas e semáforos)
void main(void) {
    // Criar filas com capacidade de apenas 1 item
    queueDepositoM1 = xQueueCreate(1, sizeof(int));
    queueM1Processado = xQueueCreate(1, sizeof(int));
    queueDepositoM2 = xQueueCreate(1, sizeof(int));
    queueDepositoSaida = xQueueCreate(1, sizeof(int));
    queueDepositoM3 = xQueueCreate(1, sizeof(int));

    if (!queueDepositoM3) {
        printf("Erro ao criar queueDepositoM3!\n");
        while (1);
    }

    if (!queueDepositoM1 || !queueM1Processado || !queueDepositoM2 || !queueDepositoSaida) {
        printf("Erro ao criar filas!\n");
        while (1);
    }

    // Criar semáforos binários e iniciar todos liberados (1 item disponível no início)
    semDepositoM1 = xSemaphoreCreateBinary();
    semDepositoM1Processado = xSemaphoreCreateBinary();
    semDepositoM2 = xSemaphoreCreateBinary();
    semDepositoSaida = xSemaphoreCreateBinary();
    semaforoM1 = xSemaphoreCreateBinary();

    if (!semDepositoM1 || !semDepositoM1Processado || !semDepositoM2 || !semDepositoSaida) {
        printf("Erro ao criar semáforos!\n");
        while (1);
    }

    xSemaphoreGive(semaforoM1); // Libera o semáforo inicialmente
    xSemaphoreGive(semDepositoM1);
    xSemaphoreGive(semDepositoM1Processado);
    xSemaphoreGive(semDepositoM2);
    xSemaphoreGive(semDepositoSaida);

    // Criar tarefas
    xTaskCreate(tarefaR1, "R1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM1, "M1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaR2, "R2", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(tarefaR3, "R3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM2, "M2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM3, "M3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaR4, "R4", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    
    vTaskStartScheduler();
    while (1){
        
    }
}
