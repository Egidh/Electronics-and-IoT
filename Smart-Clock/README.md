# Connected Clock

*This project is based on ESP-IDF framework*

## Introduction:
**My main goal in this project is, as always, to learn new things!**<br>
You may think some of the choices I made are strange, that I over-complicate things : this is (most of the time) intentional! For example it is a choice to create my own DNS server for the captive portal, it allowed me to understand how DNS and DHCP work, forced me to use nslookup for debug, etc..

## Use case:
My idea for this project is to have a fully functional clock, connected to the internet, capable of waking you up on time, telling you about the upcoming weather and the tasks you may have during the day (through a connection with your personal agenda).
The use case may differ as this project goes on.

## Actual state of the project:
As for now, I implemented the :
- Access Point/Station (AP/STA) mode
- Read/Write on nvs storage
- Captive portal
- Display on LCD (basic UI for now)

## What I learnt so far:
- **Wifi management** on esp32
- **HTTP server** management on esp32
- **FreeRTOS tasks** for the DNS server
- **nslookup** for debugging
- Overall knowledge on **DHCP**, **DNS**, and **TCP/IP**
- **LVGL** library for the LCD display

*Note for myself: need to change the way I handle the wifi init in sta*
