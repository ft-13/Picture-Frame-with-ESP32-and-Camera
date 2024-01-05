#include "SPI.h"
#include "FS.h"
#include "SD.h"
#include "DFRobot_AXP313A.h"
#include "esp_camera.h"
#include "it8951.h"
#include "picture_frame.h"

#define GPIO_WAKE_UP_HALL_TOP   HALL_TOP
#define GPIO_WAKE_UP_HALL_LEFT  HALL_LEFT
#define GPIO_WAKE_UP_HALL_RIGHT HALL_RIGHT
#define NEXT                    1
#define PREV                    0
#define OFF                     LOW
#define ON                      HIGH
#define ENABLE_HALL             LOW
#define ENABLE_SD               LOW
#define ENABLE_DISP             HIGH
#define DISABLE_HALL            HIGH
#define DISABLE_SD              HIGH
#define DISABLE_DISP            LOW
// camera frame size XGA and
// grayscale (8bpp)
#define CAMERA_FRAME_SIZE_XGA   786432 // OV2640 XGA 1024x768=786432


DFRobot_AXP313A axp;                                        // For i2c for DCDC converter on Firebeetle2 board to set camera power on/off 
SPIClass spi = SPIClass(HSPI);                              // SPI for epaper display and µSD card
RTC_DATA_ATTR bool is_batt_via_i2c_configured = false;      // preservation of powerbank permanently ON configuration
RTC_DATA_ATTR int32_t g_sd_file_pos = 1;                    // preservation of current µSD card file position

void blink_led_active(void);                                // magnet detected (HALL sensors) -> ESP32 wakeup indication
void blink_led_error(void);                                 // error detected (no SD card, camera/IT8951 init failed etc)
void write_data_batt(uint8_t, uint8_t, uint8_t);            // write i2c data to powerbank (permanent ON command)
uint8_t camera_init(void);
void cam_pic_to_sd_and_disp(void);                          // get camera frame and send to epaper display and save to µSD card
void pic_from_sd_to_disp(uint8_t);                          // left or right HALL sensor triggered -> load previous (HALL left) or next (HALL right) 
                                                            // picture from µSD card and send to epaper display
uint8_t get_wake_up_gpio_num(void);                         // get GPIO number (1 of 3 HALL sensors) from where ESP32 was woken up from deep sleep
uint8_t get_file_buffer(byte*, SDFS, uint32_t, uint8_t);    // get next or previous file from µSD card
uint8_t sd_write_file(SDFS, uint8_t *, size_t);             // save camera frame to µSD card
uint8_t get_sd_file_num(SDFS, uint32_t*);                   // counts number of files on µSD card

void setup(){
  
  Serial.begin(115200);
  //delay(3000);
  //Serial.println("\n\n\n");
  
  //pinMode(EN_PWR_HALL, OUTPUT);
  pinMode(EN_PWR_DISP, OUTPUT);
  pinMode(EN_PWR_SD, OUTPUT);
  pinMode(LED_SHOT, OUTPUT);
  pinMode(SW_DISP_SD, OUTPUT);                              // analog switch to switch between epaper display and µSD card SPI; low: display, high: µSD
  pinMode(HALL_TOP, INPUT);
  pinMode(HALL_LEFT, INPUT);
  pinMode(HALL_RIGHT, INPUT);

  digitalWrite(LED_SHOT, OFF);
  digitalWrite(EN_PWR_SD, ENABLE_SD);                       // µSD power on
  digitalWrite(EN_PWR_DISP, ENABLE_DISP);                   // epaper display power on

  // send permanent ON command to powerbank
  // after power cycle
  if(!is_batt_via_i2c_configured) {
    Wire1.begin(BATT_SDA, BATT_SCL);
    write_data_batt(BATT_IP5310_ADDRESS, \
      BATT_IP5310_REG, BATT_IP5310_VALUE);
    Wire1.end();
    is_batt_via_i2c_configured = true;
  }
    
  spi.begin(SCK, MISO, MOSI, CS);
}


void loop(){

  // figure out what HALL sensor was triggered 
  switch(get_wake_up_gpio_num()){
    case GPIO_WAKE_UP_HALL_TOP:
      blink_led_active();
      cam_pic_to_sd_and_disp();
      break;
    case GPIO_WAKE_UP_HALL_LEFT:
      blink_led_active();
      pic_from_sd_to_disp(NEXT);
      break; 
    case GPIO_WAKE_UP_HALL_RIGHT:
      blink_led_active();
      pic_from_sd_to_disp(PREV);
      break;
    default: break;
  }
  
  esp_sleep_enable_ext1_wakeup(DEEP_SLEEP_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);  // set ESP32 (ext1) deep sleep GPIO bitmask
  esp_deep_sleep_start();                                                     // keep in mind that ESP32 begins its code from the beginning after wake up from deep sleep
}


void cam_pic_to_sd_and_disp(){
  
  camera_fb_t *fb = NULL;

  spi.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
  if(!IT8951_Init()){
    blink_led_error();
    return;      
  }
  if(!camera_init()){
    blink_led_error();
    return;
  }

  digitalWrite(LED_SHOT, ON);

  // get 5 frames for camera settling
  for(uint8_t i=0; i<1; i++){ // but 1 frame seems enough
    fb = esp_camera_fb_get();
    while (!fb);
    esp_camera_fb_return(fb);
  }
  fb = esp_camera_fb_get();
  while (!fb);
    
  //Serial.printf("len %i\n", (*fb).len);
  digitalWrite(LED_SHOT, OFF);
  digitalWrite(SW_DISP_SD, LOW);
  display_buffer(fb->buf, DISP_OFFSET_X, DISP_OFFSET_Y, fb->width, fb->height); // send frame (picture) to epaper display
  spi.endTransaction();                                                         // stop using SPI bus to allow µSD card using the bus 

  digitalWrite(SW_DISP_SD, HIGH);
  delay(100);
  if (!SD.begin(CS, spi, 24000000)){
    blink_led_error();
    SD.end();
    return;
  }

  if(!sd_write_file(SD, fb->buf, fb->len)){
    blink_led_error();
    SD.end();
    return;
  }
  SD.end();

  // DCDC converter (axp) on firebeetle2 board has a I2C interface.
  // DCDC converter and camera are connected to the same I2C bus.
  // Obvisously camera_init reinitializes I2C peripheral and DCDC converter is not accessible after camera_init.
  // Workaround: Wire.end() and axp.begin again.
  Wire.end(); 
  while(axp.begin() != 0){
    blink_led_error();
    return;
  }
  axp.disablePower(); // disable camera power to save current consumption
}


void pic_from_sd_to_disp(uint8_t ts){
  
  uint32_t file_cnt = 0;
    
  //Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
  byte* psdRamBuffer = (byte*)ps_malloc(CAMERA_FRAME_SIZE_XGA); // allocate bytes in PSRAM
  //Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
  //Serial.println(ESP.getFreePsram());
    
  digitalWrite(SW_DISP_SD, HIGH);
  delay(100);
  if (!SD.begin(CS, spi, 24000000)){
    blink_led_error();
    return;
    esp_deep_sleep_start();
  }

  if(!get_sd_file_num(SD, &file_cnt)){
    blink_led_error();
    SD.end();
    return;     
  }
  if(file_cnt != 0){
    if(!get_file_buffer(psdRamBuffer, SD, file_cnt, ts)){
      blink_led_error();
      SD.end();
      return;
    }
    SD.end();

    spi.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
    digitalWrite(SW_DISP_SD, LOW);
    delay(100);
    if(!IT8951_Init()){
      blink_led_error();
      return;
    }
    
    digitalWrite(LED_SHOT, ON);
    digitalWrite(SW_DISP_SD, LOW);
    display_buffer(psdRamBuffer, DISP_OFFSET_X, DISP_OFFSET_Y, 1024, 768);
    digitalWrite(LED_SHOT, OFF);
    spi.endTransaction();
    spi.end();
  } else {
    SD.end();
  }
}


uint8_t sd_write_file(SDFS SD, uint8_t * buf, size_t len){
  
  char str[20];
  uint32_t file_num = 0;
    
  if(!get_sd_file_num(SD, &file_num)){
    blink_led_error();
    return STAT_ERR;      
  }
  file_num++;
  g_sd_file_pos = file_num; // update global file pos
  sprintf(str, "/%d.txt", file_num);
    
  File file = SD.open(str, FILE_WRITE);
  if(!file){
    blink_led_error();
    return STAT_ERR;
  }
    
  file.write(buf, len); // save camera frame on µSD card (raw bytes)
  file.close();

  return STAT_OK;
}


uint8_t get_file_buffer(byte* buff, SDFS SD, uint32_t file_cnt, uint8_t ts){
  
  char str[20];
  
  if(ts == NEXT){                         // next picture on µSD card
    g_sd_file_pos++;
    if(g_sd_file_pos > file_cnt){         // if value is higher than counted number of files on µSD card
      g_sd_file_pos = 1;                  // start from beginning
    }
  }else{                                  // prev picture on µSD card  
    g_sd_file_pos--;
    if(g_sd_file_pos <= 0){               
      g_sd_file_pos = file_cnt;           // start from last file
    }
  }

  sprintf(str, "/%d.txt", g_sd_file_pos); // all pictures are saved in x.txt format, x = increasing number
  File file = SD.open(str, FILE_READ);
  if(!file){
    file.close();
    blink_led_error();
    return STAT_ERR;
  }
  file.read(buff, CAMERA_FRAME_SIZE_XGA);
  file.close();

  return STAT_OK;
}


uint8_t get_sd_file_num(SDFS SD, uint32_t* p_nf){

  int32_t num_files = 0;
  
  File root = SD.open("/");
  if(!root){
    root.close();
    blink_led_error();
    return STAT_ERR;    
  }

  while(true){
    File entry = root.openNextFile();
    if(!entry){
      break;
    }else{
      num_files++;
    }
  }
  root.rewindDirectory();
  root.close();
  num_files--; // minus because "system volume information" is also counted
  if(num_files < 0)
    num_files = 0;

  *p_nf = (uint32_t)num_files;
  
  return STAT_OK;
}


uint8_t get_wake_up_gpio_num(){
  
  uint64_t gpio_reason = esp_sleep_get_ext1_wakeup_status();
  return (log(gpio_reason))/log(2);
 
  //Serial.print("GPIO that triggered the wake up: GPIO ");
  //Serial.println((log(GPIO_reason))/log(2), 0);
}


void write_data_batt(uint8_t slave_address,uint8_t register_address,uint8_t data){
  
  Wire1.beginTransmission(slave_address);
  Wire1.write(register_address);
  Wire1.write(data);
  Wire1.endTransmission();
}


uint8_t camera_init(){
  
  while(axp.begin() != 0){
    blink_led_error();
    return STAT_ERR;
  }
  axp.enableCameraPower(axp.eOV2640);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 12000000;//10000000; //9MHz war ok
  config.frame_size = FRAMESIZE_XGA; // 1024x768
  config.pixel_format = PIXFORMAT_GRAYSCALE;//PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_LATEST;//CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  /*if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_XGA;//FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_XGA;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 1;
    //config.grab_mode = CAMERA_GRAB_LATEST;
#endif
  }*/

/*#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif*/

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    blink_led_error();
    return STAT_ERR;
  }

  sensor_t * s = esp_camera_sensor_get();

  s->set_gain_ctrl(s, 1);                       // auto gain on 
  s->set_exposure_ctrl(s, 1);                   // auto exposure on 
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_vflip(s,1);                            // vetical flip picture
  s->set_bpc(s, 1);                             // make dark pixels more black
  s->set_wpc(s, 1);                             // make bright pixels more white
  //s->set_gainceiling(s, (gainceiling_t)6); // 0 to 6

  /*s->set_brightness(s, 0); // -2 to 2
  s->set_contrast(s, 0); // -2 to 2
  s->set_saturation(s, 0); // -2 to 2
  s->set_whitebal(s, 1); // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1); // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0); // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_bpc(s, 1); // 0 = disable , 1 = enable
  s->set_wpc(s, 1); // 0 = disable , 1 = enable
//s->set_dcw(s, 0); //bad effect  0 = disable , 1 = enable*/

/*s->set_exposure_ctrl(s, 1); // 0 = disable , 1 = enable
  s->set_aec2(s, 0); // 0 = disable , 1 = enable
//s->set_ae_level(s, 2); // -2 to 2
//s->set_aec_value(s, 400); // 0 to 1200
  s->set_gain_ctrl(s, 0); // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0); // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)6); // 0 to 6
  s->set_raw_gma(s, 1); // 0 = disable , 1 = enable (makes much lighter and noisy)
  s->set_lenc(s, 0); // 0 = disable , 1 = enable
  s->set_hmirror(s, 0); // 0 = disable , 1 = enable
  s->set_vflip(s, 0); // 0 = disable , 1 = enable*/
  
  return STAT_OK;
}


void blink_led_active(){
  
  for(uint8_t i=0; i<3; i++){
    digitalWrite(LED_SHOT, ON);
    delay(100);
    digitalWrite(LED_SHOT, OFF);
    delay(100);
  }
}


void blink_led_error(){
  
  for(uint8_t i=0; i<3; i++){
    digitalWrite(LED_SHOT, ON);
    delay(1000);
    digitalWrite(LED_SHOT, OFF);
    delay(1000);
  }
}
