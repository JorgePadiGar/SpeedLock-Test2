//Parte de inicializacion de las librerias ** si tienen un guion se refiere a la carpeta y el documento que se esta usando 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "nvs.h"   //memoria flash ADC tambien se va necesitar para las
#include "nvs_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <driver/ledc.h>
#include <driver/gpio.h>
#include "app_nvs.h"
#include "app_adc_dac.h"
#include "app_button.h"
#include "app_led.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"


#include "time.h"
#include "sys/time.h"

#define MAXIMO 2
#define MUESTRAS 12

#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP32_BT"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
#define SPP_SHOW_MODE SPP_SHOW_DATA /*Choose show mode: show data or speed*/


const char *tag = "Bluetooth";


static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

ledc_channel_config_t pwm_1;
ledc_channel_config_t pwm_2;

bool btn_level = 0;
bool btn_level2 = 0;
bool btn_level3 = 0;

int duty_cicle_1 = 0;


void init_pwm(void);
void app_main_BT(void);
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);


static void print_speed(void);
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void app_main(void)
{
    //Esta parte es para la activacion de las PWM si esta en 1 abre el bluetooth que es lo que se configura el duty cycle y la frecuencia 
    //y otras cosas mas 

    btn_level2 = gpio_get_level(GPIO_NUM_21);
   
		if (btn_level2 == 1)
            {                 
                    
                app_main_BT();
            }
        else
            {
                btn_level2 = 0;


    static esp_err_t main_err;

    static const uint8_t g_btn = GPIO_NUM_23, g_led = GPIO_NUM_32;
    static uint8_t g_adc_pines[MAXIMO] = {ADC1_CHANNEL_6, ADC1_CHANNEL_7};
    static uint8_t g_dac_val[MAXIMO], bits_adc[MAXIMO], R_Signal[1];
    static uint32_t g_adc_val[MAXIMO], adc1_array[MUESTRAS], adc2_array[MUESTRAS];
    static int sum1 = 0, sum2 = 0, prom_ADC1, prom_ADC2;

    gpio_reset_pin(GPIO_NUM_14);
    gpio_reset_pin(GPIO_NUM_12);

    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(GPIO_NUM_18, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLDOWN_ONLY);

    printf("Setup\n");

    main_err = app_nvs_init();
    if (main_err != ESP_OK)
    {
        //Que acciones tomar si existe un problema?
        // esp_restart();
    }

    main_err = app_button_init(g_btn);
    if (main_err != ESP_OK)
    {
       //Que acciones tomar si existe un problema?
    }

    main_err = app_led_init(g_led);
    if (main_err != ESP_OK)
    {
       //Que acciones tomar si existe un problema?
    }

    main_err = app_adc_init(g_adc_pines);
    if (main_err != ESP_OK)
    {
         //Que acciones tomar si existe un problema?
    }

    main_err = app_dac_init();
    if (main_err != ESP_OK)
    {
         //Que acciones tomar si existe un problema?
    }

    main_err = app_nvs_load_signal(R_Signal);
    if (main_err != ESP_OK)
    {
        //Que acciones tomar si existe un problema?
    }

    main_err = app_nvs_load_adc(g_dac_val);
    printf("%d\n", main_err);
    if (main_err != ESP_OK)
    {
        //Que acciones tomar si existe un problema?
    }

    main_err = app_adc_calib();
    if (main_err != ESP_OK)
    {
        //Que acciones tomar si existe un problema?
    }

    //AQUI HAY QUE PONER UNA FUNCION ASI COMO EL DEL BLUETOOTH Y ESO PARA LLAMARLO A LLAMAR Y NO SE CRASHEEE
    while (1)

    {

        btn_level = gpio_get_level(GPIO_NUM_4);
        printf("Button State: %i \n", btn_level);

        if (btn_level == 1)
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 1);
            sum1 = 0;
            sum2 = 0;
            prom_ADC1 = 0;
            prom_ADC2 = 0;

            main_err = app_adc_calib();
            if (main_err != ESP_OK)
            {
                //Que acciones tomar si existe un problema?
            }

            main_err = app_adc_read_milli_voltage(g_adc_val);
            if (main_err != ESP_OK)
            {
              //Que acciones tomar si existe un problema?
            }

            if (g_adc_val[0] && g_adc_val[1] < 180)
            {
                gpio_set_level(GPIO_NUM_12, 1);
            }
            else if (g_adc_val[0] && g_adc_val[1] > 180)
            {
                gpio_set_level(GPIO_NUM_12, 0);
            }

            g_adc_val[0] = 0;
            g_adc_val[1] = 0;

            for (int i = 0; i < 12; i++)
            {
                //PARTE QUE LEE MILIVOLTAJES TAMBIEN HAY QUE HACER UNA FUNCIONNNN
                main_err = app_adc_read_milli_voltage(g_adc_val);
                if (main_err != ESP_OK)
                {
                    //Que acciones tomar si existe un problema?
                }
                adc1_array[i] = g_adc_val[0];
                adc2_array[i] = g_adc_val[1];
                sum1 += adc1_array[i];
                sum2 += adc2_array[i];

                printf("ADC1: %i \t ADC2: %i \n", adc1_array[i], adc2_array[i]);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            prom_ADC1 = sum1 / MUESTRAS;
            prom_ADC2 = sum2 / MUESTRAS;
            gpio_set_level(GPIO_NUM_2, 0);
            gpio_set_level(GPIO_NUM_12, 0);

            printf("\n P_ADC1: %i \t P_ADC2: %i \n", prom_ADC1, prom_ADC2);

            bits_adc[0] = (prom_ADC1 * 256) / 3100;
            bits_adc[1] = (prom_ADC2 * 256) / 3100;

            if (g_adc_val[0] && g_adc_val[1] < 1200) // El rango de error es mayor entre 0 y 2.5 V, por eso la compensación es mayor.
            {
                // Compensación negativa para mejorar resoución:
                g_dac_val[0] = bits_adc[0] - 3;
                g_dac_val[1] = bits_adc[1] - 3;
            }

            else if (g_adc_val[0] && g_adc_val[1] > 1200)
            {
                // Compensación negativa para mejorar resoución:
                g_dac_val[0] = bits_adc[0] - 4;
                g_dac_val[1] = bits_adc[1] - 7;
            }

            btn_level2 = gpio_get_level(GPIO_NUM_18);
            
            if (btn_level2 == 1)
            {
                R_Signal[0] = 0;
                main_err = app_nvs_save_signal(R_Signal);
                if (main_err != ESP_OK)
                {
                    printf("Valor: nulo");
                    //Que acciones tomar si existe un problema?
                }
            }
            else
            {
                R_Signal[0] = 1;
                main_err = app_nvs_save_signal(R_Signal);
                if (main_err != ESP_OK)
                {
                    printf("Valor: nulo");
                    //Que acciones tomar si existe un problema?
                }
            }

            printf("Valor: %d\n", R_Signal[0]);

            if (((bits_adc[0] - 4) < 70 && (bits_adc[0] - 8) < 66) && R_Signal[0] == 1)
            {

                main_err = app_nvs_save_adc(g_dac_val);
                if (main_err != ESP_OK)
                {
                   //Que acciones tomar si existe un problema?
                }

                main_err = app_dac_write(g_dac_val[0], g_dac_val[1]);
                if (main_err != ESP_OK)
                {
                   //Que acciones tomar si existe un problema?
                }
            }

            else if (((bits_adc[0] - 4) > 0 && (bits_adc[0] - 8) > 0) && R_Signal[0] == 0)
            {
                main_err = app_nvs_save_adc(g_dac_val);
                if (main_err != ESP_OK)
                {
                    //Que acciones tomar si existe un problema?
                }

                main_err = app_dac_write(g_dac_val[0], g_dac_val[1]);
                if (main_err != ESP_OK)
                {
                   //Que acciones tomar si existe un problema?
                }
            }
        }
        else
        {
            switch (R_Signal[0])
            {
            // declarations
            // . . .
            case 1:
                // statements executed if the expression equals the
                // value of this constant_expression
                gpio_set_level(GPIO_NUM_14, 0);
                break;
            default:
                // statements executed if expression does not equal
                // any case constant_expression
                gpio_set_level(GPIO_NUM_14, 1);
            }
            
            main_err = app_dac_write(g_dac_val[0], g_dac_val[1]);
            if (main_err != ESP_OK)
            {
                //Que acciones tomar si existe un problema?
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

    }
	
  
}

void init_pwm(void)
{

    
    //printf("Resultado dutycicle: %i\n",duty_cicle_1);
    int pwm_duty_1 = duty_cicle_1 * 4096 / 100;
    int pwm_duty_2 = 4096 - pwm_duty_1;
    
    printf("Resultado pwm_duty_1: %i\n",pwm_duty_1);
    //printf("Resultado pwm_duty_1: %i\n",pwm_duty_2);

    ledc_timer_config_t timer_1 = {
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 240,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };

     ledc_timer_config_t timer_2 = {
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 240,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&timer_1);        
    //vTaskDelay(150 / portTICK_PERIOD_MS);
    vTaskDelay(190 / portTICK_PERIOD_MS);
    ledc_timer_config(&timer_2);

    pwm_1.channel = LEDC_CHANNEL_0;
    pwm_1.duty = pwm_duty_2;  //original
    pwm_1.gpio_num = GPIO_NUM_18;
    pwm_1.hpoint = 0;
    pwm_1.timer_sel = LEDC_TIMER_0;
    pwm_1.speed_mode = LEDC_HIGH_SPEED_MODE; //cambios

    pwm_2.channel = LEDC_CHANNEL_1;
    pwm_2.duty = pwm_duty_1;   //Original
    pwm_2.gpio_num = GPIO_NUM_19;
    pwm_2.hpoint = 0;
    pwm_2.timer_sel = LEDC_TIMER_1;
    pwm_2.speed_mode = LEDC_HIGH_SPEED_MODE;

    ledc_channel_config(&pwm_1);
    //vTaskDelay(190 / portTICK_PERIOD_MS);
    ledc_channel_config(&pwm_2);
    return ESP_OK;
}

static void print_speed(void)
{
    float time_old_s = time_old.tv_sec + time_old.tv_usec / 1000000.0;
    float time_new_s = time_new.tv_sec + time_new.tv_usec / 1000000.0;
    float time_interval = time_new_s - time_old_s;
    float speed = data_num * 8 / time_interval / 1000.0;
    ESP_LOGI(tag, "speed(%fs ~ %fs): %f kbit/s", time_old_s, time_new_s, speed);
    data_num = 0;
    time_old.tv_sec = time_new.tv_sec;
    time_old.tv_usec = time_new.tv_usec;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_INIT_EVT");
        esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(tag, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(tag, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(tag, "ESP_SPP_CLOSE_EVT");
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(tag, "ESP_SPP_START_EVT");
        esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
#if (SPP_SHOW_MODE == SPP_SHOW_DATA)
        ESP_LOGI(tag, "ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);

        duty_cicle_1 = 0;

        int bandera = param->data_ind.len-2;

        for (size_t i = 0; i < (param->data_ind.len) - 2; i++)
        {
            char value = param->data_ind.data[i];
            int comoEntero = value - '0';


            if(bandera == 4){
            duty_cicle_1 = duty_cicle_1 + (comoEntero * 1000);
            }
            if(bandera == 3){
            duty_cicle_1 = duty_cicle_1 + (comoEntero * 100);
            }
            if(bandera == 2){
            duty_cicle_1 = duty_cicle_1 + (comoEntero * 10);
            }
            if(bandera == 1){
            duty_cicle_1 = duty_cicle_1 + (comoEntero);
            }

            bandera = bandera-1;
            
        }
       
        init_pwm();
        //printf("Resultado dutycicle: %i\n",duty_cicle_1);
        printf("\n");

        esp_spp_write(param->data_ind.handle, param->data_ind.len, param->data_ind.data);
#else
        gettimeofday(&time_new, NULL);
        data_num += param->data_ind.len;
        if (time_new.tv_sec - time_old.tv_sec >= 3)
        {
            print_speed();
        }
#endif
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(tag, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(tag, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(tag, "ESP_SPP_SRV_OPEN_EVT");
        gettimeofday(&time_old, NULL);
        break;
    case ESP_SPP_SRV_STOP_EVT:
        ESP_LOGI(tag, "ESP_SPP_SRV_STOP_EVT");
        break;
    case ESP_SPP_UNINIT_EVT:
        ESP_LOGI(tag, "ESP_SPP_UNINIT_EVT");
        break;
    default:
        break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
    {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(tag, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(tag, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        }
        else
        {
            ESP_LOGE(tag, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:
    {
        ESP_LOGI(tag, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit)
        {
            ESP_LOGI(tag, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        }
        else
        {
            ESP_LOGI(tag, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(tag, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
        break;

    default:
    {
        ESP_LOGI(tag, "event: %d", event);
        break;
    }
    }
    return;
}

void app_main_BT(void)
{
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(tag, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(tag, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK)
    {
        ESP_LOGE(tag, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}