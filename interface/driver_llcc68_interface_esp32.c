/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_llcc68_interface_template.c
 * @brief     driver llcc68 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/04/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_llcc68_interface.h"

spi_device_handle_t spi_lora_dev_handle = NULL;

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t llcc68_interface_spi_init(void)
{
	// #define LORA_BUSY_IO GPIO_NUM_36
	// #define LORA_DIO1_IO GPIO_NUM_37
	// #define LORA_DIO3_IO GPIO_NUM_38
	// #define LORA_SPI_CS_IO GPIO_NUM_7
	// #define LORA_RESET_IO GPIO_NUM_8

	esp_err_t retval;

	spi_bus_config_t spi_bus_cfg = {.miso_io_num = GPIO_NUM_34,
									.mosi_io_num = GPIO_NUM_35,
									.sclk_io_num = GPIO_NUM_33,
									.quadwp_io_num = -1,
									.quadhd_io_num = -1};
	// .max_transfer_sz = 2048

	retval = spi_bus_initialize(SPI3_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO);

	if (retval != ESP_OK)
	{
		ESP_LOGE("llcc68_SPI_init", "SPI bus couldn't init!");
		return 1;
	}

	spi_device_interface_config_t lora_dev = {.clock_speed_hz = 9000000,
											  .mode = 0,
											  .spics_io_num = GPIO_NUM_7,
											  .queue_size = 1,
											  .flags = 0,
											  .pre_cb = NULL};

	retval = spi_bus_add_device(SPI3_HOST, &lora_dev, &spi_lora_dev_handle);

	if (retval != ESP_OK)
	{
		ESP_LOGE("llcc68_SPI_init", "SPI device couldn't be added to bus!");
		return 1;
	}

	return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t llcc68_interface_spi_deinit(void)
{
	esp_err_t ret = ESP_OK;

	// Remove SPI device handle
	if (spi_lora_dev_handle)
	{
		ret = spi_bus_remove_device(spi_lora_dev_handle);
		if (ret != ESP_OK)
		{
			ESP_LOGE("SPI_DEINIT", "Failed to remove SPI device");
			return 1;
		}
		spi_lora_dev_handle = NULL;
	}

	// Free SPI bus (uninstall SPI driver)
	ret = spi_bus_free(SPI3_HOST);
	if (ret != ESP_OK)
	{
		ESP_LOGE("SPI_DEINIT", "Failed to free SPI bus");
		return 1;
	}
	return 0;
}

/**
 * @brief      interface spi bus write read
 * @param[in]  *in_buf pointer to a input buffer
 * @param[in]  in_len input length
 * @param[out] *out_buf pointer to a output buffer
 * @param[in]  out_len output length
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t llcc68_interface_spi_write_read(uint8_t* in_buf, uint32_t in_len,
										uint8_t* out_buf, uint32_t out_len)
{
	spi_transaction_t t = {0};

	if (spi_lora_dev_handle == NULL)
	{
		ESP_LOGE("llcc68_spi_wr_rd", "spi lora dev not initialized?");
		return 1;
	}

	if ((in_len == 0) && (out_len == 0))
	{
		// Nothing to do
		return 0;
	}

	// use highest length of rx/tx
	uint32_t bytes = (in_len > out_len) ? in_len : out_len;
	t.length = bytes * 8;
	t.tx_buffer = (in_len > 0) ? in_buf : NULL;
	t.rx_buffer = (out_len > 0) ? out_buf : NULL;
	t.flags = 0;

	esp_err_t ret = spi_device_transmit(spi_lora_dev_handle, &t);
	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_wr_rd", "spi_device_transmit failed: %d", ret);
		return 1;
	}
	return 0;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_init(void)
{
	esp_err_t ret = gpio_reset_pin(GPIO_NUM_8);
	ret |= gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);
	// ret |= gpio_set_level(GPIO_NUM_8, 0);

	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_gpio_rst", "Reset gpio init failed!");
		return 1;
	}
	return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_deinit(void)
{
	esp_err_t ret = gpio_reset_pin(GPIO_NUM_8);

	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_gpio_rst", "Reset gpio init failed!");
		return 1;
	}
	return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] data written data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t llcc68_interface_reset_gpio_write(uint8_t data)
{
	uint32_t gpio_val = data ? 0 : 1;
	esp_err_t ret = gpio_set_level(GPIO_NUM_8, gpio_val);

	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_gpio_rst", "Reset gpio write failed!");
		return 1;
	}
	return 0;
}

/**
 * @brief  interface busy gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_init(void)
{
	esp_err_t ret = gpio_reset_pin(GPIO_NUM_36);
	ret |= gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);

	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_gpio_bsy", "Busy gpio init failed!");
		return 1;
	}
	return 0;
}

/**
 * @brief  interface busy gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_deinit(void)
{
	esp_err_t ret = gpio_reset_pin(GPIO_NUM_36);

	if (ret != ESP_OK)
	{
		ESP_LOGE("llcc68_spi_gpio_bsy", "Busy gpio deinit failed!");
		return 1;
	}
	return 0;
}

/**
 * @brief      interface busy gpio read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t llcc68_interface_busy_gpio_read(uint8_t* value)
{
	int readval = gpio_get_level(GPIO_NUM_36);
	*value = (uint8_t)readval;
	return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void llcc68_interface_delay_ms(uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void llcc68_interface_debug_print(const char* const fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char buf[128];	// choose a size that fits your logs
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	ESP_LOGW("llcc68", "%s", buf);
}

/**
 * @brief     interface receive callback
 * @param[in] type receive callback type
 * @param[in] *buf pointer to a buffer address
 * @param[in] len buffer length
 * @note      none
 */
void llcc68_interface_receive_callback(uint16_t type, uint8_t* buf,
									   uint16_t len)
{
	switch (type)
	{
		case LLCC68_IRQ_TX_DONE:
		{
			llcc68_interface_debug_print("llcc68: irq tx done.\n");

			break;
		}
		case LLCC68_IRQ_RX_DONE:
		{
			llcc68_interface_debug_print("llcc68: irq rx done.\n");

			break;
		}
		case LLCC68_IRQ_PREAMBLE_DETECTED:
		{
			llcc68_interface_debug_print("llcc68: irq preamble detected.\n");

			break;
		}
		case LLCC68_IRQ_SYNC_WORD_VALID:
		{
			llcc68_interface_debug_print(
				"llcc68: irq valid sync word detected.\n");

			break;
		}
		case LLCC68_IRQ_HEADER_VALID:
		{
			llcc68_interface_debug_print("llcc68: irq valid header.\n");

			break;
		}
		case LLCC68_IRQ_HEADER_ERR:
		{
			llcc68_interface_debug_print("llcc68: irq header error.\n");

			break;
		}
		case LLCC68_IRQ_CRC_ERR:
		{
			llcc68_interface_debug_print("llcc68: irq crc error.\n");

			break;
		}
		case LLCC68_IRQ_CAD_DONE:
		{
			llcc68_interface_debug_print("llcc68: irq cad done.\n");

			break;
		}
		case LLCC68_IRQ_CAD_DETECTED:
		{
			llcc68_interface_debug_print("llcc68: irq cad detected.\n");

			break;
		}
		case LLCC68_IRQ_TIMEOUT:
		{
			llcc68_interface_debug_print("llcc68: irq timeout.\n");

			break;
		}
		default:
		{
			llcc68_interface_debug_print("llcc68: unknown code.\n");

			break;
		}
	}
}
