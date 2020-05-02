# ESP32-camera-face-detection-with-python
For Demonstration Purposes Only!
The code based on the demo code of TTgo T-camera ESP32

# How it is work?
The ESP32 send images back to a python server, which dectects faces on the image and send back information to ESP32.
The code was written to LILYGO® TTgo T-camera ESP32 V17, but easy to modify to another version ESP32 camera board
https://github.com/lewisxhe/esp32-camera-series

Hopefully there is a PIR on this board, so it only sends image, when motian was detected.

Face ---> T-camera ESP32 with PIR ----- Image in UDP packets -----> python server

Face ---- T-camera ESP32 with PIR <--- Control message in UDP packets  --- python server

This scenario is much slower than face detection on ESP32, but precise and versatile

After the first boot ESP32 get the configuration from the server:
Frame size, jpeg compression, time between the images, packet size
it's possible to modify this configurations without restart or flash the ESP32.

# Requirement on server side:
- Python 3
- OpenCV 3 or higher
- FFmpeg (Optional to save the images into .mp4 video )

# Requirement on Arduino IDE side:
- ESP32 board support
- SSD1306 library
- esp_camera.h library

# Install
- Clone the repo:
  ```  
  git clone https://github.com/simonyipeter/ESP32-camera-face-detection-with-python.git && cd ESP32-camera-face-detection-with-python/
  ```  
- Install OpenCV environment or use docker:  https://github.com/simonyipeter/OpenCV4.3.0_FFmpeg to start the container:
  ``` 
  docker run -itd -v $PWD/esp32_face_det.py:/home/host_dir/esp32_face_det.py -p 8080:8080/udp --name opencv_ffmpeg opencv_ffmpeg
  ```  
- Start the pyton file:
  ```
  python3 esp32_face_det.py
  ```
  or use the started container:
  ```
  docker exec -it opencv_ffmpeg python3 /home/host_dir/esp32_face_det.py
  ```
- Modify the .ino file:

WIFI_SSID - the Wifi Access Point name, where the ESP32 is connecting.

WIFI_PASSWD - the AP password

udpAddress - the server address, where the python file is running.

udpPort - server port, default 8080

- Upload the .ino and select_pins.h file to ESP32 board



