#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "digoleserial/digoleserial.h"
#include "bigint/bigint.h"
#include "tachometer/tachometer.h"
#include "driver/stdout.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
static volatile os_timer_t loop_timer;
os_event_t user_procTaskQueue[user_procTaskQueueLen];

// forward declarations
static void nop_procTask(os_event_t *events);
void user_init(void);
void loop(void);
static void setup(void);
static char buffer[] = "                                        ";

/**
 * This is the main user program loop
 */
void ICACHE_FLASH_ATTR
loop(void) {
  static uint8_t iterations = 0;
  uint32_t sample = tachometer_getSample();
  bigint_print5Digits(0,sample);
  digoleserial_gotoXY(0,3);
  os_sprintf(buffer,"freq:%dHz      ",sample);
  digoleserial_lcdString(buffer);
  digoleserial_gotoXY(19,3);
  os_sprintf(buffer,"%c",iterations&1?0b10100001:0b11011111);
  digoleserial_lcdString(buffer);
  iterations += 1;
}

static void ICACHE_FLASH_ATTR
setup(void) {
  static uint32_t iterations = 0;
  tachometer_init();
  digoleserial_init(20,4);
  digoleserial_lcdClear();
  digoleserial_enableCursor(false);
  bigint_init();

  if (iterations>=3 && iterations<=7 ) {
    digoleserial_lcdClear();
    digoleserial_gotoXY(0,0);
    digoleserial_lcdString("Digole serial driver");
    digoleserial_gotoXY(0,1);
    digoleserial_lcdString("    for esp8266");
    digoleserial_gotoXY(0,2);
    digoleserial_lcdString("  github.com/eadf/  ");
    digoleserial_gotoXY(0,3);
    digoleserial_lcdString("esp8266_digoleserial");
  } else if (iterations==8){
    digoleserial_lcdClear();
    loop();
  } else if (iterations>8){
    loop();
  }

  iterations += 1;

  // Start loop timer
  os_timer_disarm(&loop_timer);
  os_timer_setfn(&loop_timer, (os_timer_func_t *) loop, NULL);
  os_timer_arm(&loop_timer, 1000, 1);
}

//Do nothing function
static void ICACHE_FLASH_ATTR
nop_procTask(os_event_t *events) {
  os_delay_us(10);
}

//Init function 
void ICACHE_FLASH_ATTR
user_init(void) {
  stdoutInit();

  //Set station mode
  wifi_set_opmode( NULL_MODE );

  // Start setup timer
  os_timer_disarm(&loop_timer);
  os_timer_setfn(&loop_timer, (os_timer_func_t *) setup, NULL);
  os_timer_arm(&loop_timer, 2000, 0);

  //Start no-operation os task
  system_os_task(nop_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
  system_os_post(user_procTaskPrio, 0, 0);
}
