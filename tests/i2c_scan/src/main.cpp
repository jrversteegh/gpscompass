/*
 * Copyright (c) 2019-2020 Peter Bigot Consulting, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

int main() {
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
  if (!device_is_ready(i2c_dev)) {
    printk("I2C device not ready\n");
    return -1;
  }
  printk("Scanning I2C addresses...\n");

  // Wait a little to make sure all devices have booted
  k_sleep(K_MSEC(1000));

  for (int i = 1; i < 128; ++i) {
    uint8_t data_0;
    int ret = i2c_reg_read_byte(i2c_dev, i, 0, &data_0);
    if (ret == 0) {
        uint8_t data_1;
        int ret = i2c_reg_read_byte(i2c_dev, i, 1, &data_1);
        if (ret == 0) {
          printk("Data read from device 0x%02X: 0x%02X 0x%02X\n", i, data_0, data_1);
        }
        else {
          printk("Data read from device 0x%02X: 0x%02X\n", i, data_0);
        }
    }
  }
  printk("Done!\n");
  return 0;
}
