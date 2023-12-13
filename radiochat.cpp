#define RADIOLIB_SPI_SLOWDOWN
#define RADIOLIB_TONE_UNSUPPORTED

#include <stdio.h>

#include "PicoHAL.hpp"
#include "RadioLib.h"
#include "pico/stdio_usb.h"

#define CS_PIN 8     // You can choose the GPIO pin for Chip Select (CS)
#define SCK_PIN 18   // SPI clock pin
#define MOSI_PIN 19  // SPI MOSI pin
#define MISO_PIN 16  // SPI MISO pin
#define DIO0_PIN 7
#define DIO1_PIN 10
#define NSS_PIN 9
#define SPI_BUS spi0

PicoHAL* hal = new PicoHAL(spi0, MISO_PIN, MOSI_PIN, SCK_PIN, 8000000);
SX1276 radio = new Module(hal, CS_PIN, DIO0_PIN, DIO1_PIN, RADIOLIB_NC);

int main() {
    stdio_init_all();
    gpio_init(CS_PIN);
    gpio_init(NSS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_set_dir(NSS_PIN, GPIO_OUT);

    while (!stdio_usb_connected()) {
        sleep_ms(500);
    }

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
        // send a packet
        printf("[SX1261] Transmitting packet ... ");
        state = radio.transmit("[KD2ZGA] Hello World!");
        if (state == RADIOLIB_ERR_NONE) {
            // the packet was successfully transmitted
            printf("success!\n");

            // wait for a second before transmitting again
            hal->delay(1000);

        } else {
            printf("failed, code %d\n", state);
        }
    }

    return (0);
}