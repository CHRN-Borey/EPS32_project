#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
////////////////////////////////esp now
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
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"

#define ESPNOW_MAXDELAY 512

typedef enum {
    ESPNOW_EVT_SENT,
    ESPNOW_EVT_RECEIVED
} esp_now_evt_t ;

static const char *TAG = "espnow_master";

static xQueueHandle evt_queue;

static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
// { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
uint8_t sent_mac_addr[ESP_NOW_ETH_ALEN] ;
esp_now_send_status_t sent_status;
uint8_t Data_send[32] = {0};
////////////////////////////////
static const char *TAG_I2C = "i2c-test";

#define CONFIG_ESPNOW_CHANNEL 1
#define SLAVE_BUFFER_LENGTH 512     /*!< Data buffer length of test buffer */
#define DATA_LENGTH 32              /*!< Data buffer length of test buffer */
#define I2CSLAVE_TIMEOUT_MS 5
#define I2C_SLAVE_SCL_IO 19         //4 - metro esp32   11 - feather esp32-s2
#define I2C_SLAVE_SDA_IO 18        //3 - metro esp32   10 - feather esp32-s2
#define I2C_SLAVE_NUM 0              /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * SLAVE_BUFFER_LENGTH) /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * SLAVE_BUFFER_LENGTH) /*!< I2C slave rx buffer size */

#define SLAVE_ADDR 0x04 /*!< Slave address*/




static esp_err_t i2c_slave_init(void)
{
    int i2c_slave_port = I2C_SLAVE_NUM;
    i2c_config_t conf_slave = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = SLAVE_ADDR,
    };
    esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
    ESP_LOGI("slave1", "%d", err);
    err = i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
    ESP_LOGI("slave2", "%d", err);

    err = i2c_reset_rx_fifo(i2c_slave_port);
    ESP_LOGI("slave3", "%d", err);

    err = i2c_reset_tx_fifo(i2c_slave_port);
    ESP_LOGI("slave4", "%d", err);

    return err;
}

static void slaveRead(i2c_port_t i2c_num, uint8_t *data, size_t size)
{

    int read = i2c_slave_read_buffer(i2c_num, data, size, I2CSLAVE_TIMEOUT_MS / portTICK_RATE_MS);

    printf("---- Slave read: [%d] bytes ----\n", read);

    if (read == DATA_LENGTH)
    {
        int i;
        for (i = 0; i < DATA_LENGTH; i++)
        {
            printf("%d ", data[i]);

        }
        Data_send[0] = data[0];
        Data_send[1] = data[1];
        Data_send[2] = data[2];

        printf("\n");
    }
}

static void i2c_slave_task(void *arg)
{
    uint8_t *data = (uint8_t *)malloc(SLAVE_BUFFER_LENGTH);

    while (1)
    {
        slaveRead(I2C_SLAVE_NUM, data, SLAVE_BUFFER_LENGTH);

        vTaskDelay(20 / portTICK_RATE_MS);
    }
    free(data);
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_SLAVE_NUM));
    ESP_LOGI(TAG_I2C, "I2C slave unitialized successfully");
}
///////////////////////////////////////////////////////////esp now
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
    esp_now_evt_t evt;
    memcpy(sent_mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    sent_status = status;
    evt = ESPNOW_EVT_SENT;
    xQueueSend(evt_queue, &evt, ESPNOW_MAXDELAY);
}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{

}

static void espnow_task(void *pvParameter)
{
    esp_now_evt_t evt;
    /* Initialize sending parameters. */


    vTaskDelay(50 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Start sending broadcast data");
    if (esp_now_send(s_broadcast_mac, Data_send, sizeof(Data_send)) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        vTaskDelete(NULL);
    }

    while (xQueueReceive(evt_queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt) {
            case ESPNOW_EVT_SENT: {
                if (sent_status == ESP_NOW_SEND_SUCCESS)
                {
//                    ESP_LOGI(TAG, "Data has been sent to " MACSTR "",
//                                            MAC2STR(sent_mac_addr));
        			ESP_LOGI(TAG, "Data has been sent");

                } else {
                    ESP_LOGE(TAG, "Send error");
                }
                if (esp_now_send(s_broadcast_mac, Data_send, sizeof(Data_send)) != ESP_OK) {
                    ESP_LOGE(TAG, "Send error");
                    vTaskDelete(NULL);
                }
                vTaskDelay(100 / portTICK_RATE_MS);
                break;
            }
            case ESPNOW_EVT_RECEIVED: {
                break;
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

    xTaskCreate(espnow_task, "espnow_task", 2048, NULL, 2, NULL);

    return ESP_OK;
}

////////////////////////////////////////////////////////////////


void app_main(void)
{
	/////////////////////////////////////////
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    espnow_wifi_init();
    espnow_init();
	//////////////////////////////////////////

    ESP_ERROR_CHECK(i2c_slave_init());
    ESP_LOGI(TAG_I2C, "I2C initialized successfully");

    xTaskCreate(&i2c_slave_task, "i2c_slave_task", 2048, NULL, 1, NULL);
}
