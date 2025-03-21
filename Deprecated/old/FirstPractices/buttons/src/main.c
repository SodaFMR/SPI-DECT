#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_dect_phy.h>


LOG_MODULE_REGISTER(app);

int counter = 5;

void button_handler(uint32_t button_state, uint32_t has_changed)
{
        /* int counter = 5; */
        // int *pointer = &counter;
        
        if (has_changed & button_state & DK_BTN1_MSK) {
                if(counter > 0) {
                  counter--;
                  LOG_INF("Value decremented, now; %d", counter);
                }
                else {
                  LOG_INF("Index can not be lower than 0, resetting to 0");
                  counter = 0;
                }

        }
        if (has_changed & button_state & DK_BTN2_MSK) {
                if(counter < 10) {
                  counter++;
                  LOG_INF("Value incremented, now: %d", counter);
                }
                else {
                  LOG_INF("Index can not be higher than 10, resetting to 10");
                  counter = 10;
                }
        }
        if (has_changed & button_state & DK_BTN3_MSK) {
                LOG_INF("Actual value of the counter is: %d", counter);
        }
        if (has_changed & button_state & DK_BTN4_MSK) {
                LOG_INF("Resetting the counter to initial Value: 5");
                counter = 5;
        }
        
}

int main(void)
{
    printk("Counter program while LEDs rotate\n");
    dk_buttons_init(button_handler);
    dk_leds_init();
    printk("Button 1 decrements the counter\n");
    printk("Button 2 increments the counter\n");
    printk("Button 3 shows the counter\n");
    printk("Button 4 resets the counter\n");

    int i = 0;

/*     if(counter==0){
        dk_set_led_on(DK_LED2);
        dk_set_led_off(DK_LED1);
        dk_set_led_off(DK_LED3);
    } else if(counter==10){
        dk_set_led_on(DK_LED4);
        dk_set_led_off(DK_LED1);
        dk_set_led_off(DK_LED3);
    } else if(i%2==0) {
        dk_set_led_on(DK_LED1);
        dk_set_led_off(DK_LED2);
        dk_set_led_off(DK_LED4);
    } else if (i%2!=0) {
        dk_set_led_off(DK_LED3);
        dk_set_led_off(DK_LED2);
        dk_set_led_off(DK_LED4);
    } */

    while(1) {
        k_msleep(1000);
        dk_set_led_off(DK_LED4);
        dk_set_led_on(DK_LED1);
        k_msleep(1000);
        dk_set_led_off(DK_LED1);
        dk_set_led_on(DK_LED2);
        k_msleep(1000);
        dk_set_led_off(DK_LED2);
        dk_set_led_on(DK_LED3);
        k_msleep(1000);
        dk_set_led_off(DK_LED3);
        dk_set_led_on(DK_LED4);
    }
    
/*     while (1) {
        k_msleep(1000);
      if(i%2==0) {
        dk_set_led_on(DK_LED1);
        dk_set_led_on(DK_LED2);
      }
      else {
        dk_set_led_off(DK_LED1);
        dk_set_led_off(DK_LED2);
      }
        i++;
    } */
}