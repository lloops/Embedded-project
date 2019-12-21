#include <SoftwareSerial.h>

#define RX 2
#define TX 3

//switch RX,TX conn. if no response from AT command
SoftwareSerial btSerial(TX, RX); 

uint16_t buf;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Goodnight moon!");
  
  btSerial.begin(38400);
  
  btSerial.write("AT\r\n");
  delay(500);
  btSerial.write("AT\r\n");
  delay(500);
  btSerial.write("AT+INQM=1,10,10\r\n");
  delay(500);
  btSerial.write("AT+ROLE=1\r\n");
  delay(500);
  //set the unique iac between master and target slave to filter out all other devices 
  btSerial.write("AT+IAC=9e8b30\r\n");
  delay(500);

  btSerial.write("AT+INIT\r\n");
  //delay(500);
}

String data = "";
int arg_i = 0;
//set to 20 bcz sometimes there will be more rcvd than the preset # of devices (2nd para)
int four_hex[20][4];
int hex_cnt = 0;
int hex_to_int;
uint16_t by = 0;

int tot = 0;
int num_response = 0;
int res;
int avg_lvl;

void loop() {
    avg_lvl = measure_signal();
    
    Serial.print("Avg signal lvl:");
    Serial.println(avg_lvl);
    Serial.println();   
    delay(1000);
}

int i_device = 0;
long st;
long timeout = 10000;

int measure_signal(){
  
  btSerial.write("AT+INQ\r\n");
  Serial.println("Enters loop: ");
  
  st = millis();
  
  while(1){
    // in case if data corruption causing the missing of OK/r/n, misses the break, inf loop
      if(millis()-st > timeout){
          Serial.println("timed out");
          data = "";
          arg_i= 0; 
          by = 0;
          i_device = 0;
          hex_cnt = 0; 
          
          break;
        }
    
      if (btSerial.available()){
        //Serial.print("A; ");
        buf = btSerial.read();
        Serial.write(buf);
        
        data+=buf; 
      }

      if(char(buf) == ','){
        //Serial.print("B; ");
        arg_i ++; 
      } 
      
      if(arg_i==2){
        //Serial.print("C; ");
        if(buf != 44){
          if(buf <= 57){
              hex_to_int = buf - 48;
            }
          else{
              hex_to_int = buf - 55;
            }
          four_hex[i_device][hex_cnt] =  hex_to_int;
          hex_cnt ++;
        }
      }
    
      //finishes reading all responses  
      if(data.endsWith("79751310")){
        //Serial.print("D; ");
          data = "";
          i_device = 0;
          hex_cnt = 0; 
          break;
        }

      //finishes reading 1 entry
      else if(data.endsWith("1310")){
        //Serial.print("E; ");
          num_response ++;
          i_device ++;
          hex_cnt = 0;    
           
          data = "";
          arg_i= 0; 
          by = 0;
        }   
        
     }

    hex_cnt = 0;    
    data = "";
    arg_i= 0; 
    by = 0;
    i_device = 0;

    Serial.println("Exit loop; ");
    //delay(1000);
    if(num_response == 0){
        Serial.println("Nothing detected");
        return -1;
      }
    else{
        convert_from_ascii(four_hex, num_response);
        res = tot / num_response;
        tot = 0;
        num_response = 0;  
        return res;
    }
    
}



void convert_from_ascii(int arr[20][4], int num_response){ 
  for(int i = 0; i<num_response; i++){
    for (int j = 0; j < 4; j++){
        by |= ( (byte)(arr[i][j]) << (12-j*4) );
      }
    by = (~by) + 1;
    
    tot += (int)by;
    by = 0;
  }
  
  for(int i=0; i<num_response; i++){
      for (int j = 0; j < 4; j++){
        arr[i][j] = 0;
      }
  }
}
