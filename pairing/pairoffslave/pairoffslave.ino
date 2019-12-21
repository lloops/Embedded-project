#include <SoftwareSerial.h>

#define RX 2
#define TX 3
void setup() {
  // put your setup code here, to run once:
  pinMode(7,OUTPUT);// OUR GPIO enable control
  pinMode(4,INPUT); // Our state pin
  Serial.begin(9600);
  digitalWrite(7,HIGH); //turn on enable;
  btSerial.begin(38400);
  set_slave();
}

void loop() {
  // put your main code here, to run repeatedly
    if(digitalRead(4) == 1){
      digitalWrite(7,LOW);
      delay(2000);
      digitalWrite(7,HIGH);
      delay(2000);
      set_master();
  }
}

void set_slave(){ btSerial.write("AT\r\n");
  delay(100);
  btSerial.write("AT+RESET=0\r\n");
  delay(100);
  btSerial.write("AT+ROLE=0\r\n");
  delay(100);
  btSerial.write("AT+CLASS=0\r\n");
  delay(100);
  btSerial.write("AT+INIT\r\n");
  delay(100);
  btSerial.write("AT+IAC=9e8b30\r\n");
  delay(100);
  btSerial.write("AT+INQ\r\n");
}

void set_master(){ 
  btSerial.write("AT\r\n");
  delay(500);
  btSerial.write("AT+RESET\r\n");
  delay(500);
  btSerial.write("AT=CLASS=0\r\n");
  delay(500);
  btSerial.write("AT+INQM=1,10,10\r\n");
  delay(500);
  btSerial.write("AT+ROLE=1\r\n");
  delay(500);
  btSerial.write("AT+IAC=9e8b30\r\n");
  delay(500);
  btSerial.write("AT+INIT\r\n");
}
