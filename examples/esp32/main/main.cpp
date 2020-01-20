#include "driver/gpio.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <ableton/Link.hpp>

#define LED GPIO_NUM_2

unsigned int if_nametoindex(const char* ifname)
{
  return 0;
}

char* if_indextoname(unsigned int ifindex, char* ifname)
{
  return nullptr;
}

void IRAM_ATTR timer_group0_isr(void* user_param)
{
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  TIMERG0.int_clr_timers.t0 = 1;
  TIMERG0.hw_timer[0].config.alarm_en = 1;

  xSemaphoreGiveFromISR(user_param, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void timer_group0_init(int timer_period_us, void* user_param)
{
    timer_config_t config = {
            .alarm_en = true,
            .counter_en = false,
            .intr_type = TIMER_INTR_LEVEL,
            .counter_dir = TIMER_COUNT_UP,
            .auto_reload = true,
            .divider = 80
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_period_us);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, &timer_group0_isr, user_param, 0, nullptr);

    timer_start(TIMER_GROUP_0, TIMER_0);
}

void tick_task(void* user_param)
{
  ableton::Link link(120.0f);
  link.enable(true);

  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  
  while (true)
  {
    xSemaphoreTake(user_param, portMAX_DELAY);

    const auto state = link.captureAudioSessionState();
    const auto phase = state.phaseAtTime(link.clock().micros(), 1.);
    gpio_set_level(LED, fmodf(phase, 1.) < 0.1);
  }
}

extern "C" void app_main()
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  SemaphoreHandle_t tick_semphr = xSemaphoreCreateBinary();
  timer_group0_init(100, tick_semphr);

  xTaskCreatePinnedToCore(
    tick_task, "tick", 8192, tick_semphr, configMAX_PRIORITIES - 1, nullptr, 1);

  while (true)
  {
    ableton::link::platform::IoContext::poll();
    vTaskDelay(1);
  }
}
