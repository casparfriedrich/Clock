#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/eu_dst.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "twi.h"
#include "uart.h"

#include "ds1307.hpp"
#include "ht16k33_segment.hpp"
#include "si1145.hpp"

#define disableInterrupts() cli()
#define enableInterrupts() sei()
#define UART() Uart::getInstance()

#define displayAddress 0x70
#define rtcAddress 0x68
#define sensorAddress 0x60

HT16K33_Segment *display = NULL;
DS1307 *rtc = NULL;
SI1145 *sensor = NULL;

QueueHandle_t uartRxQueue;

static void enableUartRxInterrupt() {
    UCSR0B |= (1 << RXCIE0);
}

ISR(USART_RX_vect) {
    uint8_t byte = UART_Rx(NULL);
    xQueueSendToBackFromISR(uartRxQueue, &byte, 0);
}

void task_updateDisplay(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    display->setBrightness(0xF);

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_PERIOD_MS);

        time_t time_gm = rtc->read();
        struct tm *tm_rtc = localtime(&time_gm);

        display->updateDigit(digit0, tm_rtc->tm_hour / 10);
        display->updateDigit(digit1, tm_rtc->tm_hour % 10);
        display->updateDigit(digit2, tm_rtc->tm_min / 10);
        display->updateDigit(digit3, tm_rtc->tm_min % 10);

        printf("%s\r\n", asctime(tm_rtc));
    }
}

void task_updateRtc(void *pvParameters) {
    for (;;) {
        if (4 == uxQueueMessagesWaiting(uartRxQueue)) {
            union {
                uint8_t a[4];
                time_t b;
            } timestamp;

            for (uint8_t i = 0; i < sizeof(time_t); i++) {
                xQueueReceive(uartRxQueue, &timestamp.a[i], 0);
            }

            rtc->write(timestamp.b - UNIX_OFFSET);
        }
    }
}

int main() {
    TWI_init();
    UART_Init();

    FILE *stream = fdevopen(UART_Tx, UART_Rx);
    stderr = stream;
    stdin = stream;
    stdout = stream;

    uartRxQueue = xQueueCreate(4, 1);

    display = new HT16K33_Segment(displayAddress);
    rtc = new DS1307(rtcAddress);
    sensor = new SI1145(sensorAddress);

    set_dst(eu_dst);
    set_zone(1 * ONE_HOUR);

    enableUartRxInterrupt();

    xTaskCreate(task_updateDisplay, NULL, configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(task_updateRtc, NULL, configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1, NULL);

    enableInterrupts();

    vTaskStartScheduler();

    return 0;
}
