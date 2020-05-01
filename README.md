# ESP32-camera-face-detection-with-python
For Demonstration Purposes Only!
The code based on the demo code of TTgo T-camera ESP32

# How it is work?
The ESP32 send images back to a python server, which dectects faces on the image and send back information to ESP32.
The code was written to LILYGO® TTgo T-camera ESP32 V17, but easy to modify to another version ESP32 camera board
https://github.com/lewisxhe/esp32-camera-series

Hopefully there is a PIR on this board, so it only sends image, when motian was detected.

Face ---> PIR && T-camera ESP32 --- Image in UDP packets ---> python server

Face ---- PIR && T-camera ESP32 <--- Message in UDP packets  --- python server

This scenario is much slower than face detection on ESP32, but precise and versatile

After the first boot ESP32 get the configuration from the server:
Frame size, jpeg compression, time between the images, packet size
it's possible to modify this configurations without restart or flash the ESP32.

# Requirement on server side:
- Python 3
- OpenCV 3 <
- FFmpeg (Optional to save to .mp4 )

# Requirement on Arduino IDE side:
- ESP32 board support
- SSD1306 library
- esp_camera.h library

# Install
- Install docker enviroment: 
  https://github.com/simonyipeter/OpenCV4.3.0_FFmpeg
- Start the container:

- Start the pyton file in the container:
  
- Upload the .ino and select_pins.h file to ESP32 board



