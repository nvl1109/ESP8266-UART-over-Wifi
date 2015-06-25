ESP8266 UART over Wifi
==========================

Absolutely transparent bridge for the ESP8266

This is really basic firmware for the ESP that creates a totally transparent TCP socket to ESP UART0 bridge. Characters come in on one interface and go out the other. The totally transparent bridge mode is something that has been missing and is available on higher priced boards.

**Pros: **
* It works. Do you already have a serial project to which you just want to add WiFi? This is your ticket.
* Auto connect to any wifi AP with smart config configured. After 10s, if the ESP device could not connect to AP, the smart config will be started, and then you can use the smart phone with ESP TOUCH installed to talk to your ESP device connects to your wifi as you want.
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
Upon success, all commands send back "OK" as their final output.

By default, every changes of settings will be saved into flash. If you want to make temporary change, you must run the bellow command first:
```
+++AT FLASH 0
```

Example session - assume your ESP8266's IP is 192.168.1.197:
```
user@host:~$ telnet 192.168.1.197
Trying 192.168.1.197...
Connected to 192.168.1.197.
Escape character is '^]'.
+++AT
OK
+++AT BAUD 115200
OK
this text will be sent to uart after ENTER pressed

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
