#include "driver/gpio.h"
#include "esp_event.h"
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
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  while (true)
  {
    const auto state = pLink->captureAudioSessionState();
    const auto phase = state.phaseAtTime(pLink->clock().micros(), 1.);
    gpio_set_level(LED, fmodf(phase, 1.) < 0.1);
  }
}

extern "C" void app_main()
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());
  ableton::Link link(120.0f);

  xTaskCreatePinnedToCore(
    tick_task, "tick", 8192, static_cast<void*>(&link), 10, nullptr, 1);

  link.enable(true);

  while (true)
  {
    ableton::link::platform::IoContext::poll();
  }
}
