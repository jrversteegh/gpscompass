/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gpscompass);

#if !DT_NODE_EXISTS(DT_NODELABEL(battery))
#error "Battery voltage input not defined."
#endif

#if !DT_NODE_EXISTS(DT_NODELABEL(off_switch))
#error "Overlay for power off switch not properly defined."
#endif

static constexpr float battery_level_multiplier = 0.00203;
static const struct adc_dt_spec battery = ADC_DT_SPEC_GET(DT_NODELABEL(battery));
static const struct gpio_dt_spec off_switch = GPIO_DT_SPEC_GET(DT_NODELABEL(off_switch), gpios);


bool initialize_battery_level() {

  if (!adc_is_ready_dt(&battery)) {
    LOG_ERR("ADC device for battery level not ready");
    return false;
  }

  int ret = adc_channel_setup_dt(&battery);
  if (ret) {
    LOG_ERR("ADC channel setup for battery level failed: %d", ret);
    return false;
  }
  return true;
}

bool initialize_off_switch() {
  if (!gpio_is_ready_dt(&off_switch)) {
    LOG_ERR("GPIO not ready");
    return false;
  }

  int ret = gpio_pin_configure_dt(&off_switch, GPIO_DISCONNECTED);
  if (ret < 0) {
    LOG_ERR("Failed to configure offswitch to disconnected");
    return false;
  }
  return true;
}

float get_battery_level() {
  uint16_t sample;
  struct adc_sequence sequence = {
    .buffer = &sample,
    .buffer_size = sizeof(sample),
  };
  adc_sequence_init_dt(&battery, &sequence);
  int err = adc_read_dt(&battery, &sequence);
  if (err < 0) {
    LOG_ERR("Could not read: %d", err);
    return 0.0f;
  }
  int32_t val = static_cast<int32_t>(sample);
  err = adc_raw_to_millivolts_dt(&battery, &val);
  if (err < 0) {
    LOG_ERR("Failed to convert to millivolts: %d", err);
    return 0.0f;
  }
  float value = battery_level_multiplier * static_cast<float>(val);
  static float mean = value;
  mean = 0.9f * mean + 0.1f * value;
  return mean;
}

bool switch_off() {
  LOG_INF("Enabling output...");
  int ret = gpio_pin_configure_dt(&off_switch, GPIO_OUTPUT);
  if (ret < 0) {
    LOG_ERR("Failed to configure offswitch to output");
    return false;
  }
  LOG_INF("Switching off...");
  ret = gpio_pin_set_dt(&off_switch, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to switch off");
    return false;
  }
  LOG_INF("Switched off!");
  return true;
}

int main(void) {
  LOG_INF("GPS Compass initializing...");

  if (!initialize_battery_level()) return -1;
  if (!initialize_off_switch()) return -1;

  LOG_INF("GPS Compass running...");
  int i = 0;
  while (true) {

    k_sleep(K_MSEC(1000));
    auto battery_level = get_battery_level();
    LOG_INF("Battery: %f V", static_cast<double>(battery_level));
    if (battery_level < 3.5f) {
      LOG_WRN("Low Battery!");
      switch_off();
      k_fatal_halt(0);
    }
    ++i;
  }

  LOG_INF("GPS Compass done");
  return 0;
}

