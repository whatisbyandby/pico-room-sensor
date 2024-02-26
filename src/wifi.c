#include "pico/cyw43_arch.h"
#include "wifi.h"


bool wifi_connect(const char* ssid, const char* password)
{

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("failed to connect\n");
        return false;
    }

    return true;
}

bool wifi_init()
{
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();
    return true;
}

bool is_wifi_connected()
{
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}