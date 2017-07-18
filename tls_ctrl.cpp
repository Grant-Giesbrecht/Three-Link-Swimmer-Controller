#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <stdint.h>

#include "Ardcom.hpp"
#include "string_manip.hpp"
#include "stdutil.hpp"

#ifdef _WIN32
	#define USB_1 "COM1"
    #define CLEAR_CMD "CLS"
#elif __APPLE__
	//MacBook Pro USB Port Macros
	#define USB_LEFT "/dev/cu.usbmodem1411"
    #define CLEAR_CMD "clear"
#elif __linux__
	#define USB "/dev/ttyACM0"
    #define CLEAR_CMD "clear"
#else
  #error "Unknown compiler"
#endif

//MCU Parameters
#define REP_BATTERY 0
#define ACT_SERVO_Q 0
#define ACT_SERVO_W 1
#define ACT_SERVO_E 2

using namespace std;

string prompt_avail_USB(string indentation);



int main(int argc, char** argv){

    cout << " -----------  THREE LINK SWIMMER CONTROLLER  ----------- " << endl;

    string port = prompt_avail_USB("");

    if (port == "BAD_PORT" || port == "NO_PORTS"){
        return -1;
    }

    Ardcom mcu(port);
    if (mcu.is_open()){
        cout << "Connected to MCU on port " << port << '.' << endl;
    }else{
        cout << "ERROR: Failed to connect to MCU" << endl;
        return -1;
    }

    bool running = true;
    string input, raw_input;
    vector<string> words;
    vector<string> raw_words;
    float x = -1;
    while (running){

        //Retrieve and format input
        cout << "> ";
        getline(cin, raw_input);
        input = to_uppercase(raw_input);

        //Parse string
        words = parse(input, " ");
        raw_words = parse(raw_input, " ");

        if (words.size() < 0){
            continue;
        }

        //Look for keywords and execute command
        if (words[0] == "HELP"){
            print_file("tls_ctrl_help.txt", 1);
        }else if(words[0] == "CONFIG"){

        }else if(words[0] == "BATTERY"){
            mcu.read_MCU_parameter(REP_BATTERY, x);
            unsigned char* c;
            for (int i = 0 ; i < sizeof(float) ; i++){
                c = (unsigned char*)((&x)+i);
                cout << "\t\t" << (int)(*c) << endl;
            }
            unsigned char c1 = *( (unsigned char*)( (&x) ) );
            cout << "Battery Voltage : " << ((int)(c1))*5./255 << endl; //(int)(voltage_char) << " \t" << to_string(voltage_char*5./255) << 'V' << endl;
        }else if(words[0] == "SERVO"){
            cout << "POSITION: ";
            string pos;
            bool suc;
            getline(cin, pos);
            float posv = strtod(pos, &suc);
            if (!suc) continue;
            if (posv < 45){
                mcu.set_MCU_parameter(ACT_SERVO_Q, x);
            }else if (posv > 135){
                mcu.set_MCU_parameter(ACT_SERVO_W, x);
            }else{
                mcu.set_MCU_parameter(ACT_SERVO_E, x);
            }
        }else if(words[0] == "CLEAR"){
            system(CLEAR_CMD);
        }else if(words[0] == "EXIT"){
            running = false;
        }else{
            cout << "\tERORR: Unrecognized keyword '" + raw_words[0] << "'." << endl;
        }
    }

    return 0;
}










/*****************************************************************************************
*************************************** FUNCTIONS ****************************************
*****************************************************************************************/


string prompt_avail_USB(string indentation){

    string input;

    //Find available USB ports
	vector<string> ports;
	Ardcom test0;
	if (test0.open("/dev/ttyUSB0")){
		ports.push_back("/dev/ttyUSB0");
		test0.close();
	}


	Ardcom test1;
	if (test1.open("/dev/ttyUSB1")){
		ports.push_back("/dev/ttyUSB1");
		test1.close();
	}

    Ardcom test4;
	if (test4.open("/dev/ttyUSB2")){
		ports.push_back("/dev/ttyUSB2");
		test4.close();
	}


	Ardcom test2;
	if (test2.open("/dev/ttyACM0")){
		ports.push_back("/dev/ttyACM0");
		test2.close();
	}


	Ardcom test3;
	if (test3.open("/dev/ttyACM1")){
		ports.push_back("/dev/ttyACM1");
		test3.close();
	}

    Ardcom test5;
	if (test5.open("/dev/ttyACM2")){
		ports.push_back("/dev/ttyACM2");
		test5.close();
	}

	//List available ports and read user selection
	if (ports.size() == 0){
		cout << indentation << "No devices found." << endl;
		return "NO_PORT";
	}

	//Propmt user to select port
    cout << indentation << "Select a port to which to attach..." << endl;
	int a;
    bool success;
	do{
		for (int i = 0 ; i < ports.size() ; i++){
			cout << i << "). " << ports[i] << endl;
		}
		cout << indentation << endl << "Port: ";
		getline(cin, input);
        if (input == "x" || input == "X") break;
        a = (int)strtod(input, &success);
        if (!success) continue;
	}while(a >= ports.size());

	//Check if user entered x (to exit)
	if (input == "x"){
		cout << indentation << "Exiting" << endl;
		return "BAD_PORT";
	}

    return ports[a];
}
