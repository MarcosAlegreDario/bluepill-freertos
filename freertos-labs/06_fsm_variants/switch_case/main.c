#include "../include/fsm.h"
#include "FreeRTOS.h"
#include "task.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>

static void send_line(const char *msg)
{
    while (*msg)
        usart_send_blocking(USART1, *msg++);
    usart_send_blocking(USART1, '\r');
    usart_send_blocking(USART1, '\n');
}

static void gpio_setup(void)
{
    // 1. Configuración del LED integrado (PC13)
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_clear(GPIOC, GPIO13);

    // 2. NUEVO: Configuración del LED externo (PB0)
    rcc_periph_clock_enable(RCC_GPIOB); // Habilitamos el reloj del puerto B
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO0); // PB0 como salida push-pull
    gpio_clear(GPIOB, GPIO0); // Lo iniciamos apagado por seguridad
}

static void usart_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_PULL_UPDOWN, GPIO10);
    gpio_set(GPIOA, GPIO10);

    usart_set_baudrate(USART1, 9600);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_enable(USART1);
}

static fsm_event_t event_from_char(char c) {
    switch (c) {
    case '0': 
        gpio_set(GPIOB, GPIO0);   // Enciende el LED externo
        return (fsm_event_t){EVENT_CMD_0};
    case '1': 
        gpio_clear(GPIOB, GPIO0); // Apaga el LED externo
        return (fsm_event_t){EVENT_CMD_1};
    case '2': 
        gpio_set(GPIOB, GPIO0);   // Enc0=iende el LED externo
        return (fsm_event_t){EVENT_CMD_2};
    default:  
        return (fsm_event_t){EVENT_INVALID};
    }
}

static void led_task(void *args)
{
    (void)args;
    while (1) {
        switch (fsm_get_state()) {
        case STATE_OFF:
            gpio_set(GPIOC, GPIO13);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case STATE_BLINK_SLOW:
            gpio_toggle(GPIOC, GPIO13);
            vTaskDelay(pdMS_TO_TICKS(500));
            break;
        case STATE_BLINK_FAST:
            gpio_toggle(GPIOC, GPIO13);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case STATE_ERROR:
            gpio_clear(GPIOC, GPIO13);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        }
    }
}

static char recv_char(void)
{
    while (!usart_get_flag(USART1, USART_SR_RXNE))
        taskYIELD();
    return usart_recv(USART1);
}

static void fsm_task(void *args) {
    (void)args;
    fsm_reset();
    send_line("FSM ready");
    while (1) {
        char c = recv_char();
        char buf[16];
        snprintf(buf, sizeof(buf), "Input: %c", c);
        send_line(buf);
        fsm_handle_event(event_from_char(c));
    }
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    gpio_setup();
    usart_setup();

    BaseType_t ok;
    ok = xTaskCreate(fsm_task, "FSM", 256, NULL, 1, NULL);
    configASSERT(ok == pdPASS);
    ok = xTaskCreate(led_task, "LED", 128, NULL, 1, NULL);
    configASSERT(ok == pdPASS);

    vTaskStartScheduler();
    while (1);
}
