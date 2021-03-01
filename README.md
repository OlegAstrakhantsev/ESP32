# ESP32 walkie-talkie
The goal of the project is to make a simple, inexpensive encrypted intercom. 

First, it was necessary to check the ability to transmit sound between two esp32. Use EspNow (Success).

Mic -- i2s --> Esp32(transmit) ------ wifi -----> Esp32(recive) -- i2s --> Speaker

# Connection:
ESP32 - INMP441(mic)
 IO14 - SCK
 IO15 - WS
 IO32 - SD
 		L/R(NC)

ESP32 - Max98357(speaker)
 IO26 - BCLK
 IO25 - LRC
 IO22 - DIN
 		GAIN(NC)
 		SD(NC)

# TODO
-> Combining code into a transceiver 
-> Add: PTT button 
-> Add: Online subscriber detection
-> Add: Audio stream compression
-> Add: Switching to sleep mode if there is no subscriber
-> Add: Repeater mode    