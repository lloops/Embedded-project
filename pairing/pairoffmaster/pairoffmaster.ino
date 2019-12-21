#include <SoftwareSerial.h>

#define RX 2
#define TX 3
String address = " " ;//Enter address value here
void setup() {
  // put your setup code here, to run once:
  pinMode(7,OUTPUT);// OUR GPIO enable control
  pinMode(4,INPUT); // Our state pin
  Serial.begin(9600);
  digitalWrite(7,HIGH); //turn on enable;
  btSerial.begin(38400);
  set_master();
  pair(address);
}

void loop() {
  

}

void pair(String address){ // Master runs this when it gets close
  while(digitalRead(4) == 0){
    btSerial.print("AT+PAIR="+address+",20\r\n");
    delay(20000);
    btSerial.print("AT+LINK="+address+"\r\n");
    delay(5000);
  }
  if(digitalRead(4) ==1){
    digitalWrite(7,LOW);
    delay(2000);
    digitalWrite(7,HIGH);
    delay(2000);
    btSerial.write("AT+RESET\r\n");
    
  }
}
