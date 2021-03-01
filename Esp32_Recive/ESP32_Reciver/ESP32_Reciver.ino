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

const i2s_port_t I2S_PORT_SPKR = I2S_NUM_1;
uint8_t buff_count = 0;
bool msg_recive = false;
int msg_len = 0;

// Буфер для принимаемого аудиосообщения
#define SPKR_BUFF_SIZE 60
uint8_t DataResive[SPKR_BUFF_SIZE*4];

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xA4, 0xCF, 0x12, 0x8A, 0x21, 0xB0};

/******************* Callback when data is sent ******************************************/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}

/****************** Callback when data is received ****************************************/
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&DataResive, incomingData, len);
  msg_recive = true;
  msg_len = len;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  esp_err_t err;

  /******************************* SPEAKER CONFIG *************************************************/
  // The I2S config as per the example
  const i2s_config_t i2s_config_spkr = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 16000,                         // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, 
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, 
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 8,               // number of buffers
      .dma_buf_len = 500     //samples per buffer 
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config_spkr = {
      .bck_io_num   = 26,   // BCKL
      .ws_io_num    = 25,   // LRCL
      .data_out_num = 22,   // DIN
      .data_in_num  = -1   
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT_SPKR, &i2s_config_spkr, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver SPKR: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT_SPKR, &pin_config_spkr);
  if (err != ESP_OK) {
    Serial.printf("SPKR Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S SPKR driver installed.");

  /****************************** WI-FI **********************************************/
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
  
  if(msg_recive == true){
    i2s_write_bytes((i2s_port_t)I2S_PORT_SPKR, (char *)&DataResive, msg_len, portMAX_DELAY);
    msg_recive = false;
    //Serial.println(msg_len);
    msg_len = 0;
  }

}
