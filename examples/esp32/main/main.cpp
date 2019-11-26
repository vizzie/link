#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include <ableton/Link.hpp>

#define LED GPIO_NUM_2

unsigned int if_nametoindex(const char *ifname) {
  return 0;
}

char *if_indextoname(unsigned int ifindex, char *ifname) {
  return nullptr;
}

void link_task(void* user_param) {
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());
  ableton::Link link(120.0f);
  link.enable(true);
  while (true) {
    const auto state = link.captureAppSessionState();
    const auto phase = state.phaseAtTime(link.clock().micros(), 1.);
    gpio_set_level(LED, fmodf(phase, 1.) < 0.1);
  }
}

extern "C" void app_main() {
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  xTaskCreatePinnedToCore(
    link_task,
    "link",
    8192,
    nullptr,
    10,
    nullptr, 1);
  vTaskDelete(nullptr);
}
