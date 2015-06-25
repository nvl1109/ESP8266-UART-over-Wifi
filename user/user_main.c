/*
 * File : user_main.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.   If not, see <http://www.gnu.org/licenses/>.
 */
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "driver/uart.h"
#include "task.h"
#include "smartconfig.h"
#include "gpio.h"
#include "user_config.h"
#include "user_interface.h"

#include "server.h"
#include "config.h"
#include "flash_param.h"

os_event_t      recvTaskQueue[recvTaskQueueLen];
extern  serverConnData connData[MAX_CONN];

#define MAX_UARTBUFFER (MAX_TXBUFFER/4)
static uint8 uartbuffer[MAX_UARTBUFFER];

// Functions
void user_rf_pre_init(void)
{
}

static void ICACHE_FLASH_ATTR recvTask(os_event_t *events)
{
    uint8_t i;
    while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
    {
        WRITE_PERI_REG(0X60000914, 0x73); //WTD
        uint16 length = 0;
        while ((READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) && (length<MAX_UARTBUFFER))
            uartbuffer[length++] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
        for (i = 0; i < MAX_CONN; ++i)
            if (connData[i].conn)
                espbuffsent(&connData[i], uartbuffer, length);
    }

    if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
    {
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
    }
    else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
    {
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
    }
    ETS_UART_INTR_ENABLE();
}

// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;
static volatile os_timer_t connectTimer;
static flash_param_t *flash_param;

void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *evt)
{
    uint8 i;
    switch (evt->event) {
        case EVENT_STAMODE_CONNECTED:
        // Disable timer when connected
        os_timer_disarm(&connectTimer);
        break;

        case EVENT_STAMODE_GOT_IP:
        // Disable timer when connected
        os_timer_disarm(&connectTimer);
        serverInit(flash_param->port);

        #ifdef CONFIG_GPIO
            config_gpio();
        #endif

        for (i = 0; i < 16; ++i)
            uart0_sendStr("\r\n");
        break;
    }

}
void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    uint8 phone_ip[4] = {0};
    struct station_config *sta_conf = pdata;

    switch(status) {
        case SC_STATUS_WAIT:
#if DEBUG
            os_printf("SC_STATUS_WAIT\n");
#endif
            break;
        case SC_STATUS_FIND_CHANNEL:
#if DEBUG
            os_printf("SC_STATUS_FIND_CHANNEL\n");
#endif
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
#if DEBUG
            os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
#endif
            break;
        case SC_STATUS_LINK:
#if DEBUG
            os_printf("SC_STATUS_LINK\n");
#endif

            wifi_station_set_config(sta_conf);
            wifi_station_disconnect();
            wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
#if DEBUG
            os_printf("SC_STATUS_LINK_OVER\n");
#endif
            os_memcpy(phone_ip, (uint8*)pdata, 4);
#if DEBUG
            os_printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
#endif
            smartconfig_stop();
            wifi_station_set_auto_connect(1);
            break;
    }
}

// Check the wifi connectivity.
void timer_check_connection(void *arg)
{
    smartconfig_start(SC_TYPE_ESPTOUCH, smartconfig_done);
}

void user_init(void)
{
    struct station_config stationConf;

    flash_param_init();
    flash_param = flash_param_get();
    UartDev.data_bits = GETUART_DATABITS(flash_param->uartconf0);
    UartDev.parity = GETUART_PARITYMODE(flash_param->uartconf0);
    UartDev.stop_bits = GETUART_STOPBITS(flash_param->uartconf0);
    uart_init(flash_param->baud, BIT_RATE_115200);
    os_printf("size flash_param_t %d\n", sizeof(flash_param_t));

    wifi_station_set_auto_connect(1);

    //Set station mode
    wifi_set_opmode(STATION_MODE);

    // Check Flash saved configuration
    wifi_station_get_config_default(&stationConf);

    //Set ap settings
    wifi_station_set_config_current(&stationConf);
    wifi_set_event_handler_cb(wifi_event_cb);
    wifi_station_set_reconnect_policy(1);

    // Configure the timer to check connection timeout.
    // After 10s, if wifi is not connected, start the smartconfig
    os_timer_disarm(&connectTimer);
    os_timer_setfn(&connectTimer, (os_timer_func_t *)timer_check_connection, NULL);
    os_timer_arm(&connectTimer, 10000, 0);

    system_os_task(recvTask, recvTaskPrio, recvTaskQueue, recvTaskQueueLen);
}
