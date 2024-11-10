/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gpscompass);

#include "errors.h"

static const struct gpio_dt_spec sw0_gpio =
    GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


static void initialize_buttons() {
  static struct gpio_callback button_cb_data;

  if (!gpio_is_ready_dt(&sw0_gpio)) {
    error(0, "Button io not ready.");
    return;
  }

  gpio_pin_configure_dt(&sw0_gpio, GPIO_INPUT);

  gpio_pin_interrupt_configure_dt(&sw0_gpio, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&button_cb_data, button_pressed, BIT(sw0_gpio.pin));

  gpio_add_callback(sw0_gpio.port, &button_cb_data);
}

static void initialize_led() {
  if (!gpio_is_ready_dt(&led)) {
    LOG_ERR("GPIO not ready.");
  }

  int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to initialize LED.");
  }
}

static void toggle_led() {
  static bool led_state = true;
  int ret = gpio_pin_toggle_dt(&led);
  if (ret < 0) {
    LOG_ERR("Failed to toggle LED.");
  }
  led_state = !led_state;
}

int main(void) {
  LOG_INF("GPS Compass initializing...");
  initialize_led();
  initialize_buttons();

  LOG_INF("GPS Compass running...");
  int i = 0;
  while (true) {
    if (gpio_pin_get_dt(&sw0_gpio)) {
    }

    ++i;
  }

  LOG_INF("GPS Compass done.");
  return 0;
}
