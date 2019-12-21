#include <SoftwareSerial.h>

#define RX 2
#define TX 3

SoftwareSerial btSerial(TX, RX); //TX RX???


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Goodnight moon!");
  delay(100);
  btSerial.begin(38400);
  delay(100);
  btSerial.write("AT\r\n");
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

void loop() {

}
