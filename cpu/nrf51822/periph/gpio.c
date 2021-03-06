/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_nrf51822
 * @{
 *
 * @file        gpio.c
 * @brief       Low-level GPIO driver implementation
 *
 * NOTE: this GPIO driver implementation supports due to hardware limitations
 *       only one pin configured as external interrupt source at a time!
 *
 * @author      Christian Kühling <kuehling@zedat.fu-berlin.de>
 * @author      Timo Ziegler <timo.ziegler@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "sched.h"
#include "thread.h"
#include "periph/gpio.h"
#include "periph_conf.h"

typedef struct {
    void (*cb)(void);
} gpio_state_t;

static gpio_state_t gpio_config;

/**
 * @brief helper function to get the pin that corresponds to a GPIO device
 *
 * @param[in] dev       the device the returned pin corresponds to
 *
 * @return              the pin number of for the given device
 * @return              -1 if device unknown
 */
static inline int get_pin(gpio_t dev)
{
    switch (dev) {
#if GPIO_0_EN
        case GPIO_0:
            return GPIO_0_PIN;
            break;
#endif
#if GPIO_1_EN
        case GPIO_1:
            return GPIO_1_PIN;
            break;
#endif
#if GPIO_2_EN
        case GPIO_2:
            return GPIO_2_PIN;
            break;
#endif
#if GPIO_3_EN
        case GPIO_3:
            return GPIO_3_PIN;
            break;
#endif
#if GPIO_4_EN
        case GPIO_4:
            return GPIO_4_PIN;
            break;
#endif
#if GPIO_5_EN
        case GPIO_5:
            return GPIO_5_PIN;
            break;
#endif
#if GPIO_6_EN
        case GPIO_6:
            return GPIO_6_PIN;
            break;
#endif
#if GPIO_7_EN
        case GPIO_7:
            return GPIO_7_PIN;
            break;
#endif
#if GPIO_8_EN
        case GPIO_8:
            return GPIO_8_PIN;
            break;
#endif
#if GPIO_9_EN
        case GPIO_9:
            return GPIO_9_PIN;
            break;
#endif
#if GPIO_10_EN
        case GPIO_10:
            return GPIO_10_PIN;
            break;
#endif
#if GPIO_11_EN
        case GPIO_11:
            return GPIO_11_PIN;
            break;
#endif
#if GPIO_12_EN
        case GPIO_12:
            return GPIO_12_PIN;
            break;
#endif
#if GPIO_13_EN
        case GPIO_13:
            return GPIO_13_PIN;
            break;
#endif
#if GPIO_14_EN
        case GPIO_14:
            return GPIO_14_PIN;
            break;
#endif
#if GPIO_15_EN
        case GPIO_15:
            return GPIO_15_PIN;
            break;
#endif
        case GPIO_UNDEFINED:
        default:
            return -1;
    }
}

int gpio_init_out(gpio_t dev, gpio_pp_t pullup)
{
    int pin = get_pin(dev);
    if (pin < 0) {
        return pin;
    }

    /* set pin to output mode */
    NRF_GPIO->DIRSET = (1 << pin);

    /* set pin to standard configuration */
    NRF_GPIO->PIN_CNF[pin] = 0;

    /* set pull register configuration */
    switch (pullup) {
        case GPIO_NOPULL:
            break;
        case GPIO_PULLUP:
            NRF_GPIO->PIN_CNF[dev] |= (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);
            break;
        case GPIO_PULLDOWN:
            NRF_GPIO->PIN_CNF[dev] |= (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos);
            break;
    }

    return 0;
}

int gpio_init_in(gpio_t dev, gpio_pp_t pullup)
{
    int pin = get_pin(dev);
    if (pin < 0) {
        return pin;
    }

    /* set pin to output mode */
    NRF_GPIO->DIRCLR = (1 << pin);

    /* set pin to standard configuration */
    NRF_GPIO->PIN_CNF[pin] = 0;

    /* set pull register configuration */
    switch (pullup) {
        case GPIO_NOPULL:
            break;
        case GPIO_PULLUP:
            NRF_GPIO->PIN_CNF[dev] |= (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);
            break;
        case GPIO_PULLDOWN:
            NRF_GPIO->PIN_CNF[dev] |= (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos);
            break;
    }

    return 0;
}

int gpio_init_int(gpio_t dev, gpio_pp_t pullup, gpio_flank_t flank, void (*cb)(void))
{
    int res;
    uint32_t pin;

    /* configure pin as input */
    res = gpio_init_in(dev, pullup);
    if (res < 0) {
        return res;
    }

    /* get pin */
    pin = get_pin(dev);         /* no need to check return value, init_in already did */

    /* set interrupt priority and enable global GPIOTE interrupt */
    NVIC_SetPriority(GPIOTE_IRQn, GPIO_IRQ_PRIO);
    NVIC_EnableIRQ(GPIOTE_IRQn);

    /* save callback */
    gpio_config.cb = cb;

    /* reset GPIOTE configuration register to EVENT mode*/
    NRF_GPIOTE->CONFIG[0] = GPIOTE_CONFIG_MODE_Event;

    /* select active pin for external interrupt */
    NRF_GPIOTE->CONFIG[0] |= (pin << GPIOTE_CONFIG_PSEL_Pos);

    /* set active flank */
    switch (flank) {
        case GPIO_FALLING:
            NRF_GPIOTE->CONFIG[0] |= (1 << GPIOTE_CONFIG_POLARITY_Pos);
            break;
        case GPIO_RISING:
            NRF_GPIOTE->CONFIG[0] |= (2 << GPIOTE_CONFIG_POLARITY_Pos);
            break;
        case GPIO_BOTH:
            NRF_GPIOTE->CONFIG[0] |= (3 << GPIOTE_CONFIG_POLARITY_Pos);
            break;
    }

    /* enable external interrupt */
    NRF_GPIOTE->INTENSET |= GPIOTE_INTENSET_IN0_Msk;

    return 0;
}

int gpio_irq_enable(gpio_t dev)
{
    NRF_GPIOTE->INTENSET |= GPIOTE_INTENSET_IN0_Msk;

    return 0;
}

int gpio_irq_disable(gpio_t dev)
{
    NRF_GPIOTE->INTENCLR |= GPIOTE_INTENSET_IN0_Msk;

    return 0;
}

int gpio_read(gpio_t dev)
{
    uint32_t pin;
    int res = -1;

    /* get pin */
    pin = get_pin(dev);
    if (pin < 0) {
        return pin;
    }

    /* read pin value depending if pin is input or output */
    if (NRF_GPIO->DIR & (1 << pin)) {
        res = (NRF_GPIO->OUT & (1 << pin));
    }
    else {
        res = (NRF_GPIO->IN & (1 << pin));
    }

    /* fix issue with negative number if pin 31 is set */
    if (res < -1) {
        res = 1;
    }

    return -1;
}

int gpio_set(gpio_t dev)
{
    int pin = get_pin(dev);
    if (pin < 0) {
        return pin;
    }

    NRF_GPIO->OUTSET = (1 << pin);

    return 0;
}

int gpio_clear(gpio_t dev)
{
    int pin = get_pin(dev);
    if (pin < 0) {
        return pin;
    }

    NRF_GPIO->OUTCLR = (1 << pin);

    return 0;
}

int gpio_toggle(gpio_t dev)
{
    if (gpio_read(dev)) {
        return gpio_clear(dev);
    } else {
        return gpio_set(dev);
    }
}

int gpio_write(gpio_t dev, int value)
{
    if (value) {
        return gpio_set(dev);
    } else {
        return gpio_clear(dev);
    }
}

__attribute__((naked)) void isr_gpiote(void)
{
    ISR_ENTER();
    if (NRF_GPIOTE->EVENTS_IN[0] == 1)
    {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
        gpio_config.cb();
    }
    if (sched_context_switch_request) {
        thread_yield();
    }
    ISR_EXIT();
}
