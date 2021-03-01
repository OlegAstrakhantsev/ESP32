#include <driver/i2s.h>

const i2s_port_t I2S_PORT_SPKR = I2S_NUM_1;
const i2s_port_t I2S_PORT_MIC  = I2S_NUM_0;

uint8_t buff_count = 0;

#define SPKR_BUFF_SIZE 10
uint32_t block[SPKR_BUFF_SIZE];

void setup() {
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
      .dma_buf_count = 5,               // number of buffers
      .dma_buf_len = 200     //samples per buffer 
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config_spkr = {
      .bck_io_num = 26,   // BCKL
      .ws_io_num = 25,    // LRCL
      .data_out_num = 22, 
      .data_in_num = -1   
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


    //i2s_write_bytes((i2s_port_t)I2S_PORT_SPKR, block, 1000, portMAX_DELAY);

}

void loop() {

  int32_t sample = 0;
  
  while( buff_count!=SPKR_BUFF_SIZE-1 ){
    int bytes_read = i2s_pop_sample(I2S_PORT_MIC, (char *)&sample, portMAX_DELAY); // no timeout
    if (bytes_read > 0) {
      block[buff_count] = sample;
      buff_count++;
    }
  }
 
  i2s_write_bytes((i2s_port_t)I2S_PORT_SPKR, (char *)&block, 4*SPKR_BUFF_SIZE, portMAX_DELAY);

  buff_count = 0;
}
