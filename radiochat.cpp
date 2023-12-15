#define RADIOLIB_SPI_SLOWDOWN
#define RADIOLIB_TONE_UNSUPPORTED

#include <stdio.h>
#include <string.h>

#include "PicoHAL.hpp"
#include "RadioLib.h"
#include "pico/multicore.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "tcpip.hpp"

#define CS_PIN 8     // You can choose the GPIO pin for Chip Select (CS)
#define SCK_PIN 18   // SPI clock pin
#define MOSI_PIN 19  // SPI MOSI pin
#define MISO_PIN 16  // SPI MISO pin
#define DIO0_PIN 7
#define DIO1_PIN 10
#define NSS_PIN 9
#define SPI_BUS spi0

#define DEBUG_PIN 1

#define HTML_BODY "<html><head></head><body><h1>Radiochat</h1><p>Last message received:</p><pre><code>%s</code></pre><form action=\"/send_message\" method=\"POST\"><input name=\"message\" type=\"text\" /><input type=\"submit\" value=\"Send!\" /></form></body></html>"

PicoHAL* hal = new PicoHAL(spi0, MISO_PIN, MOSI_PIN, SCK_PIN, 8000000);
SX1276 radio = new Module(hal, CS_PIN, DIO0_PIN, RADIOLIB_NC, DIO1_PIN);

uint32_t spinlock_num_radio;
uint32_t spinlock_num_message;
spin_lock_t *lock_radio;
spin_lock_t *lock_message;

char message[64];
char compose[64];
volatile int message_in_outbox = 0;

static int handle_request(int is_post, const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    if (is_post) {
        printf("This is where we send the message. Parsing params\n%s\n", params);
        sscanf(params, "message=%s", compose);
        printf("Broadcasting the message: %s\n", compose);
        // we're sending the message here.

        // Send the new message
        // printf("1 Core %d acquiring radio lock\n", get_core_num());
        // spin_lock_unsafe_blocking(lock_radio);
        // printf("1 Core %d got radio lock\n", get_core_num());

        // int status = radio.transmit(compose);
        // if (status != RADIOLIB_ERR_NONE) {
        //     printf("Radiolib error %d\n", status);
        // }

        message_in_outbox = 1;

        // printf("1 Core %d releasing radio lock\n", get_core_num());
        // spin_unlock_unsafe(lock_radio);
        // printf("1 Core %d released radio lock\n", get_core_num());
    }

    // Get the last message
    uint32_t lock = spin_lock_blocking(lock_message);
    len = snprintf(result, max_result_len, HTML_BODY, message);
    spin_unlock(lock_message, lock);

    return len;
}

void core1_main() {
    spawn_server(&handle_request);
    for (;;)
        tight_loop_contents();
}

int main() {
    stdio_init_all();
    gpio_init(CS_PIN);
    gpio_init(NSS_PIN);
    gpio_init(DEBUG_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_set_dir(NSS_PIN, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN, GPIO_OUT);

    spinlock_num_radio = spin_lock_claim_unused(true);
    spinlock_num_message = spin_lock_claim_unused(true);

    lock_radio = spin_lock_init(spinlock_num_radio);
    lock_message = spin_lock_init(spinlock_num_message);

    // while (!stdio_usb_connected()) {
    //     sleep_ms(500);
    // }

    printf("Spawning other thread!\n");
    multicore_launch_core1(&core1_main);

    sleep_ms(10);
    gpio_put(NSS_PIN, 0);
    sleep_ms(10);
    gpio_put(NSS_PIN, 1);

    printf("[SX1276] Initializing ... ");
    int state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        printf("(%d) failed, code %d\n", state);
        return (1);
    }

    printf("success!\n");

    radio.setFrequency(435.000);

    // loop forever
    for (;;) {
        // printf("0 Core %d acquiring radio lock\n", get_core_num());
        // spin_lock_unsafe_blocking(lock_radio);
        // printf("0 Core %d got radio lock\n", get_core_num());
        // send a packet
        uint8_t data[64];
        int state = radio.receive(data, 64);

        if (message_in_outbox) {
            printf("Sending...");
            int status = radio.transmit(compose);
            if (status != RADIOLIB_ERR_NONE) {
                printf("Radiolib error %d\n", status);
            }
            printf("Done\n");
            message_in_outbox = 0;
        }

        // printf("0 Core %d releasing radio lock\n", get_core_num());
        // spin_unlock_unsafe(lock_radio);
        // printf("0 Core %d released radio lock\n", get_core_num());

        if (state == RADIOLIB_ERR_NONE) {
            uint32_t lock = spin_lock_blocking(lock_message);
            // the packet was successfully transmitted
            printf("success! received data: %s\n", data);
            strcpy(message, (char *)data);
            spin_unlock(lock_message, lock);

        } else if (state != RADIOLIB_ERR_RX_TIMEOUT) {
            printf("failed, code %d\n", state);
        }

        sleep_ms(1);
    }

    return (0);
}