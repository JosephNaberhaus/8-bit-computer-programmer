#include "input.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const int delayBeforeVerify = 50;
static const int delayBeforeReEnable = 250;

typedef struct {
    gpio_num_t pin;
    input_callback callback;
} input_t;

static QueueHandle_t gpio_eventt_queue = NULL;

static void input_isr_handler(void* arg) {
    input_t* input = (input_t*) arg;

    // Temporily disable the interupt to to reduce bouncing.
    gpio_intr_disable(input->pin);

    xQueueSendToBackFromISR(gpio_eventt_queue, input, NULL);
}

static void input_task(void* arg) {
    input_t input;
    while (true) {
        if (xQueueReceive(gpio_eventt_queue, &input, portMAX_DELAY)) {
            #ifdef DEBUG
            printf("Task for pin %d\n", input.pin);
            #endif

            // Wait a moment and then make sure that the pin is actually on.
            // This prevents false high edges when turning off.
            vTaskDelay(delayBeforeVerify / portTICK_PERIOD_MS);
            if (gpio_get_level(input.pin) == 1) {
                #ifdef DEBUG
                printf("Calling callback for pin %d\n", input.pin);
                #endif

                input.callback();
            }

            #ifdef DEBUG
            printf ("Task finished for pin %d\n", input.pin);
            #endif

            // Wait a bit and then re-enable the interupt.
            vTaskDelay(delayBeforeReEnable / portTICK_PERIOD_MS);
            #ifdef DEBUG
            printf ("Debounce delay finished for pin %d\n", input.pin);
            #endif
            gpio_intr_enable(input.pin);
        }
    }
}

static void init_input_event_queue() {
    if (gpio_eventt_queue == NULL) {
        #ifdef DEBUG
        printf("Performing ont-time setup of input event gueue\n");
        #endif

        gpio_eventt_queue = xQueueCreate(10, sizeof(input_t));
        xTaskCreate(input_task, "input_task", 4096, NULL, 10, NULL);
    }
}

void set_up_input(gpio_num_t pin, input_callback callback) {
    init_input_event_queue();

    #ifdef DEBUG
    printf("Setting up GPIO pin %d as an input\n", pin);
    #endif

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ull << pin;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_set_intr_type(pin, GPIO_INTR_POSEDGE);

    input_t* input = malloc(sizeof(input));
    input->pin = pin;
    input->callback = callback;

    gpio_isr_handler_add(pin, input_isr_handler, input);
}