#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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

#define GAIT_BUFFER_SIZE 1024

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";
const byte revaddress[6] = "00010";

const int buf_len = 32;
char trans_buf[buf_len];

void setup() {
  Serial.begin(9600);

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);

  while (Serial.available() > 0){
    Serial.read();
  }
  
  radio.begin();
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, revaddress); //rev
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

float f;
bool recieved;
char operation;

void loop() {

  unsigned char x;
  if(Serial.available() >= buf_len){

//    digitalWrite(4, LOW);

    byte rbv = Serial.readBytes(trans_buf, buf_len);
    
    if (rbv == buf_len){ //USB read was successful, send packet to ARD-S

      operation = trans_buf[0];
      
//      digitalWrite(2, HIGH);

      char tb2[32];
      memcpy(tb2, trans_buf, 32);
      
      //Write over 2.4GHz radio
      radio.write(trans_buf, buf_len); 

      //read reply from ARD-S
      radio.startListening();
      recieved = false;
      int count = 0;
      bool entered_list = false;
      while (!recieved){
        if (radio.available()) {
          entered_list = true;
          if(operation == OP_PSERV){
            radio.read(trans_buf, 5); //Not reading full packet, just neccesary component  
            if (memcmp(trans_buf, tb2, 32) == 0){
              digitalWrite(2, HIGH);
            }else{
              digitalWrite(3, HIGH);
            }
          }else if(operation == OP_BATT){
            radio.read(trans_buf, 6); //Not reading full packet, just neccesary component  
          }else if(operation == OP_UPLDF || operation == OP_UPLDA || operation == OP_DNLDF || operation == OP_DNLDA){
            radio.read(trans_buf, 32); //Read full buffer
          }else{
            radio.read(trans_buf, 5); //Not reading full packet, just neccesary component
          }
          radio.stopListening();
          recieved = true;
//          digitalWrite(3, HIGH);
        }
        delay(1);
        if (++count > 1000){
          if (entered_list){
            digitalWrite(4, HIGH);
          }else{
            digitalWrite(3, HIGH);
          }
          trans_buf[0] = OP_FAILED0;
          break;
        }
      }

      if(operation == OP_PSERV){
        //Reply to GPC with reduced packet
        if (Serial.write(trans_buf, 5) != 5){
          digitalWrite(4, HIGH);
        }
      }else if(operation == OP_BATT){
        //Reply to GPC with reduced packet
        if (Serial.write(trans_buf, 6) != 6){
          digitalWrite(4, HIGH);
        }
      }else if(operation == OP_UPLDF || operation == OP_UPLDA || operation == OP_DNLDF || operation == OP_DNLDA){
        if (Serial.write(trans_buf, 32) != 32){
          digitalWrite(4, HIGH);
        }
      }else{
        //Reply to GPC with reduced packet
        if (Serial.write(trans_buf, 5) != 5){
          digitalWrite(4, HIGH);
        }
      }
      
    }else{ //Incomplete packet was recieved over USB from GPC
//      digitalWrite(3, HIGH);
    }
    
    

  }

  delayMicroseconds(100);
}
