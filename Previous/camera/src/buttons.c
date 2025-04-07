#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_dect_phy.h>
//#include "broadcast.h"
#include "tx_rx.h"



LOG_MODULE_REGISTER(app);


void button_handler(uint32_t button_state, uint32_t has_changed)
{
        if (has_changed & button_state & DK_BTN1_MSK) {
                LOG_INF("Button 1 pressed");
                LOG_INF("Initializing broadcast demo....");
                //demo();
        }
        if (has_changed & button_state & DK_BTN2_MSK) {
                LOG_INF("Button 2 pressed");
                LOG_INF("Initializing data transmission....");
                simple_tx();
        }
        if (has_changed & button_state & DK_BTN3_MSK) {
                LOG_INF("Button 3 pressed");
                LOG_INF("Initializing data reception....");
                simple_rx();
        }
        if (has_changed & button_state & DK_BTN4_MSK) {
                LOG_INF("Button 4 pressed");
        }
        
}

int main(void)
{
    printk("Hello World!\n");
    dk_buttons_init(button_handler);
    dk_leds_init();
    printk("Press the buttons!\n");
    int i = 0;
    
     


    while (1) {
        k_msleep(330);
      if(i%2==0) {
        dk_set_led_on(DK_LED1);
        dk_set_led_on(DK_LED2);
      }
      else {
        dk_set_led_off(DK_LED1);
        dk_set_led_off(DK_LED2);
      }
        i++;
        if(i==10) i=0;
        if(i == INT_MAX) return 0;
    }

}