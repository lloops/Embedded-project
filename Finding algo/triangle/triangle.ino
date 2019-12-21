#include <SoftwareSerial.h>

// ------------------- motors
//pwm fwd: 255(0) to 400(max. speed)
//max speed = 400
#define M1_DIR  7 //direction (Phase)
#define M2_DIR  6 
#define M1_PWM  9 //pwm  (Enable) //R
#define M2_PWM  10 //L
// M2 right , M1 left (dependent on system)

// set Mx_DIR to high or low to switch direction
int max_spd = 400; //400-255 vals (pwm duty cycle)
int spd = max_spd * 51 / 80; // for analog
//------------------------------------------motors



// ------------------ bluetooth
#include <SoftwareSerial.h>
#define RX 2
#define TX 3

SoftwareSerial btSerial(TX, RX); 

//switch RX,TX conn. if no response from AT command
int curr_lvl;

uint16_t buf;
String data = "";
int arg_i = 0;

int prev_lvl = 10000; // so that the first loop always tells to move fwd
int time_circle = 8000;

int meas_points = 12;
int point_cnt = 0;
int min_point = 1;

//set to 20 bcz sometimes there will be more rcvd than the preset # of devices (2nd para)
int four_hex[25][4];
int hex_cnt = 0;
int hex_to_int;
uint16_t by = 0;

int tot = 0;
int num_response = 0;
int res;
int avg_lvl;

// measure
int i_device = 0;
long st;
long timeout = 10000;

//------------------bluetooth




int state = 0;
void setup() {//setting input and outputs
  
  // -------------------------- motors
  digitalWrite(M1_PWM, LOW);
  pinMode(M1_PWM, OUTPUT);
  digitalWrite(M1_PWM, LOW);
  digitalWrite(M2_PWM, LOW);
  pinMode(M2_PWM, OUTPUT);
  digitalWrite(M2_PWM, LOW);
  digitalWrite(M1_DIR, LOW);
  pinMode(M1_DIR, OUTPUT);
  digitalWrite(M1_DIR, LOW);
  digitalWrite(M2_DIR, LOW);
  pinMode(M2_DIR, OUTPUT);
  digitalWrite(M2_DIR, LOW);

  // 16MHz / 1 (prescaler) / 2 (phase-correct) / 400 (top) = 20kHz
  TCCR1A = 0b10100000;
  TCCR1B = 0b00010001;
  ICR1 = 400;
  // -------------------------- motors

 
  //--------------------------- BLUETOOTH
  // ----------------------------Reading the distance 
  Serial.begin(9600);
  Serial.println("Goodnight moon!");
  //6.17 pm
  btSerial.begin(38400);
  delay(500);
  //6.17 pm
  btSerial.write("AT\r\n");
  delay(500);
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
  //--------------------------- bluetooth

  //bt warmup:
  Serial.println("BT warming up...");
  int w_up = 0;
  int warmup_signal;
  while( w_up < 5){
      warmup_signal = measure_signal();
      delay(2000);
      Serial.print("Warmup signal:");
      Serial.println(warmup_signal);
      Serial.println();

      w_up++;
    }

  Serial.println("Ends Warming up...");
  //---search-algo, first record the init. signal value if not -1 (no detect)
  prev_lvl = measure_signal();
  if(prev_lvl == -1){
      prev_lvl = 10000;
    }
}



//finding algo
void loop() {

    if(state == 0){
    
        fwd();    
        
        delay(1000);
    
        avg_of_avg(3);
    
        // if measured lvl is invalid, keep measuring
        while(curr_lvl <= 0 || curr_lvl > 200){
            avg_of_avg(3);
          }
          
        Serial.println("///////////////////////////////////");
        Serial.println(curr_lvl);
        Serial.println(prev_lvl);
        Serial.println("///////////////////////////////////");
    
        //threshold
        if (curr_lvl < 53){
          Serial.println("---------------------------------");
          state = 1;
        }
        
        //compare curr with prev
        else if (curr_lvl <= prev_lvl && curr_lvl > 0){         
          prev_lvl = curr_lvl;
          Serial.println();
          Serial.print("keep going");
          Serial.println();
        }
        
        else if (curr_lvl > prev_lvl){       
          prev_lvl = curr_lvl;
          Serial.println();
          Serial.print("Try another direction");
          Serial.println();
    
          turn120();
        }
              
    }// end of state 0

    if (state == 1){
        Serial.print("Arrived");
        stopCar();
    }
}// end of loop


void avg_of_avg(int num_of_avgs){
  //each measurement should get at least 5 responses
    //boolean num_gteq_5 = false;
    int tot = 0;
    int pm_cnt = 0;
    Serial.println("Measuring signal...");
    
    while(pm_cnt < num_of_avgs){
      
      curr_lvl = measure_signal();
      
      if(num_response >= 5 && curr_lvl < 100){
          tot += curr_lvl;
          pm_cnt ++;
        }
        
      delay(1000);
    }
    
    curr_lvl = tot / num_of_avgs;
    Serial.println("End measure..."); 

    Serial.println();
    Serial.print("At point#");
    Serial.print(point_cnt - 1);
    Serial.print("; Avg signal lvl: ");
    Serial.println(curr_lvl);
    Serial.println();
}

void turn120(){
  Serial.println("Turning 120...");
  
  OCR1B = 255;
  OCR1A = 340;
  
  delay(2200);
  OCR1B = 0;
  OCR1A = 0;
  delay(3000);
}

void stopCar(){
  OCR1B = 0;
  OCR1A = 0;
  Serial.println("StopCar");
  delay(5000);
}

//distance travelled to be within 2 feet 
void fwd(){
    delay(1000);
    Serial.println("Fwd for 3 secs...");
    //move fwd for 3 secs
    OCR1B = 330;// l
    OCR1A = 330;// r
    delay(2000);
    //end moving & stop
    OCR1B = 0;
    OCR1A = 0;
    delay(2000);
}


//Bt measure
int measure_signal(){
  //moved to here
  num_response = 0; 
  //moved to here
  btSerial.write("AT+INQ\r\n");
  Serial.println("Enters loop: ");
  
  st = millis();
  
  while(1){
    
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
      // in case if data corruption causing the missing of OK/r/n, misses the break ~bug_2, inf loop
        
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
        Serial.print("Num responses: ");
        Serial.println(num_response);
        //num_response = 0;  
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
