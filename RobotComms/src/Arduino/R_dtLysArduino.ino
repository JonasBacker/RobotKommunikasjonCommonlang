#include <Servo.h>
#include <SPI.h>
#include <Pixy.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Pixy pixy;
Servo servoRight;
Servo servoLeft;
WiFiClient espClient;
PubSubClient client(espClient);
int touchingLeft;
int touchingRight;
int seeingLeft;
int seeingRight;
bool objectDetected;
bool started = false; // Mqtt command on topic
const char* ssid = "MagnusSugeIMarioCart";
const char* password = "atiganger";
const char* mqtt_server = "192.168.1.4";
const char* tilEV3 = "tilEV3";
const char* tilPc = "tilPc";
const char* go = "go";
const char* found = "found";
const char* stopp = "stop";
const char* loss = "loss";
//char* widthdiff;
int x = 0;
int width = 0; // ev3 block width
int widthTemp = 0;
int count = 0;
bool movement = false;


void turnRight(int degrees) {
  servoLeft.writeMicroseconds(1547);
  servoRight.writeMicroseconds(1547);
  delay(degrees * 12);
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1495);
  delay(5);
}

void turnLeft(int degrees) {
  servoLeft.writeMicroseconds(1453);
  servoRight.writeMicroseconds(1453);
  delay(degrees * 12);
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1495);
  delay(5);
}



void turnTo() {

  if (x < 100) {
    turnLeft(-(x - 145) / 3);
  }
  else if (x > 180) {
    turnRight((x - 145) / 3);
  }

}

void getBlockData() {
  int j;
  uint16_t blocks;
  char buf[32];

  blocks = pixy.getBlocks();
  blocks = pixy.getBlocks();
  blocks = pixy.getBlocks();
  blocks = pixy.getBlocks();
  blocks = pixy.getBlocks();



  sprintf(buf, "Detected %d:\n", blocks);
  Serial.print(buf);

  for (j = 0; j < blocks; j++)
  {
    sprintf(buf, "  block %d: ", j);
    Serial.print(buf);
    pixy.blocks[j].print();
  }

  x = pixy.blocks[0].x;
  width = pixy.blocks[0].width;

  Serial.print("x: ");
  Serial.println(x);
  Serial.print("width: ");
  Serial.println(width);


  if (!blocks) {
    x = 0;
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266 Client")) {
      Serial.println("connected");
      // ... and subscribe to topic
      client.subscribe("arduino");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
    if (receivedChar == '1') {
      started = true;
    }
    if (receivedChar == '0') {
      started = false;
    }
  }

}

void setup() {
  servoRight.attach(D2); //Right Servo
  servoLeft.attach(D3); //Left Servo
  pinMode(D1, INPUT); // Left Whisker
  pinMode(D0, INPUT); // Right Whisker
  pinMode(D8, OUTPUT); //ledpin
  client.setServer(mqtt_server, 1883); //Set client server
  client.setCallback(callback); //Set callback method
  servoRight.writeMicroseconds(1495);
  Serial.begin(9600);
  pixy.init();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (started) {

    client.loop();
    if (started) {
      digitalWrite(D8, LOW);
      turnRight(190);
    }
    client.loop();
    delay(3000);
    client.loop();
    
    if (started) {
      
      // snu seg mot ev3
      turnRight(100);
      digitalWrite(D8, HIGH);
      client.publish(tilEV3, stopp);
      turnRight(100);

      // finn EV3
      while ((x == 0 || x < 100 || x > 180) && started) {
        client.loop();
        delay(500);
        getBlockData();
        delay(500);
        turnTo();
        Serial.println(x);
      }

      // send "found" til EV3
      client.publish(tilEV3, found);

      // Sjekk for bevegelse
      while (count < 8 && started && !movement){
        getBlockData();      // Hent width
        widthTemp = width;   // Lage width i en temp variabel
        delay(1000);
        getBlockData();      // Hent ny width
        //sprintf(widthdiff, "%d", (width - widthTemp));
        //client.publish(tilPc, widthdiff);
        Serial.println((width - widthTemp));
        if (width - widthTemp > 3) {
          client.publish(tilEV3, loss);
          started = false;
          movement = true;
        }
        count++;
      }

      

      // Reset variables
      x = 0;
      movement = false;
      count = 0;
    }

    client.loop();

  }
}





