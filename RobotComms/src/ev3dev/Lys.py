#!/usr/bin/env python3

import ev3dev.ev3 as ev3
from time import sleep as _sleep
import paho.mqtt.client as mqtt

previousSonicRead = 0
previousSonicRead2 = 0
sonicDiff = 0
count = 0

def setup():
    global rightMotor
    global leftMotor
    global touchingLeft
    global touchingRight
    global seeingRight
    global seeingLeft
    global client
    global stopped
    global found
    rightMotor = ev3.LargeMotor('outA')
    leftMotor = ev3.LargeMotor('outB')
    rightMotor.reset()
    leftMotor.reset()
    touchingLeft = False
    touchingRight = False
    seeingRight = 0
    seeingLeft = 0
    stopped = False
    found = False
    
    client = mqtt.Client()
    client.connect("192.168.1.4",1883,60)
    client.on_connect = on_connect
    client.on_message = on_message
    
    
def loop():
    global previousSonicRead
    global previousSonicRead2
    global sonicDiff
    global count
    global stopped
    global found
    while True:
        client.loop()
        ReadSensors()
        previousSonicRead2 = previousSonicRead
        previousSonicRead = ((seeingLeft+seeingRight)/2)
        _sleep(0.5)
        if stopped and found:
            count+=1
            client.publish("tilPc", "Stopped!!")
        if stopped and found and (count > 3):
            if (previousSonicRead - previousSonicRead2) < 0:
                sonicDiff = -(previousSonicRead - previousSonicRead2)
            else: sonicDiff = (previousSonicRead - previousSonicRead2)
            client.publish("tilPc", sonicDiff)
            if (sonicDiff > 10):
                moveForward(2)
                stopped = False
                found = False
                count = 0
        if Touching():
            stop()
            client.publish("arduino", "0")

def main():
    setup()
    loop()


#METODER
def on_connect(client, userdata, flags, rc):
  print("Connected with result code "+str(rc))
  client.subscribe("tilEV3")

def on_message(client, userdata, msg):
  global stopped
  global found
  if msg.payload.decode() == "found":
    found = True
  if msg.payload.decode() == "stop":
    stop()
    stopped = True
  if msg.payload.decode() == "loss":
    turnLeft(175)
    moveForward(2)
    if ((2000-(seeingLeft+seeingRight/2)) > 0):
        _sleep((2000-(seeingLeft+seeingRight/2))*0.01)
    stop()
    turnLeft(170)
    client.publish("arduino", "1")
  client.publish("tilPc", "Message Confirm")

def moveForward(time):
    global rightMotor
    global leftMotor
    rightMotor.run_forever(speed_sp=100)
    leftMotor.run_forever(speed_sp=100)
    _sleep(time*0.001)
def moveBackward(time):
    global rightMotor
    global leftMotor
    rightMotor.run_direct(duty_cycle_sp=-75)
    leftMotor.run_direct(duty_cycle_sp=-75)
    _sleep(time*0.001)
def turnLeft(degrees):
    global rightMotor
    global leftMotor
    rightMotor.run_direct(duty_cycle_sp=37)
    leftMotor.run_direct(duty_cycle_sp=-37)
    initial = ev3.GyroSensor().value()
    while ev3.GyroSensor().value() < initial + degrees :
        _sleep(0.001)
    rightMotor.reset()
    leftMotor.reset()
def turnRight(degrees):
    global rightMotor
    global leftMotor
    rightMotor.run_direct(duty_cycle_sp=-37)
    leftMotor.run_direct(duty_cycle_sp=37)
    initial = ev3.GyroSensor().value()
    while ev3.GyroSensor().value() > initial - degrees :
        _sleep(0.001)
    rightMotor.reset()
    leftMotor.reset()
def idle(time):
    leftMotor.reset()
    rightMotor.reset()
    _sleep(time*0.001)
def stop():
    leftMotor.reset()
    rightMotor.reset()
def ReadSensors():
    global touchingLeft
    global touchingRight
    global seeingLeft
    global seeingRight
    touchingLeft = ev3.TouchSensor().value()
    touchingRight = ev3.TouchSensor().value()
    seeingLeft = ev3.UltrasonicSensor().value()
    seeingRight = ev3.UltrasonicSensor().value()
def Touching():
    global touchingLeft
    global touchingRight
    return touchingLeft or touchingRight
def TouchingBoth():
    global touchingLeft
    global touchingRight
    return touchingLeft and touchingRight
def TouchingLeft():
    global touchingLeft
    return touchingLeft
def TouchingRight():
    global touchingRight
    return touchingRight
def Seeing():
    global seeingLeft
    global seeingRight
    return seeingLeft or seeingRight
def SeeingBoth():
    global seeingLeft
    global seeingRight
    return seeingLeft  and  seeingRight
def SeeingLeft():
    global seeingLeft
    return seeingLeft
def SeeingRight():
    global seeingRight
    return seeingRight
def MoveForward(time):
    moveForward(time)
def MoveBackward(time):
    moveBackward(time)
def TurnRight(degrees):
    turnRight(degrees)
def TurnLeft(degrees):
    turnLeft(degrees)
def Idle(time):
    idle(time)
main()