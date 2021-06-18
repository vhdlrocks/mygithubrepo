

import serial
import argparse, os
import time
import sys

ser = serial.Serial(timeout=0.5, parity=serial.PARITY_NONE, rtscts=False)
ser.baudrate = 115200
ser.port = 'COM7'
ser.open()

def do_debug(doit, msg):
    if doit:
        print(msg)

def pause(msg):
    agn=input(msg)
    if agn == 'x' or agn == 'X':
        return False
    else:
        return True

def reading_raw_resp():
    maxtimeout = 20
    for timeout in range(0, maxtimeout):
        tmp_line = ser.readlines()
        for line in tmp_line:
            if line.rstrip() == 'OK':
                return
            else:
                if line[0:3] == '<<<':
                    line = line.replace('<<<','')
                print(line.rstrip())
    return

def reading_resp(numSec=2):
    time.sleep(numSec)
    tmp_line = ser.readlines()
    for line in tmp_line:
        print(line)
    #return tmp_line
    return

def waiting_response(resp, maxtimeout = 20):
    found = 0
    for timeout in range(0, maxtimeout):
        tmp_line = ser.readlines()
        for line in tmp_line:
            line = line.replace('\r', '').replace('\n', '')
            if line == resp:
                found = 1
                return True
        time.sleep(1)
    if found == 0:
        print('... not found. Exit.')
        sys.exit()

def waiting_response_noexit(resp):
    found = 0
    maxtimeout = 5
    for timeout in range(0, maxtimeout):
        tmp_line = ser.readlines()
        for line in tmp_line:
            line = line.replace('\r', '').replace('\n', '')
            if line == resp:
                found = 1
                return True
        time.sleep(1)
    return False

def get_line_include(word):
    maxtimeout = 20
    for timeout in range(0, maxtimeout):
        tmp_line = ser.readlines()
        for line in tmp_line:
            if line.find(word) != -1:
                return line
        time.sleep(1)
    return ''

def checking_at():
    ser.readlines()
    ser.write('AT\r')
    # waiting_response('OK', 3)



if __name__ == '__main__':
    # ans=input("Enter value: ")
    # print("Value: " + str(ans))

    print("Starting modem interpreter")
    print("checking_at...")
    # checking_at()
    ans=True
    while ans:
        print("               ");
        print("     ****      ");
        print("    **  **     Python HTTP/MQTT/HTTPS example using the Sequans Monarch Go Starter Kit")
        print("  ** ==== **   ");
        print ("""
        1.  Raw command
        2.  ctrl-z term command
        3.  Send esc character
        4.  HTTP POST Test
        5.  HTTP DELETE Test
        6.  HTTP GET Test
        7.  MQTT Subscribe
        8.  MQTT Post
        9.  Toggle TLS
        10. HTTP GET Test (from test site)
        0.  Exit
        """)
        ans=input("CMD#: ")
        if ans=="1":
            valToSend = "AT+CEREG?\r"
            ser.write(valToSend.encode())
            # ans=input("> ")
            # ans += '\r'
            print("sending: {}".format(valToSend))
            # ser.write("ans".encode())
            reading_resp(2)
            # print('=================================')
            # pause("Press <ENTER> to continue. ")
        elif ans=="2":
            ans=input("> ")
            ans += '\r'
            print("sending: {}".format(ans))
            ser.write("ans".encode())
            reading_resp(5)
            ans=input(">>> ")
            ans += '\x1A'
            print("sending: {}".format(ans))
            ser.write("ans".encode())
            reading_resp(5)
            print('=================================')
            pause("Press <ENTER> to continue. ")
        elif ans=="3":
            ser.write(b'\x1B\r')
            reading_resp(5)
            print('=================================')
            pause("Press <ENTER> to continue. ")
        elif ans=="4":
            print("\n HTTP POST Test")
            post_test()
        elif ans=="5":
            print("\n HTTP DELETE Test")
            delete_test()
        elif ans=="6":
            print("\n HTTP GET Test")
            get_test()
        elif ans=="7":
            print("\nSetup for MQTT Subscribe")
            mqtt_recv(args.debug, args.server, args.port)
        elif ans=="8":
            print("\nSetup for MQTT Post")
            mqtt_post(args.debug, args.server, args.port)
        elif ans=="9":
            print("\nToggle TLS setting")

        elif ans=="10":
            print("\n HTTP GET Test (from test site)")
            get_test_website()

        elif ans=="0":
            print("\n Exit.")
            sys.exit()
        else:
            print("\n Invalid entry!")

    print('Exit programm.')
    ser.close()


    # val = 5
    # test_func()
    # ans=True
    # while ans:
        # ans=input("Enter value: ")
        # print("Hello")
    # print('Exit programm.')
    # ser.close()
    # sys.exit()
