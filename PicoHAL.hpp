#ifndef PICO_HAL_H
#define PICO_HAL_H

#include "RadioLib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdio.h"
#include "pico/time.h"

class PicoHAL : public RadioLibHal {
   public:
    PicoHAL(spi_inst_t* spiChannel, uint misoPin, uint mosiPin, uint sckPin, uint baudrate = 20000000) : RadioLibHal(GPIO_IN, GPIO_OUT, 0, 1, GPIO_IRQ_EDGE_RISE, GPIO_IRQ_EDGE_FALL),
                                                                                                         _spiChannel(spiChannel),
                                                                                                         _baudrate(baudrate),
                                                                                                         _misoPin(misoPin),
                                                                                                         _mosiPin(mosiPin),
                                                                                                         _sckPin(sckPin) {
    }

    void init() override {
        stdio_init_all();

        spi_init(_spiChannel, _baudrate);
        // Format (channel, data bits per transfer, polarity, phase, order)
        spi_set_format(_spiChannel, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_LSB_FIRST);

        gpio_set_function(_misoPin, GPIO_FUNC_SPI);
        gpio_set_function(_sckPin, GPIO_FUNC_SPI);
        gpio_set_function(_mosiPin, GPIO_FUNC_SPI);
    }

    void term() override {
        spi_deinit(_spiChannel);
    }

    void pinMode(uint32_t pin, uint32_t mode) override {
        if (pin == RADIOLIB_NC) {
            return;
        }

        gpio_set_dir(pin, mode);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override {
        if (pin == RADIOLIB_NC) {
            return;
        }

        gpio_put(pin, value);
    }

    uint32_t digitalRead(uint32_t pin) override {
        if (pin == RADIOLIB_NC) {
            return 0;
        }

        return gpio_get(pin);
    }

    void attachInterrupt(uint32_t pin, void (*callback)(void), uint32_t edge) override {
        if (pin == RADIOLIB_NC) {
            return;
        }

        gpio_set_irq_enabled_with_callback(pin, edge, true, (gpio_irq_callback_t)callback);
    }

    void detachInterrupt(uint32_t pin) override {
        gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
    }

    void delay(unsigned long ms) override {
        sleep_ms(ms);
    }

    void delayMicroseconds(unsigned long us) override {
        sleep_us(us);
    }

    unsigned long millis() override {
        return to_ms_since_boot(get_absolute_time());
    }

    unsigned long micros() override {
        return to_us_since_boot(get_absolute_time());
    }

    long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override {
        if (pin == RADIOLIB_NC) {
            return 0;
        }

        this->pinMode(pin, GPIO_IN);
        uint32_t start = this->micros();
        uint32_t curtick = this->micros();

        while (this->digitalRead(pin) == state) {
            if ((this->micros() - curtick) > timeout) {
                return (0);
            }
        }

        return (this->micros() - start);
    }

    void spiBeginTransaction() override {
        // not needed
    }

    void spiEndTransaction() override {
        // not needed
    }

    void spiBegin() override {
        // not needed
    }

    void spiEnd() override {
        // not needed
    }

    void spiTransfer(uint8_t* src, size_t len, uint8_t* dst) override {
        spi_write_read_blocking(this->_spiChannel, src, dst, len);
    }

   private:
    spi_inst_t* _spiChannel;
    uint _baudrate;
    uint _misoPin;
    uint _mosiPin;
    uint _sckPin;
};

#endif