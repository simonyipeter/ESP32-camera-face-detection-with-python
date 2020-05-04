#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "esp_camera.h"
#include "SSD1306.h"

#define T_Camera_V17_VERSION
#include "select_pins.h"

#define WIFI_SSID   "xxx"
#define WIFI_PASSWD "xxx"

String ipAddress = "";

const char * udpAddress = "1.2.3.4";  // Python server address
const int udpPort = 8080;                    // Python server port
//The udp library class
WiFiUDP udp;

//timer varibales:
unsigned long lastPIR;    
unsigned long capture;    
unsigned long get_config; 
unsigned long get_info;
unsigned long write2oled;
unsigned long refresh_time = 800;

uint8_t esp32_address[] =  {'0','1'};       //Address of the ESP32, if you have more ESP32 modify it!
uint8_t get_config_command[] =  {'8','8'}; 
uint8_t get_info_command[] =  {'4','4'}; 

uint16_t  packet_size = 1400;               //UDP packet's payload size

String buffer_string_old ="";
String info_string ="";

bool PIR_presents = false;

#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, (OLEDDISPLAY_GEOMETRY)SSD130_MODLE_TYPE);

//Explode a string
String getValue(String data, char separator, int index) {    int found = 0;    int strIndex[] = { 0, -1 };    int maxIndex = data.length() - 1;
    for (int i = 0; i <= maxIndex && found <= index; i++) {    if (data.charAt(i) == separator || i == maxIndex) {            found++;            strIndex[0] = strIndex[1] + 1;            strIndex[1] = (i == maxIndex) ? i+1 : i;        }
    }    return found > index ? data.substring(strIndex[0], strIndex[1]) : ""; }

bool setupCamera(){
    
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
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    config.frame_size = FRAMESIZE_SVGA;  //UXGA SXGA SVGA VGA QVGA Do not use sizes above QVGA when not JPEG
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // camera init
    esp_err_t err = esp_camera_init(&config);   if (err != ESP_OK) {    Serial.printf("Camera init failed with error 0x%x\n", err);        return false;    }

    sensor_t *s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 0);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_SVGA);
    //s->set_quality(s, 10);
    s->set_hmirror(s, 0);

    return true;
}
//END setupCamera

void setupNetwork()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {        delay(300);        Serial.print(".");    }
    Serial.println("");
    Serial.println(F("WiFi connected"));
    Serial.print(F("IP address: "));  Serial.println(WiFi.localIP());    
    ipAddress = WiFi.localIP().toString();
}
//END setupNetwork

void get_config_from_server(){
     udp.beginPacket(udpAddress, udpPort);
     udp.write(esp32_address, 2); udp.write( get_config_command, 2); 
     udp.endPacket();
     Serial.println(F("Send config command"));

     uint8_t buffer[50] = "xxx";   memset(buffer, 0, 50);
     udp.parsePacket();
     //receive response from server
     if(udp.read(buffer, 50) > 0){
     String buffer_string = (char *)buffer;
     Serial.println((char *)buffer);
     if ( buffer_string != "" && buffer_string_old != buffer_string && getValue((char *)buffer, ',', 0) =="config"  ){
        buffer_string_old = buffer_string;
        esp_camera_deinit();
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
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_JPEG;
        //init with high specs to pre-allocate larger buffers
        config.frame_size = FRAMESIZE_SVGA;  //UXGA SXGA SVGA VGA QVGA Do not use sizes above QVGA when not JPEG
        //config.jpeg_quality = 12;
        config.fb_count = 2;
 
       String resolution = getValue(buffer_string, ',', 1);
       if (resolution == "FRAMESIZE_VGA") {  config.frame_size = FRAMESIZE_VGA; } 
       if (resolution == "FRAMESIZE_SVGA") {  config.frame_size = FRAMESIZE_SVGA; }
       if (resolution == "FRAMESIZE_SXGA") {  config.frame_size = FRAMESIZE_SXGA; }
       if (resolution == "FRAMESIZE_UXGA") {  config.frame_size = FRAMESIZE_UXGA; }
       
  
       int jpeg_quality= getValue(buffer_string, ',', 2).toInt();       config.jpeg_quality = jpeg_quality;

       // camera init
      esp_err_t err = esp_camera_init(&config);      if (err != ESP_OK) {    Serial.printf("Camera init failed with error 0x%x\n", err);          }

       //Serial.println(getValue((char *)buffer, ',', 3).toInt() ); 
       if(getValue(buffer_string, ',', 3).toInt() != 0){refresh_time = getValue(buffer_string, ',', 3).toInt(); }
       if(getValue(buffer_string, ',', 4).toInt() != 0){packet_size = getValue(buffer_string, ',', 4).toInt(); }
       
     }
     else{Serial.println("No-config values or No changes"); }
     
     }
     
   }
//END get_config

void get_info_from_server(){
     udp.beginPacket(udpAddress, udpPort);
     udp.write(esp32_address, 2); udp.write( get_info_command, 2); 
     udp.endPacket();
     //Serial.println("Send info command");

     uint8_t buffer[50] = "xxx";     memset(buffer, 0, 50);
     //processing incoming packet, must be called before reading the buffer
     udp.parsePacket();
     //receive response from server
     if(udp.read(buffer, 50) > 0 && getValue((char *)buffer, ',', 0) == "info" ){
     //Serial.print("Server to client: ");
      Serial.println((char *)buffer);      info_string = getValue((char *)buffer, ',', 1);
     }
     else{      info_string = "";      }
     
   }
//END get_info

void setup() {
  
  Serial.begin(115200);
  #if defined(I2C_SDA) && defined(I2C_SCL)
    Wire.begin(I2C_SDA, I2C_SCL);
  #endif

  pinMode(AS312_PIN, INPUT);

  bool status;  status = setupCamera();  Serial.print(F("setupCamera status ")); Serial.println(status);

  setupNetwork();

  oled.init();
  // Wire.setClock(100000);  //! Reduce the speed and prevent the speed from being too high, causing the screen
  oled.setFont(ArialMT_Plain_10);  oled.setTextAlignment(TEXT_ALIGN_CENTER);
    
}

void loop() {
 
  if(WiFi.status() == WL_CONNECTED){
    
    if ( PIR_presents == true && (millis() - capture)> refresh_time ) {              capture = millis();                      
      camera_fb_t *fb = NULL;      fb = esp_camera_fb_get();    if (!fb) {   Serial.printf("Camera capture failed");  }      //Serial.println("Send UDP one JPG...");
      uint8_t fragments =  (uint8_t)( (uint32_t)fb->len / packet_size ) + 1; 
      uint8_t img_id[] =  {random(1, 255),random(1, 255)}; 
      
      Serial.printf("Packets: %u, %u Byte\n", fragments, (uint32_t)(fb->len));
       for (uint8_t i = 1; i <= fragments; i++) {        
        udp.beginPacket(udpAddress, udpPort);
        udp.write(esp32_address, 2);    udp.write(img_id, 2);          udp.write(fragments);      udp.write(i);
        if (i==1){ udp.write(fb->buf[0]); }
        for (uint16_t j = (i-1)*packet_size+1; (j <= i*packet_size) && (j <= (uint16_t)fb->len ); j++) {   udp.write(fb->buf[j]);  /*Serial.printf("%u: %u\n", i, j);  */       }
        udp.endPacket();       }
      esp_camera_fb_return(fb);
    }

    if (  (millis() - get_info)>800  && PIR_presents == true ) {              get_info = millis();        get_info_from_server();    }

    if (  (millis() - get_config)>8000  && PIR_presents == false ) {              get_config = millis();        get_config_from_server();    }
   
  }
 
  if (digitalRead(AS312_PIN)) {               lastPIR = millis();            }
  if (millis() - lastPIR < 6000) {   PIR_presents = true;   }
  if (millis() - lastPIR > 6000) {   PIR_presents = false;  if (millis() - lastPIR > 30000) { lastPIR = millis();    } }

  if (  (millis() - write2oled)>300) {              write2oled = millis();      
      oled.clear();
      oled.drawString( oled.getWidth() / 2, 10,  ipAddress ); 
      if(PIR_presents){oled.drawString( oled.getWidth() / 2, 25,  "Searching..." );  }
      oled.drawString( oled.getWidth() / 2, 40,  info_string ); 
      oled.display();
     }

    
}
