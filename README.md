ESP8266 UART over Wifi
==========================

Absolutely transparent bridge for the ESP8266

This is really basic firmware for the ESP that creates a totally transparent TCP socket to ESP UART0 bridge. Characters come in on one interface and go out the other. The totally transparent bridge mode is something that has been missing and is available on higher priced boards.

**Pros: **

* It works. Do you already have a serial project to which you just want to add WiFi? This is your ticket.
* Auto connect to any wifi AP with smart config configured.
* Configure the UART information by telnet into the module and issue commands prefixed by +++AT to escape each command from bridge mode.
```
+++AT                                    # do nothing, print OK
+++AT BAUD                               # print current UART settings
+++AT BAUD <baud> [data [parity [stop]]] # set current UART baud rate and optional data bits = 5/6/7/8 , parity = N/E/O, stop bits = 1/1.5/2
+++AT PORT                               # print current incoming TCP socket port
+++AT PORT <port>                        # set current incoming TCP socket port (restarts ESP)
+++AT FLASH                              # print current flash settings
+++AT FLASH <1|0>                        # 1: The changed UART settings (++AT BAUD ...) are saved ( Default after boot), 0= no save to flash.
+++AT RESET                              # software reset the unit
+++AT GPIO2 <0|1|2 100>                  # 1: pull GPIO2 pin up (HIGH) 0: pull GPIO2 pin down (LOW) 2: reset GPIO2, where 100 is optional to specify reset delay time in ms (default 100ms)
```
Upon success, all commands send back "OK" as their final output.  Note that passwords may not contain spaces.

The settings are saved after the commands
+++AT PORT <port>
+++AT BAUD <baud> ...

After +++AT FLASH 0 the parameter of command +++AT BAUD <baud> ... are  NOT saved to the flash memory.
The new settings are applied to the UART and saved only in RAM.
But a following +++AT PORT <port>  need to flash the settings for the necessary reboot. Then also the changed UART setting are saved to flash.

The disable of flash the settings is for devices with baud rate changes to avoid permanently flash of the setting sector.
Some electric meter start conversion with 300 baud and accept a command to change to 9600.

Example session:
```
user@host:~$ telnet 192.168.1.197
Trying 192.168.1.197...
Connected to 192.168.1.197.
Escape character is '^]'.
+++AT MODE
MODE=3
OK
+++AT AP
SSID=ESP_9E2EA6 PASSWORD= AUTHMODE=0 CHANNEL=3
OK
+++AT AP newSSID password
OK
+++AT AP
SSID=newSSID PASSWORD=password AUTHMODE=2 CHANNEL=3
OK
+++AT AP ESP_9E2EA6
OK
+++AT AP
SSID=ESP_9E2EA6 PASSWORD= AUTHMODE=0 CHANNEL=3
OK
^]c

telnet> c
Connection closed.
```
**Cons:**

* limited buffered TCP writes. The first buffer is the UART FIFO. The second buffer is to collect new uart chars until the previous packet is sent.
From SDK 0.9.4 the next espconn_sent must after espconn_sent_callback of the pre-packet.
All incoming UART characters in the FIFO gets sent immediately via the tx-buffer. The resulting TCP packet has only some bytes.

This could potentially impact performance, however, in my hands that hasn't been an issue.


Build command:
```
./build.sh
```

Flash command, (after switch into flash mode):
```
./flash.sh /dev/ttyUSB0
```
