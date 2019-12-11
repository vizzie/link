#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

void tick_task(void* user_param)
{
  auto pLink = static_cast<ableton::Link*>(user_param);
  while (true)
  {
    const auto state = pLink->captureAudioSessionState();
    const auto phase = state.phaseAtTime(pLink->clock().micros(), 1.);
    gpio_set_level(LED, fmodf(phase, 1.) < 0.1);
  }
}

void link_task(void* user_param)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());
  esp_wifi_set_ps(WIFI_PS_NONE);

  ableton::Link link(120.0f);
  link.enable(true);

  xTaskCreatePinnedToCore(
    tick_task, "tick_task", 8192, static_cast<void*>(&link), 10, nullptr, 1);

  while (true)
  {
    vTaskDelay(portMAX_DELAY);
  }
}

extern "C" void app_main()
{
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);

  xTaskCreatePinnedToCore(link_task, "link_task", 8192, nullptr, 10, nullptr, 0);

  vTaskDelete(nullptr);
}
