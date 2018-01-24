/*
 * Debug/status LEDs:
 *  2 - 
 *  3 - ON : Upload in progress, OFF : no upload in progress
 *  4 - Not Assigned
 * 
 * 
 * 
 */

#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_BATT_LEVEL 0
#define V_MAX 5 //Battery voltage when measured voltage is 5V

//Codes to send in packet to tell ARD-S what command is being executed
#define OP_PSERV 'P'
#define OP_BATT 'B'
#define OP_UPLDF 'U'
#define OP_UPLDA 'u'
#define OP_DNLDF 'D'
#define OP_DNLDA 'd'
#define OP_HALT 'H'
#define OP_RUN 'R'
#define OP_SDELAY 'S'
#define OP_FAILED0 'F' //What is reported if ARD-M recieves bad value from ARD-S

#define GAIT_BUFFER_SIZE 1024 //Must be even number

RF24 radio(7, 8); // CE, CSN

Servo fserv;
Servo aserv;

const byte address[6] = "00001";
const byte revaddress[6] = "00010";

const int buf_len = 32;
char trans_buf[buf_len];

//Gait buffers
unsigned char f_gait_buf[GAIT_BUFFER_SIZE/2];
unsigned char a_gait_buf[GAIT_BUFFER_SIZE/2];

//State variables
bool run_gait = false;
float t_delay = 100; //ms - delay time between frames of gait
int gait_frame = 0; //Which frame in gait buffer to execute next
int reset_frame = -1; //Frame at which to reset the gait

void setup() {

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.openWritingPipe(revaddress); //rev
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

//  dummy_gait();

  fserv.attach(9);
  fserv.write(0);
  aserv.attach(10);
  aserv.write(180);
}




//Other variables
unsigned long t_last = 0;

float f;
int a;
char cstr[32];
String val_str;

bool uploading_f = false;
bool uploading_a = false;

int count;

void loop() {

  //Look for serial input
  if (radio.available()) {
    radio.read(trans_buf, buf_len);
    radio.stopListening();
    if (trans_buf[0] == OP_PSERV){ //Servo manual control

      delay(1); //Wait one millisecond so transmission doesn't occur before other radio is ready

      //Respond to ARD-M
      count = 0;
      while (!radio.write(trans_buf, 5) && (count++ < 20)){
        digitalWrite(2, HIGH);
      }
      radio.startListening();

      //set servo position
      memcpy(cstr, trans_buf+1, 4);
      cstr[4] = '\0';
      f = atof(cstr);
      fserv.write(f);
      aserv.write(f);
    }else if(trans_buf[0] == OP_BATT){ //Report battery voltage

      //Read battery voltage
      a = analogRead(PIN_BATT_LEVEL);
      val_str = String(a/1023.0*V_MAX);

      for (int i = 0 ; (i < val_str.length()) && (i+1 < buf_len) ; i++){
        trans_buf[i+1] = val_str[i];
      }
      
      //Respond to ARD-M
      count = 0;
      while (!radio.write(trans_buf, 6) && (count++ < 20)){
        digitalWrite(2, HIGH);
      }
      radio.startListening();
    }else if(trans_buf[0] == OP_UPLDF || trans_buf[0] == OP_UPLDA){ //Upload gait buffer

      
      
      //Read segment location
      int segment_loc = 10*(int)((unsigned char)(trans_buf[1])); //Get 10s place
      segment_loc += (int)((unsigned char)(trans_buf[2])); //get 1s place

      //Copy data from packet into buffer
      if (trans_buf[0] = OP_UPLDF){
        for (int i = 0 ; i < (buf_len - 3) ; i++){
          if (segment_loc*(buf_len-3)+i < GAIT_BUFFER_SIZE/2){ // Ensure loop does not exceed buffer bounds
            f_gait_buf[segment_loc*(buf_len-3)+i] = trans_buf[i+3]; //Copy contents
          }
        }
      }else{
        for (int i = 0 ; i < (buf_len - 3) ; i++){
          if (segment_loc*(buf_len-3)+i < GAIT_BUFFER_SIZE/2){ //Ensure loop does not exceed buffer bounds
            a_gait_buf[segment_loc*(buf_len-3)+i] = trans_buf[i+3]; //Copy contents
          }
        }
      }

      //Set gait execution to false until both buffers fully rewritten

      (trans_buf[0] == OP_UPLDF)? uploading_f = true : uploading_a = true; //Report that F or A buffer has been modified
      
      if ((segment_loc+1)*(buf_len-3) >= GAIT_BUFFER_SIZE/2){ //Clear modification flag for buffer if upload is finished
        (trans_buf[0] == OP_UPLDF)? uploading_f = false : uploading_a = false;
      }
      
    }else{
      
    }
  }

  //Execute Gait
  if (run_gait && !uploading_f && !uploading_a && (((millis() - t_last) > t_delay)) || (millis() < t_last)){

    t_last = millis();
    fserv.write(f_gait_buf[gait_frame]);
    aserv.write(a_gait_buf[gait_frame++]);

    if (gait_frame >= GAIT_BUFFER_SIZE/2 || gait_frame >= reset_frame){
      gait_frame = 0;
    }
    
  }

  //Illuminate LED 3 if upload is in progress
  if (uploading_f || uploading_a){
    digitalWrite(3, HIGH);
  }else{
    digitalWrite(3, LOW);
  }
  
  
}

void dummy_gait(){

  for (int i = 0 ; i <= 180 ; i++){
    f_gait_buf[i] = (char)(i);
    a_gait_buf[i] = (char)(180-i);
  }
  
  for (int i = 179 ; i >= 1 ; i--){
    f_gait_buf[(180-i)+180] = (char)i;
    a_gait_buf[(180-i)+180] = (char)(180-i);
  }

  reset_frame = 360;
}


































