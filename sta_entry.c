#include <stdio.h>

#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include <unistd.h>
#include "hi_wifi_api.h"
//#include "wifi_sta.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"

#include "lwip/sockets.h"


#include "robot_control.h"
#include "udp_control.h"

#define WIFI_SSID		"TP-LINK_416"
#define WIFI_PASSWD		"31415926"

#define APP_INIT_VAP_NUM    2
#define APP_INIT_USR_NUM    2

volatile char start_wifi_connected_flg = 0;

static struct netif *g_lwip_netif = NULL;

/* clear netif's ip, gateway and netmask */
void hi_sta_reset_addr(struct netif *pst_lwip_netif)
{
    ip4_addr_t st_gw;
    ip4_addr_t st_ipaddr;
    ip4_addr_t st_netmask;
    printf("%s %d \r\n", __FILE__, __LINE__);
    if (pst_lwip_netif == NULL) {
        printf("hisi_reset_addr::Null param of netdev\r\n");
        return;
    }

    IP4_ADDR(&st_gw, 0, 0, 0, 0);
    IP4_ADDR(&st_ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&st_netmask, 0, 0, 0, 0);

    netifapi_netif_set_addr(pst_lwip_netif, &st_ipaddr, &st_netmask, &st_gw);
}

void wifi_wpa_event_cb(const hi_wifi_event *hisi_event)
{
    if (hisi_event == NULL)
        return;

    switch (hisi_event->event) {
        case HI_WIFI_EVT_SCAN_DONE:
            printf("WiFi: Scan results available\n");
            break;
        case HI_WIFI_EVT_CONNECTED:
            printf("WiFi: Connected\n");
            netifapi_dhcp_start(g_lwip_netif);

            start_wifi_connected_flg = 1;

            break;
        case HI_WIFI_EVT_DISCONNECTED:
            printf("WiFi: Disconnected\n");
            netifapi_dhcp_stop(g_lwip_netif);
            hi_sta_reset_addr(g_lwip_netif);
            break;
        case HI_WIFI_EVT_WPS_TIMEOUT:
            printf("WiFi: wps is timeout\n");
            break;
        default:
            break;
    }
}

int hi_wifi_start_connect(void)
{
    int ret;
    errno_t rc;
    hi_wifi_assoc_request assoc_req = {0};

    /* copy SSID to assoc_req */
    //热点名称
    rc = memcpy_s(assoc_req.ssid, HI_WIFI_MAX_SSID_LEN + 1, WIFI_SSID, strlen(WIFI_SSID)); /* 9:ssid length */
    if (rc != EOK) {
        printf("%s %d \r\n", __FILE__, __LINE__);
        return -1;
    }

    /*
     * OPEN mode
     * for WPA2-PSK mode:
     * set assoc_req.auth as HI_WIFI_SECURITY_WPA2PSK,
     * then memcpy(assoc_req.key, "12345678", 8).
     */
    //热点加密方式
    assoc_req.auth = HI_WIFI_SECURITY_WPA2PSK;

    /* 热点密码 */
    memcpy(assoc_req.key, WIFI_PASSWD, strlen(WIFI_PASSWD));


    ret = hi_wifi_sta_connect(&assoc_req);
    if (ret != HISI_OK) {
        printf("%s %d \r\n", __FILE__, __LINE__);
        return -1;
    }
    printf("%s %d \r\n", __FILE__, __LINE__);
    return 0;
}

int hi_wifi_start_sta(void)
{
    int ret;
    char ifname[WIFI_IFNAME_MAX_SIZE + 1] = {0};
    int len = sizeof(ifname);
    const unsigned char wifi_vap_res_num = APP_INIT_VAP_NUM;
    const unsigned char wifi_user_res_num = APP_INIT_USR_NUM;

    printf("%s %d \r\n", __FILE__, __LINE__);

    ret = hi_wifi_init(wifi_vap_res_num, wifi_user_res_num);
    if (ret != HISI_OK) {
        printf("%s %d \r\n", __FILE__, __LINE__);
        //return -1;
    }

    printf("%s %d \r\n", __FILE__, __LINE__);
    ret = hi_wifi_sta_start(ifname, &len);
    if (ret != HISI_OK) {
        printf("%s %d \r\n", __FILE__, __LINE__);
        return -1;
    }

    /* register call back function to receive wifi event, etc scan results event,
     * connected event, disconnected event.
     */
    ret = hi_wifi_register_event_callback(wifi_wpa_event_cb);
    if (ret != HISI_OK) {
        printf("register wifi event callback failed\n");
    }

    /* acquire netif for IP operation */
    g_lwip_netif = netifapi_netif_find(ifname);
    if (g_lwip_netif == NULL) {
        printf("%s: get netif failed\n", __FUNCTION__);
        return -1;
    }

    /* if received scan results, select one SSID to connect */
    ret = hi_wifi_start_connect();
    if (ret != 0) {
        printf("%s %d \r\n", __FILE__, __LINE__);
        return -1;
    }


    return 0;
}

void hi_wifi_stop_sta(void)
{
    int ret;

    ret = hi_wifi_sta_stop();
    if (ret != HISI_OK) {
        printf("failed to stop sta\n");
    }

    ret = hi_wifi_deinit();
    if (ret != HISI_OK) {
        printf("failed to deinit wifi\n");
    }

    g_lwip_netif = NULL;

    
}

void mqtt_test_task(void *arg)
{
    arg = arg;
    
    //开始进入MQTT测试
    //mqtt_test();
}

void MqttEntry(void)
{
    osThreadAttr_t attr1;
    
    attr1.name = "mqtt_task";
    attr1.attr_bits = 0U;
    attr1.cb_mem = NULL;
    attr1.cb_size = 0U;
    attr1.stack_mem = NULL;
    attr1.stack_size = 4096;
    attr1.priority = 26;

    if (osThreadNew((osThreadFunc_t)mqtt_test_task, NULL, &attr1) == NULL) {
        printf("[MqttEntry] Falied to create mqtt_test_task!\n");
    }
}


void mqtt_test_thread(void * argv)
{

    argv = argv;

    hi_wifi_start_sta();

    while(start_wifi_connected_flg == 0)
    {
        usleep(300000);
    }

    sleep(3);
    MqttEntry();
    start_udp_thread();

}



void StaExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "wifi_config_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)mqtt_test_thread, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create LedTask!\n");
    }

}

SYS_RUN(StaExampleEntry);