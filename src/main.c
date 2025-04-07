#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*
    - Não há limites nos depósitos de entrada e de saída da célula
    - Não se faz necessário uso de semáforos para controlar a entrada e saída da célula
    - Não se faz necessário uso de filas para a entrada e a saída da célula
*/

// Definição das filas
QueueHandle_t queueDepositoM1;        // Entrada de M1 (1 item máximo)
QueueHandle_t queueDepositoM2;        // Entrada de M2 (1 item máximo)
QueueHandle_t queueDepositoM3;        // Entrada de M3 (1 item máximo)

QueueHandle_t queueM1Processado;      // Saída de M1 (1 item máximo)
QueueHandle_t queueM2Processado;      // Saída de M2 (1 item máximo)
QueueHandle_t queueM3Processado;      // Saída de M3 (1 item máximo)

// Semáforos para controlar a capacidade dos depósitos
SemaphoreHandle_t semDepositoM1;
SemaphoreHandle_t semDepositoM1Processado;

SemaphoreHandle_t semDepositoM2;
SemaphoreHandle_t semDepositoM2Processado;

SemaphoreHandle_t semDepositoM3;
SemaphoreHandle_t semDepositoM3Processado;

// Semáforo para controlar o acesso de R2 e R3 ao depósito de saída de M1
SemaphoreHandle_t semaforoM1;

int contadorItensSaida = 0; // Apenas para debug

// Função de R1: transporta itens do depósito de entrada da célula para a M1
void tarefaR1(void *pvParameters) {
    int item = 0;
    while (1) {
        if (xSemaphoreTake(semDepositoM1, portMAX_DELAY)) { // Espera até que o depósito tenha espaço
            xQueueSend(queueDepositoM1, &item, portMAX_DELAY); // Envia para o depósito de entrada de M1
            printf("[R1] colocou um item no deposito da M1.\n");
        }
        vTaskDelay(pdMS_TO_TICKS(700)); // Tempo de transporte
    }
}

// Função de R2: transporta itens do depósito de saída da M1 para a M2
void tarefaR2(void *pvParameters) {
    int item;
    while (1) {
        if (uxQueueSpacesAvailable(queueDepositoM2) > 0) {
            if (xSemaphoreTake(semaforoM1, pdMS_TO_TICKS(100))) {
                if (xQueueReceive(queueM1Processado, &item, pdMS_TO_TICKS(100))) {
                    printf("[R2] Pegou item processado de M1.\n");
                    xSemaphoreGive(semDepositoM1Processado); // Libera o depósito de saída de M1
                    vTaskDelay(pdMS_TO_TICKS(700)); // Tempo de transporte
                    xQueueSend(queueDepositoM2, &item, portMAX_DELAY); // Envia para o depósito de entrada de M2
                    printf("[R2] Colocou item no deposito de M2.\n");
                }
                xSemaphoreGive(semaforoM1);
            }
        }
    }
}

// Função do R3: transporta itens do depósito de saída da M1 para a M3
void tarefaR3(void *pvParameters) {
    int item;
    while (1) {
        if (xSemaphoreTake(semaforoM1, pdMS_TO_TICKS(100))) {
            if (xQueueReceive(queueM1Processado, &item, pdMS_TO_TICKS(100))) {
                printf("[R3] Pegou item processado de M1.\n");
                xSemaphoreGive(semDepositoM1Processado); // Libera o depósito de saída de M1
                vTaskDelay(pdMS_TO_TICKS(1000)); // Tempo de transporte
                xQueueSend(queueDepositoM3, &item, portMAX_DELAY); // Envia para o depósito de entrada de M3
                printf("[R3] Colocou item no deposito de M3.\n");
            }
            xSemaphoreGive(semaforoM1);
        }
    }
}

// Função do R4: transporta itens ou de M2 ou de M3 para a saída da célula
void tarefaR4(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueM2Processado, &item, pdMS_TO_TICKS(50))) { // Tenta pegar itens de M2 primeiro
            printf("[R4] pegou um item da M2.\n");
            xSemaphoreGive(semDepositoM2Processado); // Libera o depósito de sáida de M2
        } else if (xQueueReceive(queueM3Processado, &item, pdMS_TO_TICKS(50))) { // Se não tem itens em M2, pega de M3
            printf("[R4] pegou um item da M3.\n");
            xSemaphoreGive(semDepositoM3Processado); // Libera o depósito de saída de M3
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); // Espera um pouco antes de tentar de novo
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(700)); // Tempo de transporte
        contadorItensSaida++;
        printf("[R4] colocou um item na saida. Total: %d\n", contadorItensSaida);
    }
}

// Função da M1
void tarefaM1(void *pvParameters) {
    int item;
    while (1) {
        if (xQueueReceive(queueDepositoM1, &item, portMAX_DELAY)) {
            printf("[M1] Comecou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(1500)); // Tempo de processamento
            printf("[M1] Finalizou o processamento de um item.\n");

            xSemaphoreGive(semDepositoM1); // Libera o espaço do depósito de entrada
            xSemaphoreTake(semDepositoM1Processado, portMAX_DELAY); // Espera espaço na saída

            xQueueSend(queueM1Processado, &item, portMAX_DELAY); // Envia para o depósito de saída
            printf("[M1] Colocou um item processado no deposito de saida.\n");
        }
    }
}

// Função da M2
void tarefaM2(void *pvParameters) {
    int item;
    while (1) {
        // Verifica se há um item no depósito de entrada
        if (xQueueReceive(queueDepositoM2, &item, portMAX_DELAY)) {
            printf("[M2] Comecou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(1500)); // Tempo de processamento
            printf("[M2] Finalizou o processamento de um item.\n");

            xSemaphoreGive(semDepositoM2); // Libera o espaço do depósito de entrada
            xSemaphoreTake(semDepositoM2Processado, portMAX_DELAY); // Espera espaço na saída

            xQueueSend(queueM2Processado, &item, portMAX_DELAY); // Envia para o depósito de saída
            printf("[M2] Colocou um item processado no deposito de saida.\n");
        }
    }
}

// Função da M3
void tarefaM3(void *pvParameters) {
    int item;
    while (1) {
        // Verifica se há um item no depósito de entrada
        if (xQueueReceive(queueDepositoM3, &item, portMAX_DELAY)) {
            printf("[M3] Comecou a processar um item.\n");
            vTaskDelay(pdMS_TO_TICKS(3000)); // Tempo de processamento
            printf("[M3] Finalizou o processamento de um item.\n");

            xSemaphoreGive(semDepositoM3); // Libera o espaço do depósito de entrada
            xSemaphoreTake(semDepositoM3Processado, portMAX_DELAY); // Espera espaço na saída

            xQueueSend(queueM3Processado, &item, portMAX_DELAY); // Envia para o depósito de saída
            printf("[M3] Colocou um item processado no deposito de saida.\n");
        }
    }
}

// Função principal (inicializa filas e semáforos)
void main(void) {
    // Cria as filas com capacidade de apenas 1 item
    queueDepositoM1 = xQueueCreate(1, sizeof(int));
    queueM1Processado = xQueueCreate(1, sizeof(int));
    
    queueDepositoM2 = xQueueCreate(1, sizeof(int));
    queueM2Processado = xQueueCreate(1, sizeof(int));

    queueDepositoM3 = xQueueCreate(1, sizeof(int));
    queueM3Processado = xQueueCreate(1, sizeof(int));

    if (!queueDepositoM1 || !queueM1Processado || !queueDepositoM2 || !queueM2Processado || !queueDepositoM3 || !queueM3Processado) {
        printf("Erro ao criar as filas!\n");
        while (1);
    }

    // Cria os semáforos
    semDepositoM1 = xSemaphoreCreateBinary();
    semDepositoM1Processado = xSemaphoreCreateBinary();

    semDepositoM2 = xSemaphoreCreateBinary();
    semDepositoM2Processado = xSemaphoreCreateBinary();

    semDepositoM3 = xSemaphoreCreateBinary();
    semDepositoM3Processado = xSemaphoreCreateBinary();

    semaforoM1 = xSemaphoreCreateMutex();

    if (!semDepositoM1 || !semDepositoM1Processado || !semDepositoM2 || !semDepositoM2Processado || !semDepositoM3 || !semDepositoM3Processado || !semaforoM1) {
        printf("Erro ao criar os semáforos!\n");
        while (1);
    }

    xSemaphoreGive(semaforoM1);

    xSemaphoreGive(semDepositoM1);
    xSemaphoreGive(semDepositoM1Processado);

    xSemaphoreGive(semDepositoM2);
    xSemaphoreGive(semDepositoM2Processado);

    xSemaphoreGive(semDepositoM3);
    xSemaphoreGive(semDepositoM3Processado);

    // Cria as tarefas
    xTaskCreate(tarefaR1, "R1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM1, "M1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaR2, "R2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaR3, "R3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM2, "M2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaM3, "M3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(tarefaR4, "R4", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    
    vTaskStartScheduler(); // Inicializa o sistema
    
    while (1){
        
    }
}
