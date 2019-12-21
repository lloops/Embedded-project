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

int min_lvl = 10000;
int time_circle = 8000;

int meas_points = 8;
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
  while( w_up < 8){
      warmup_signal = measure_signal();
      delay(2000);
      Serial.print("Warmup signal:");
      Serial.println(warmup_signal);
      Serial.println();

      w_up++;
    }

  Serial.println("Ends Warming up...");
  Serial.println();
  //---search-algo, first record the init. signal value if not -1 (no detect)
//  min_lvl = measure_signal();
//  if(min_lvl == -1){
//      min_lvl = 10000;
//    }
}




void loop() {
  
if(state == 0){
  
    //moving to next point ................................
    //make a circle check 12 points, time_circle = 8s
    // circle vals
    OCR1B = 330;
    OCR1A = 280;
    delay(2000);
    
    OCR1B = 0;// l
    OCR1A = 0;// r
//stop..................................................

    Serial.print("1st circle; At Point#");
    Serial.println(point_cnt);
    Serial.println();

    point_cnt++;
    
    delay(1000);

    avg_of_avg(4);

    // if invalid signal, measure again
    while(curr_lvl <= 0 || curr_lvl > 200){
        Serial.println();
        Serial.println("Invalid, again!");
        avg_of_avg(3);
        Serial.println();
      }
    
    //update the min_val
    if (curr_lvl < min_lvl && curr_lvl > 0){
      min_lvl = curr_lvl;
      //deleted the min. point, using comparison, not min. point
    }

    //stop if
    if (curr_lvl <= 45 && curr_lvl > 0){
        Serial.println("!!!!!!! Arrived; stop forever !!!!!!!");
        OCR1A = 0;
        OCR1B = 0;

        state = 4;
      }
      
    //finishes 1st circle, enters the second
    else if (point_cnt == meas_points){
      //cleaning up, ready to enter 2nd circle after 3 secs
      OCR1B = 0;// l 
      OCR1A = 0;// r
      state = 3;
      point_cnt = 0;
      Serial.println("1st Circle ENDS");
      Serial.println();
    }  
}// end of state 0



int prev_round_min;
// 2nd circle + moving fwd
if(state == 3){

   //moving to next point ................................
    //make a circle check 12 points, time_circle = 8s
    // circle vals
    OCR1B = 330;
    //if the second circle is smaller than the first, then make the diff bet. wheels 
    //smaller, less abrupt curve
    OCR1A = 280;
    delay(2000);
    
    OCR1B = 0;// l
    OCR1A = 0;// r
  //stop..................................................
  Serial.println();
  Serial.print("2nd circle; At Point#");
  Serial.println(point_cnt);
  Serial.println();

  point_cnt ++;
  
  delay(1000);

  avg_of_avg(4);
  
  //Arrived:
  if (curr_lvl <= 45 && curr_lvl > 0){
    Serial.println("!!!!!!! Arrived; stop forever !!!!!!!");
    OCR1A = 0;
    OCR1B = 0;

    state = 4;
  }

  
  // if the signal levl matched:
  else if(curr_lvl - min_lvl <= 2){
    //min_lvl = measure_signal(); 
    prev_round_min = curr_lvl; 
    //reset min_lvl
    min_lvl = 10000; 
      
    Serial.println("!!!!!!!!! Cmp signal lvl found !!!!!!!!");
    Serial.println(curr_lvl);
    Serial.println();
    
    OCR1B = 0;// l
    OCR1A = 0;// r
    Serial.println("Turning 90...");
    delay(1000);
    turn90();
    Serial.println("Ends turning 90...");
    delay(1000);
    
  //clean up point cnt
      point_cnt = 0;
      fwd();
      Serial.println("Fwd ends; Enters Another roud circle 1 -> 2"); 
      state = 0;     
    }


  //if match signal not found: go back to state 0, first circle again
   else if (point_cnt == meas_points){
    Serial.println("Cmp signal not found"); 
      point_cnt = 0;
      state = 0;
      min_lvl = 10000;
    }  
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
      
      if(num_response >= 5 && (curr_lvl < 100 && curr_lvl > 0)){
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


void turn90(){
  OCR1B = 255;
  OCR1A = 340;
  delay(600);// 90 degrees

  OCR1B = 0;
  OCR1A = 0;
}

void fwd(){
    delay(1000);
    Serial.println("Fwd for 3 secs...");
    //move fwd for 3 secs
    OCR1B = 300;// l
    OCR1A = 300;// r
    delay(3000);
    //end moving & stop
    OCR1B = 0;
    OCR1A = 0;
    delay(3000);

    avg_of_avg(4);

    Serial.print("Signal Now at stop: ");
    Serial.print(curr_lvl);
    Serial.println();

    if (curr_lvl <= 45 && curr_lvl > 0){
        Serial.println("!!!!!!! Arrived; stop forever !!!!!!!");
        OCR1A = 0;
        OCR1B = 0;
        state = 4; //nonexistent
    }
}

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
        Serial.println();
        return -1;
      }
    else{
        convert_from_ascii(four_hex, num_response);
        res = tot / num_response;
        tot = 0;
        Serial.print("Num responses: ");
        Serial.println(num_response);
        Serial.println();
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
