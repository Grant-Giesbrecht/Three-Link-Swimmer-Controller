//
//  Ardcom.cpp
//
//
//  Created by Grant Giesbrecht on 6/29/17.
//
//

#include "Ardcom.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>

using namespace std;

Ardcom::Ardcom(){
	is_open_bool = false;
}

Ardcom::Ardcom(std::string port){
    open(port);
}

bool Ardcom::open(std::string port){
	Ardcom::file = fopen(port.c_str(),"r+");  //Opening device file
	if (file == NULL){
		Ardcom::is_open_bool = false;
	}else{
		Ardcom::is_open_bool = true;
	}

	return Ardcom::is_open_bool;
}

Ardcom::~Ardcom(){
	if (Ardcom::is_open_bool){
		Ardcom::close();
	}
}

void Ardcom::close(){
	fclose(Ardcom::file);
	Ardcom::is_open_bool = false;
}

void Ardcom::wait_for_MCU(double seconds){

    seconds = ((int)(seconds * 10))/10;

    long delay = 50000*seconds;

    cout << "Waiting for MCU |                     |\rWaiting for MCU  |" << std::flush;
    for (int i = 0 ; i < 20 ; i++){
        usleep(delay); //Sleep 100 ms
        cout << "=" << std::flush;
    }
    cout << endl << "MCU Ready" << endl;
}

bool Ardcom::is_open(){
    return Ardcom::is_open_bool;
}

bool Ardcom::verify_connection(){

    send_char(FN_VERIFY);

    char x;
    recieve_char(x);

    if (x != VAL_ACK) return false;

    return true;
}

void Ardcom::read_MCU_parameter(char parameter, float& value){

    send_char(FN_INQUIRY);
    send_char(parameter);
    recieve_float(value);

}

void Ardcom::set_MCU_parameter(char parameter, float value){

    send_char(FN_SET);
    send_char(parameter);
    send_float(value);

}

void Ardcom::ping_float(char parameter, float send, float& recieve){

    send_char(FN_PING);
    send_char(parameter);
    send_float(send);
    recieve_float(recieve);

}

void Ardcom::pinMode(int pin, int mode){

    send_char(FN_PINMODE);
    send_float((float)pin);
    send_char((char)mode);

}
void Ardcom::digitalWrite(int pin, int value){

    send_char(FN_DIGW);
    send_float(pin);
    send_char((char)value);

}

int Ardcom::digitalRead(int pin){

    send_char(FN_DIGR);
    send_float((float)pin);
    char recieve;
    recieve_char(recieve);
    return (int)recieve;

}

int Ardcom::analogRead(int pin){

    send_char(FN_ANLGR);
    send_float((float)pin);
    float recieve;
    recieve_float(recieve);

    return (int)recieve;

}

/*
 Sends a float to a USB MCU

 PARAMETERS:
 x - float to send
 file - Connected MCU FILE

 Returns true if successful write of 4 bytes
 */
bool Ardcom::send_float(float x){

    if (sizeof(float) != 4) return false;

    char send_bytes[4];

    memcpy(send_bytes, &x, 4);

    fprintf(file, "%c", send_bytes[0]);
    fprintf(file, "%c", send_bytes[1]);
    fprintf(file, "%c", send_bytes[2]);
    fprintf(file, "%c", send_bytes[3]);
    fflush(file);
    //    if (fprintf(file, "%c%c%c%c", send_bytes[0], send_bytes[1], send_bytes[2], send_bytes[3]) != 4){ //Send four bytes to MCU
    //        return false;
    //    }

    return true;
}

/*
 Reads (blocking) a float from a USB MCU

 PARAMETERS:
 x - float reference at which to write recieved float
 file - Connected MCU FILE

 Returns true if read 4 bytes successfully
 */
bool Ardcom::recieve_float(float& x){

    if (sizeof(float) != 4) return false;

    char bytes_in[4];

    for (int i = 0 ; i < 4 ; i++){ //Read four bytes in from MCU
        fscanf(file, "%c", &bytes_in[i]);
    }

    memcpy(&x, bytes_in, 4);

    return true;

}

void Ardcom::send_char(char x){

    fprintf(file, "%c", x);
    fflush(file);

}

void Ardcom::recieve_char(char& x){

    fscanf(file, "%c", &x);

}
