#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "queue.h"

QueueHandle_t buttonQueue;

const uint button = 5;
const uint led_pin_blue = 11;
const uint led_pin_red = 12;
void setup(){
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_init(led_pin_blue);
    gpio_set_dir(led_pin_blue, GPIO_OUT);

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}
void vbuttonTask(){
    for(;;){
        int buttonState = !gpio_get(button);
        xQueueSend(buttonQueue, &buttonState, portMAX_DELAY);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void vBlinkTask(){
    int buttonState;
    for (;;){
        if(xQueueReceive(buttonQueue, &buttonState, portMAX_DELAY)){
            if (buttonState){
                gpio_put(led_pin_red, 1);
                printf("Aceso"); 
            }else{
            gpio_put(led_pin_red, 0);
            printf("Apagado");
    }
    }
}  
}
int main(void) {
    setup();
    buttonQueue = xQueueCreate(5, sizeof(int));
    if (buttonQueue == NULL){
        printf("Erro ao criar a fila\n");
        for(;;){;}
    }
    
    xTaskCreate(vbuttonTask, "Button Read", 128, NULL, 1, NULL);
    xTaskCreate(vBlinkTask, "Button process", 128, NULL, 1, NULL);
    vTaskStartScheduler();
    for (;;)
    {
        ;
    }
}
