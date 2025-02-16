# LoRa Simple Reciever

**This project is second part of LoRa transmitter/reciever project. Please see LoRaSimpleTransmitter before you start reading this project.**

Code example show how to read data (humidity and temperature) from remote transmitter. Second STM32WLE5JC recieve data, convert recieved data bytes to decimal namber according to formula from Fig.1 and send it to PC via uart/USB converter.

**Most important files:**

1. SubGHz_Phy/App/subghz_phy_app.c - Recieving and converting data.

**Hardwear**

1. Wio E5 mini development board

![Formula](https://github.com/user-attachments/assets/68e000f3-e90c-4491-92d9-3daf80d56f99)
Fig.1 Formula to calculate humidity and temperature. Srh and St are 16 bits value from STH30

![receiver](https://github.com/user-attachments/assets/6c670857-171d-4dc9-adb8-7d5a6f720a0b)
Fig.2 Hardwear

![TeraTerm](https://github.com/user-attachments/assets/44f6b003-d7e2-4124-9e37-263ab257a795)
Fig.3 Reading data from TeraTerm console. 
