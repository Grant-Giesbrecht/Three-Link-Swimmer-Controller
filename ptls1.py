# This program was created by Grant Giesbrecht on 15.1.2018
#
# Its purpose is to control the arduino system used to automate
# the three-link-swimmer. It is written in python because C++
# was giving me trouble I couldn't troubleshoot. I had no idea what
# was failing and after months of troubleshooting I have
# switched to python.
#

import serial
import re #For string parsing
import math #For ceil()
#=======================================================#
#============= PREFERENCES AND CONSTANTS ===============#
#=======================================================#

buf_len = 32; #Size of packets sent between computers

gait_buffer_size = 1024; #Size of buffer in ARD-S used to store gait

#Command codes used to tell arduinos what command is being executed
op_pserv = 'P';
op_batt = 'B';
op_upldf = 'U'; #Upload gait to forward servo
op_uplda = 'u'; #Upload gait to aft servo
op_dnldf = 'D'; #Download gait of forward servo
op_dnlda = 'd'; #Download gait of aft servo
op_halt = 'H';
op_run = 'R';
op_sdelay = 'S';
op_failed0 = 'F';

f_gait_buf = [0]*(gait_buffer_size/2); #forward servo gait buffer
a_gait_buf = [0]*(gait_buffer_size/2); #Aft servo gait buffer

#=======================================================#
#================== DEFINE FUNCTIONS ===================#
#=======================================================#

buf_len

ser = serial.Serial('/dev/ttyUSB0', 9600);


# Write a position to a servo
#
# pos : the position as an int (0-180)
#
# Void return
def servoPosition(pos):

    #Ensure pos is within bounds
    if (pos < 0):
        pos = 0;
    elif(pos > 180):
        pos = 180;

    #Convert to string of the correct length
    pos_str = str(int(pos));
    while (len(pos_str) < 4):
        pos_str = '0' + pos_str;

    #Add command ID to string
    pos_str = op_pserv + pos_str;

    #Fill string to make it the correct packet length
    while (len(pos_str) < 32):
        pos_str = pos_str + '0';

    #Print what is being sent
    print('\tWriting to arduino: ' + pos_str);

    #Write to Arduino
    ser.write(pos_str);

    #Read position back & print it out
    print('\t' + ser.read(5));




# Read the swimmer's battery voltage
#
# No arguments
#
# Returns voltage in volts
def getBattLevel():

    #Add command ID to string
    cmd_str = op_batt;

    #Fill string to make it the correct packet length
    while (len(cmd_str) < 32):
        cmd_str = cmd_str + '0';

    #Write to arduino
    ser.write(cmd_str);

    #Read value from arduino
    batt_level = ser.read(6);
    print("OC: " + batt_level[0]);
    print("Battery voltage: " + batt_level[1:len(batt_level)] + " V"); #Trim command ID from level


# Reads a KV file containing gait lists for the swimmer
#
# filename - name of file to read
#
# Void return
def readGaitFromFile(filename):

    #Load KV file contents

    #Transfer KV file contents into buffers
    
    for i in range(0, 181):
        f_gait_buf[i] = i;
        a_gait_buf[i] = 180-i;

    for i in range(1, 180):
        f_gait_buf[i+181] = 180-i;
        a_gait_buf[i+181] = i;
    
    

    
    

# Sends 30 values from buffer, beginning with 'start' to swimmer
#
# start - first index to upload (and next 29)
# gait_sel - select forward or aft servo gait to modify
#   0 - forward servo
#   1 - aft servo (default)
#
# Returns true if swimmer verified correct response (else false)
def uploadGaitSegment(start, gait_sel):
    
    #Write command ID to packet
    if (gait_sel == 0):
        pac_str = op_upldf;
    else:
        pac_str = op_uplda;

    #Write index to packet
    if (start > 99):  #Check bounds (only 2 digits allowed)
        start = 99;
    elif (start < 0):
        start = 0;

    pac_str = pac_str + str(int(start)); #add to string

    #Write gait segment to packet
    for i in range(1, buf_len-3):
        if (int(start)*(buf_len-3)+i < gait_buffer_size/2): #Ensure loop stays within bounds of buffer
            if (gait_sel == 0):
                pac_str = pac_str + str(f_gait_buf[int(start)*(buf_len-3)+i])
            else:
                pac_str = pac_str + str(a_gait_buf[int(start)*(buf_len-3)+i])
        else:
            pac_str = pac_str + " ";

    #Write segment to swimmer
    ser.write(pac_str);

    #Receive verification
    rec_val = ser.read(buf_len);
    return (rec_val == pac_str);    


# Upload gait buffer to swimmer
#
# No arguments
#
# returns numer of segments that failed to upload correctly
def uploadGait():

    num_failed = 0;

    #Loop through all neccesary segments
    for i in range(0, int(math.ceil(gait_buffer_size/2/(buf_len-3)))):
        print("Uploading segment " + str(i) + " with " + str(num_failed) + " failures and coutning");
        if (not uploadGaitSegment(i*buf_len, 0)): #forward servo
            num_failed += 1;
        if (not uploadGaitSegment(i*buf_len, 0)): #Aft servo
            num_failed += 1;

    return num_failed;





#=======================================================#
#===================== MAIN LOOP =======================#
#=======================================================#
    
running = 1;
while (running == 1):

    #Get input from user
    line_in = raw_input('Command: ');

    if (len(line_in) < 1):
        continue;

    words_uc = re.sub("[^\w]", " ", line_in.upper()).split();

    if (len(words_uc) <= 0):
        continue;
    
    if (words_uc[0] == "EXIT"): #Command is exit
        break;
    elif( words_uc[0] == "POS"): #Command set servo position
        #Define servo position
        position = 90;

        #Read user defined position if available
        if (len(words_uc) >= 2):
            try:
                position = int(words_uc[1]);
            except:
                print("\tERROR: Invalid position in command (" + words_uc[1]  + "). Sending 90 degrees.");

        #Send position
        servoPosition(position);
    elif (words_uc[0] == "BAT"):
        
        getBattLevel();

    elif (words_uc[0] == "UPLD"):

        readGaitFromFile("This isn't a real filename");

        uploadGait();
    
    else:
        print("\tERROR: Command not recognized.");













