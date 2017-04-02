import subprocess  # Used to run console commands in special circumstances
import serial  # Serial Comms
import sys
import os
import time
from platform import _ver_output

# Open a logfile for debugging...
debugFile = open("./debug", 'w')
    
serialTTY = ""
print("Looking for Serial ACM", end='', flush=True)
loopCount = 0

# Wait until it is found
while (serialTTY == ""):
    if (loopCount > 30):
        loopCount = 0
        print(".", end='', flush=True)
    # Since the serial port for the Teensy may change on reboots and resets, we search it up to be sure we don't have issues.
    proc = subprocess.Popen('ls /dev/tty* | grep ACM', shell=True, stdout=subprocess.PIPE)
    output = proc.stdout.read()    # Run it and store the result
    output = output[:-1]        # Remove the last character (the newline)
    
    # The proc output comes as a byte string, we need a string literal. Decode it.
    serialTTY = output.decode("ascii")
    
    loopCount += 1
    
print ("\n")

print("Waiting for permission from OS...")
time.sleep(2)

# Open Serial Comms
port = serial.Serial()
port.baudrate = 9600
port.port = serialTTY

# Continually try to open the port until it is actually open (in case the Teensy isn't ready/is booting up)
while True:
    port.open()
    if port.isOpen():
        break

class Command:
    """Handles Command I/O and parses input strings into more usable forms."""
    cmd = ""
    args = []

    def send(self):
        # send converts the cmd and arg list into a valid string and sends it over serial.

        # First get the command and append the semicolon
        output = self.cmd
        output = output + ";"
        
        # Next join the list of args by a comma and append it.
        output = output + ",".join(self.args)

        # Get the validation byte and append it to the start
        byteValue = len(output) + 1
        validByte = chr(byteValue)
        output = validByte + output
        
        global debugFile
        debugFile.write("OUT: ")
        debugFile.write(output)
        debugFile.write("\n")

        # Convert to a byte string for serial comms
        serOut = bytes(output, "ascii")

        # Send it out over the global port.
        global port
        port.write(serOut)

    def parse(self, inputStr):
        # parse takes an input string and fills out the cmd and args members accordingly.

        # first we ditch the verification byte (verification byte is used for serial buffer purposes, and is already checked before it gets here)
        workingStr = inputStr[1:]

        # The next three characters will be the command. Take them, then ditch them from the working string along with the semicolon.
        self.cmd = workingStr[:3]
        workingStr = workingStr[4:]

        # If there are characters remaining, those must be arguments.
        if (len(workingStr) > 0):
            # Split them by comma (so much easier than in Arduino...)
            self.args = workingStr.split(',')
        else:
            # Otherwise, the argument list is empty.
            self.args = []
            
def fetchCmd():
    # This fetches the next command in the serial buffer.
    
    global port
    output = ""

    # Wait until a line is available and grab it
    while True:
        if (port.in_waiting > 0):
            nextLine = port.readline()
            
            global debugFile
            debugFile.write("IN: ")
            debugFile.write(nextLine.decode("ascii"))
            debugFile.write("\n")
            
            # Ditch the newline characters
            nextLine = nextLine[:-2]

            # verify the length using the byte
            if (nextLine[0] != len(nextLine)):
                received = nextLine[0]
                expected = len(nextLine)
                print("ERROR: Verification byte invalid. Received {}, Expected {}.\n".format(received, expected))
            else:
                output = nextLine.decode("ascii")
            
            break

    return(output)

def sendRdy():
    # Creates a standard RDY command and sends it.
    readyCommand = Command()

    readyCommand.cmd = "RDY"
    readyCommand.args = []

    readyCommand.send()

def sendAck():
    # Creates a standard ACK command and sends it.
    ackCommand = Command()

    ackCommand.cmd = "ACK"
    ackCommand.args = []

    ackCommand.send()
    
def sendNxt():
    # Creates a standard NXT command and sends it.
    nxtCommand = Command()

    nxtCommand.cmd = "NXT"
    nxtCommand.args = []

    nxtCommand.send()
    
def sendEnd():
    # Creates a standard END command and sends it.
    endCommand = Command()

    endCommand.cmd = "END"
    endCommand.args = []

    endCommand.send()

def sendError(err):
    # Creates an ERR command using err as the arg and sends it.
    errCommand = Command()

    errCommand.cmd = "ERR"
    errCommand.args = [err]

    errCommand.send()

def sendDat(data):
    # Creates a DAT command using data as the arg and sends it.
    datCommand = Command()

    datCommand.cmd = "DAT"
    datCommand.args = [data]

    datCommand.send()
    
def waitFor(command):
    # Waits for the given command and passes it back when received.
    
    cmdRecieved = False
    
    while (not cmdRecieved):
        thisInput = fetchCmd()
        thisCmd = Command()
        thisCmd.parse(thisInput)
    
        if thisCmd.cmd == command:
            cmdRecieved = True
        else:
            print("WARNING: Received unexpected Command. Received {}. Expected {}. Continuing.\n".format(thisCmd.cmd, command))
    
    return(thisCmd)
            
def clearScreen():
    os.system('clear')
    
    # Title Line
    print("Resistor Sortation System\n\n")
            
            
            