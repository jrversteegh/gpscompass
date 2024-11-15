/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_display.h>
#include <lvgl_mem.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#define DEV0 DEVICE_DT_GET(DT_NODELABEL(sh1106_128_64_0))
#define DEV1 DEVICE_DT_GET(DT_NODELABEL(sh1106_128_64_1))

static uint32_t count;


#ifdef CONFIG_LV_Z_ENCODER_INPUT
static const struct device *lvgl_encoder =
	DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_lvgl_encoder_input));
#endif /* CONFIG_LV_Z_ENCODER_INPUT */


static lv_disp_t* lvgl_init_first(const struct device* dev) {
	if (!device_is_ready(dev)) {
		LOG_ERR("Device 0 not ready");
		return nullptr;
	}

	display_blanking_off(dev);
    return lv_disp_get_default();
}

static int lvgl_allocate_rendering_buffers(lv_disp_drv_t *disp_driver)
{
  void *buf0 = NULL;
  void *buf1 = NULL;
  uint16_t buf_nbr_pixels;
  uint32_t buf_size;
  struct lvgl_disp_data *data = (struct lvgl_disp_data *)disp_driver->user_data;

  disp_driver->hor_res = data->cap.x_resolution;
  disp_driver->ver_res = data->cap.y_resolution;

  buf_nbr_pixels = (CONFIG_LV_Z_VDB_SIZE * disp_driver->hor_res * disp_driver->ver_res) / 100;
  /* one horizontal line is the minimum buffer requirement for lvgl */
  if (buf_nbr_pixels < disp_driver->hor_res) {
    buf_nbr_pixels = disp_driver->hor_res;
  }

  switch (data->cap.current_pixel_format) {
    case PIXEL_FORMAT_ARGB_8888:
      buf_size = 4 * buf_nbr_pixels;
      break;
    case PIXEL_FORMAT_RGB_888:
      buf_size = 3 * buf_nbr_pixels;
      break;
    case PIXEL_FORMAT_RGB_565:
      buf_size = 2 * buf_nbr_pixels;
      break;
    case PIXEL_FORMAT_MONO01:
    case PIXEL_FORMAT_MONO10:
      buf_size = buf_nbr_pixels / 8;
      buf_size += (buf_nbr_pixels % 8) == 0 ? 0 : 1;
      break;
    default:
      return -ENOTSUP;
  }

  buf0 = LV_MEM_CUSTOM_ALLOC(buf_size);
  if (buf0 == NULL) {
    LOG_ERR("Failed to allocate memory for rendering buffer");
    return -ENOMEM;
  }

#ifdef CONFIG_LV_Z_DOUBLE_VDB
  buf1 = LV_MEM_CUSTOM_ALLOC(buf_size);
  if (buf1 == NULL) {
    LV_MEM_CUSTOM_FREE(buf0);
    LOG_ERR("Failed to allocate memory for rendering buffer");
    return -ENOMEM;
  }
#endif

  disp_driver->draw_buf = (lv_disp_draw_buf_t*)LV_MEM_CUSTOM_ALLOC(sizeof(lv_disp_draw_buf_t));
  if (disp_driver->draw_buf == NULL) {
    LV_MEM_CUSTOM_FREE(buf0);
    LV_MEM_CUSTOM_FREE(buf1);
    LOG_ERR("Failed to allocate memory to store rendering buffers");
    return -ENOMEM;
  }

  lv_disp_draw_buf_init(disp_driver->draw_buf, buf0, buf1, buf_nbr_pixels);
  return 0;
}


static lv_disp_t* lvgl_init_second(const struct device* dev) {
	if (!device_is_ready(dev)) {
		LOG_ERR("Device 1 not ready");
		return nullptr;
	}

    static lv_disp_drv_t disp_drv;
    static struct lvgl_disp_data disp_data = {
      .blanking_on = false,
    };

    disp_data.display_dev = dev;
    display_get_capabilities(dev, &disp_data.cap);
    lv_disp_drv_init(&disp_drv);
    disp_drv.user_data = (void *)&disp_data;
    if (lvgl_allocate_rendering_buffers(&disp_drv) != 0) {
        return nullptr;
    }

    if (set_lvgl_rendering_cb(&disp_drv) != 0) {
        LOG_ERR("Display not supported.");
        return nullptr;
    }

    lv_disp_t *display = lv_disp_drv_register(&disp_drv);
    if (display == nullptr) {
        LOG_ERR("Failed to register display device.");
        return nullptr;
    }
    return display;
}

int main(void)
{
    LOG_INF("Starting Display Test");
	char count_str[11] = {0};

	auto display_0 = lvgl_init_first(DEV0);
    auto display_1 = lvgl_init_second(DEV1);
    if (display_0 == nullptr || display_1 == nullptr) {
      LOG_ERR("Display initialization failed");
      return -1;
    }

    lv_obj_t *hello_label = lv_label_create(lv_disp_get_scr_act(display_0));
    lv_obj_t *world_label = lv_label_create(lv_disp_get_scr_act(display_0));
    lv_obj_t *hello_too = lv_label_create(lv_disp_get_scr_act(display_1));

	lv_label_set_text(hello_label, "Hello");
	lv_label_set_text(world_label, "World!");
	lv_obj_align(hello_label, LV_ALIGN_CENTER, 0, -8);
	lv_obj_align(world_label, LV_ALIGN_CENTER, 0, 8);
	lv_label_set_text(hello_too, "Hello Too!");
	lv_obj_align(hello_too, LV_ALIGN_BOTTOM_MID, 0, 0);

	lv_obj_t *count_label = lv_label_create(lv_disp_get_scr_act(display_0));
	lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_t *count_too = lv_label_create(lv_disp_get_scr_act(display_1));
	lv_obj_align(count_too, LV_ALIGN_TOP_MID, 0, 0);

	lv_task_handler();

	while (1) {
		if ((count % 100) == 0U) {
			sprintf(count_str, "%d", count/100U);
			lv_label_set_text(count_label, count_str);
			lv_label_set_text(count_too, count_str);
		}
		lv_task_handler();
		++count;
		k_sleep(K_MSEC(10));
	}
}
