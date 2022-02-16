/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "driver/gpio.h"

/////////////////////////////////////////////////////////////
#include "driver/i2c.h"
static const char *TAG_I2C = "i2c-test";

#define DATA_LENGTH 3              /*!< Data buffer length of test buffer */

#define I2C_MASTER_SCL_IO 25        //34 - mtero express    9 - feather esp32-s2
#define I2C_MASTER_SDA_IO 26       //33 - metro express   8 - feather esp32-s2
#define I2C_MASTER_NUM 1            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 5
#define SLAVE_ADDR 0x04 /*!< Slave address*/
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
uint8_t data_i2c[3] = {0};

/////////////////////////////////////////////////////
#define ESPNOW_MAXDELAY 512
#define ONBOARD_LED 2
QueueHandle_t led1;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

uint8_t last_data[32] = {0};
typedef enum {
    ESPNOW_EVT_SENT,
    ESPNOW_EVT_RECEIVED
} esp_now_evt_t ;

static const char *TAG = "espnow_master";

static xQueueHandle evt_queue;

static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN];
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = {0x94, 0xb9, 0x7e, 0xd5, 0x24, 0x4c}; // master address

uint8_t rcv_mac_addr[ESP_NOW_ETH_ALEN];
uint8_t *rcv_data;
uint8_t rcv_data_len;
uint8_t Data[32]={0};

/* WiFi should start before using ESPNOW */
static void espnow_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B |
            WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR) );
#endif
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{

}
static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    esp_now_evt_t evt;
    memcpy(rcv_mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    rcv_data_len = len;
    rcv_data = data;

    evt = ESPNOW_EVT_RECEIVED;
    if(broadcast_mac[0] == rcv_mac_addr[0] &&    // verify address with master
    		broadcast_mac[1] == rcv_mac_addr[1] &&
			broadcast_mac[2] == rcv_mac_addr[2] &&
			broadcast_mac[3] == rcv_mac_addr[3] &&
			broadcast_mac[4] == rcv_mac_addr[4] &&
			broadcast_mac[5] == rcv_mac_addr[5]){

    xQueueSend(evt_queue, &evt, ESPNOW_MAXDELAY);}
}
static void espnow_task(void *pvParameter)
{
    esp_now_evt_t evt;
    while (xQueueReceive(evt_queue, &evt, portMAX_DELAY) == pdTRUE) {

        switch (evt) {
            case ESPNOW_EVT_SENT: {
                break;
            }
            case ESPNOW_EVT_RECEIVED: {
//                ESP_LOGI(TAG, "Data has been received from " MACSTR "",
//                        MAC2STR(rcv_mac_addr));
//                printf("ALL Data:");
//                printf("\n");
                for (int i = 0; i < rcv_data_len; ++i){
//                	printf("Data[%d] = ",i);
//                    printf("%d\n ", rcv_data[i]);
                }
                break;
            }
        }
        if(rcv_data[0] != last_data[0]){
        	last_data[0] = rcv_data[0];
        	Data[0] =rcv_data[0];
        	xQueueSend(led1, &Data[0], 0);
        	printf("ALL Data:\n ");

            for (int i = 0; i < rcv_data_len; ++i){
                	printf("Data[%d] = ",i);
                    printf("%d\n ", rcv_data[i]);
            }
        }
        if(rcv_data[1] != last_data[1]){
        	last_data[1] = rcv_data[1];
        	Data[1] =rcv_data[1];
        	printf("ALL Data:\n ");

            for (int i = 0; i < rcv_data_len; ++i){
                	printf("Data[%d] = ",i);
                    printf("%d\n ", rcv_data[i]);
            }
        }
        if(rcv_data[2] != last_data[2]){
        	last_data[2] = rcv_data[2];
        	Data[2] =rcv_data[2];
        	printf("ALL Data:\n ");

            for (int i = 0; i < rcv_data_len; ++i){
                	printf("Data[%d] = ",i);
                    printf("%d\n ", rcv_data[i]);
            }
        }

    }
}

static esp_err_t espnow_init(void)
{
    evt_queue = xQueueCreate(1, sizeof(esp_now_evt_t));
    if (evt_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(evt_queue);
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = WIFI_IF_STA;
    peer->encrypt = false;
    memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    xTaskCreate(espnow_task, "espnow_task", 2048, NULL, 1, NULL);
    return ESP_OK;
}

void led_task(void *pvParameters){
    uint8_t Led1 = 0;
	gpio_config_t io_config;
	io_config.mode = GPIO_MODE_INPUT_OUTPUT;
	io_config.pin_bit_mask = (1ULL << ONBOARD_LED);
	io_config.pull_down_en = false;
	io_config.pull_up_en = false;
	io_config.intr_type = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&io_config));
	while(true){
        xQueueReceive(led1, &Led1, portMAX_DELAY);
		gpio_set_level(ONBOARD_LED, Led1);
	    ESP_LOGI("LED 1 = ", "%d", Led1);
		vTaskDelay(10 / portTICK_RATE_MS);
	}

}

//////////////////////////////////////////////i2c
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t write_I2C(i2c_port_t i2c_num, uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    err = i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    err = i2c_master_write(cmd, data, size, ACK_CHECK_EN);
    err = i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(i2c_num, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}
static void i2c_master_task(void *arg)
{
    while (1)
    {
    	data_i2c[0]=Data[0];
    	data_i2c[1]=Data[1];
    	data_i2c[2]=Data[2];
        write_I2C(I2C_MASTER_NUM, data_i2c, DATA_LENGTH);
        vTaskDelay(50 / portTICK_RATE_MS);
//        ESP_LOGI(TAG_I2C, "I2C ");

    }
    free(data_i2c);
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG_I2C, "I2C unitialized successfully");
}

void app_main(void)
{

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG_I2C, "I2C initialized successfully");


	led1 = xQueueCreate(1, sizeof(uint16_t));
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    espnow_wifi_init();
    espnow_init();
	xTaskCreatePinnedToCore(&led_task,
			"LED_task",
			2048,
			NULL,
			2,
			NULL,
			0);
    xTaskCreate(&i2c_master_task, "i2c_master_task", 2048, NULL, 2, NULL);

}
