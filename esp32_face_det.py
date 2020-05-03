# import the necessary packages
import numpy as np
import datetime
import cv2
import os, time, json, calendar
import subprocess as sp
from threading import Thread
import requests
import socket
import array 


pics = {}
lock_pics = False
last_seen_face_time = time.time() +5
ffmpeg_recording = False

def capture(threadname):
 HOST = '0.0.0.0'      
 PORT = 8080
 sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)    
 sock.bind((HOST, PORT))
 print ("Server running", HOST, PORT)
 global pics
 global lock_pics
 global last_seen_face_time
  
 while True:
  
  data, addr = sock.recvfrom(1500)
  if data:
   img_id = int.from_bytes(data[2:4],  byteorder='big', signed=False)
   fragments = int.from_bytes(data[4:5],  byteorder='big', signed=False)
   frag = int.from_bytes(data[5:6],  byteorder='big', signed=False)
   #print("Address:", data[0:2], "Img_ID:", img_id , "Fragments:", fragments, "/", frag )
   if data[0:2] == B"01" and data[2:4] != B"88" and data[2:4] != B"44":
    #print ("IMG_ID:", img_id, " Frag:", frag, " Len: ", len(data[6:]) )
    if img_id in pics:
     pics[img_id]["data"].update({ frag: data[6:] })
    else:
     pics.update({  img_id: { "Fragments": fragments, "intime": time.time(), "data": { frag: data[6:] } }   })
   if data[0:2] == B"01" and data[2:4] == B"88": 
    print("Config")  
    response = "config,FRAMESIZE_VGA,12,400,1400"; #Send config: command, resulution, Jpeg quality, time between frames in ms, MTU size in byte
    sent = sock.sendto(response.encode(), addr)
   if data[0:2] == B"01" and data[2:4] == B"44": 
    if (time.time()-last_seen_face_time < 2) :
     print("Face detected")  
     response = "info,Face detected"; #Send current state
     sent = sock.sendto(response.encode(), addr)
    else:
     response = "info, "; #Send nothing
     sent = sock.sendto(response.encode(), addr)
     
  
 sock.close()  
	
def packet_mgmt(threadname):
 global pics
 global lock_pics
 global last_seen_face_time
 
 print_time = time.time()

 ffmpeg_ref_time=30
 ffmpeg_last_time = time.time()
 ffmpeg_res = '640x480'
 #ffmpeg_res = '800x600'
 #ffmpeg_res = '1280x1024'
 #ffmpeg_res = '1600x1200'
 ffmpeg_bitr = '500k'
 path = os.path.abspath(os.getcwd())+"/"
 file = path+str(calendar.timegm(time.gmtime()))+'_video.mp4'
 #print(file)
 command = [ '/usr/local/bin/ffmpeg',        #'-re',
        '-f', 'rawvideo', '-vcodec','rawvideo', '-s', ffmpeg_res,
        '-pix_fmt', 'bgr24',    '-r', '5','-i', '-', '-an',  '-b:v', ffmpeg_bitr, '-vcodec','h264',file, '-y'  ]
 if ffmpeg_recording:
  proc = sp.Popen(command, stdin=sp.PIPE, stdout=sp.PIPE, stderr=sp.PIPE, bufsize=10**8)

 # Load the cascade
 face_cascade = cv2.CascadeClassifier('haar.xml')
 
 while True:

  if (time.time()-print_time > 2) :
   print_time = time.time()
    #write out into a file, send into the mp4 and mark for delete:
   del_pic = {}
   for pic in list(pics):
    #print(pics[pic]["Fragments"], "/", len( pics[pic]["data"].keys() ) )
    
    if pics[pic]["Fragments"] == len( pics[pic]["data"].keys() ):
     #print("Done")
     file_data = bytearray()
     #order packets by frag
     for i in range(1, pics[pic]["Fragments"]):
     #  print(i,": ", pics[pic]["data"][i])
       file_data +=pics[pic]["data"][i]
     #print("IMG_ID", pic)
     #print(pics[pic]["data"].keys())
     inp = np.asarray(bytearray(file_data), dtype=np.uint8)
     frame = cv2.imdecode(inp, cv2.IMREAD_COLOR)
	 # Convert into grayscale
     gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	 # Detect faces
     faces = face_cascade.detectMultiScale(gray, 1.1, 4)
	 # Draw rectangle around the faces
     for (x, y, w, h) in faces:
       cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 255, 0), 1)
       #print("face")
       last_seen_face_time = time.time()
     cv2.imwrite( os.path.abspath(os.getcwd())+"/pics.jpg", frame );
     try:
      if ffmpeg_recording:
       proc.stdin.write(frame)
     except:
      print("Jpeg error")
     #print("File written: ", pic," ", len(file_data)," B"  )
     del pics[pic]
    else:
     print("Missing ", pics[pic]["Fragments"], "/", len( pics[pic]["data"].keys() ))
   
   
   #remove old pics and damaged pics:
   remove = [k for k in pics if time.time()-pics[k]["intime"] > 3]
   for k in remove: del pics[k]
   

  #restart the ffmpeg subprocess
  if time.time()-ffmpeg_last_time > ffmpeg_ref_time and ffmpeg_recording:
     ffmpeg_last_time = time.time()
     print("Restart ffmpeg")
     proc.stdin.close()
     proc.stderr.close()
     proc.wait()
     file = path+str(calendar.timegm(time.gmtime()))+'_video.mp4'
     print(file)
     command = [ '/usr/local/bin/ffmpeg',        #'-re',
        '-f', 'rawvideo', '-vcodec','rawvideo', '-s', ffmpeg_res,
        '-pix_fmt', 'bgr24',    '-r', '5','-i', '-', '-an',  '-b:v', ffmpeg_bitr, '-vcodec','h264',file, '-y'     ]
     proc = sp.Popen(command, stdin=sp.PIPE, stdout=sp.PIPE, stderr=sp.PIPE, bufsize=10**8)


thread1 = Thread( target=capture, args=("Capture", ) )
thread2 = Thread( target=packet_mgmt, args=("Packet_mgmt", ) )


thread1.start()
thread2.start()

  
