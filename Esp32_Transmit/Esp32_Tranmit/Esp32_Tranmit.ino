/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <driver/i2s.h>

const i2s_port_t I2S_PORT_MIC  = I2S_NUM_0;

uint8_t buff_count = 0;
#define SPKR_BUFF_SIZE 60
uint32_t DataTransmit[SPKR_BUFF_SIZE];
bool send_success;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xFC, 0x7E, 0x08};

/******************* Callback when data is sent ******************************************/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  //if(status == ESP_NOW_SEND_SUCCESS){ send_success = true; }
  //else { send_success = false; }
}

/****************** Callback when data is received ****************************************/
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //memcpy(&DataResive, incomingData, len);
  //Serial.print("Bytes received: ");
  //Serial.println(len);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  esp_err_t err;

  /************************************* MIC CONFIG **********************************************/
  // The I2S config as per the example
  const i2s_config_t i2s_config_mic = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = 16000,                         // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 8,                           // number of buffers
      .dma_buf_len = 8                              // 8 samples per buffer 
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config_mic = {
      .bck_io_num = 14,   // BCKL
      .ws_io_num = 15,    // LRCL
      .data_out_num = -1, // not used (only for speakers)
      .data_in_num = 32   // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT_MIC, &i2s_config_mic, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing MIC driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT_MIC, &pin_config_mic);
  if (err != ESP_OK) {
    Serial.printf("MIC: Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S MIC driver installed.");

  /****************************** WI-FI **************************************************/
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

  int32_t sample = 0;
  while( buff_count!=SPKR_BUFF_SIZE-1 ){
    int bytes_read = i2s_pop_sample(I2S_PORT_MIC, (char *)&sample, portMAX_DELAY); // no timeout
    if (bytes_read > 0) {
      DataTransmit[buff_count] = sample;
      buff_count++;
    }
  }
  buff_count = 0;
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DataTransmit, SPKR_BUFF_SIZE*4);
  //delay(2);
}
