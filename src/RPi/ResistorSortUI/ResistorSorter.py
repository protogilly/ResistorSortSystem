import serial                           # Serial Comms
import subprocess                       # Used to run console commands in special circumstances

# Since the serial port for the Teensy may change on reboots and resets, we search it up to be sure we don't have issues.
proc = subprocess.Popen('ls /dev/tty* | grep ACM', shell=True, stdout=subprocess.PIPE)
output = proc.stdout.read()    # Run it and store the result
output = output[:-1]        # Remove the last character (the newline)

# The proc output comes as a byte string, we need a string literal. Decode it.
serialTTY = output.decode("ascii")

# Open Serial Comms
port = serial.Serial()
port.baudrate = 9600
port.port = serialTTY

# Continually try to open the port until it is actually open (in case the Teensy isn't ready/is booting up)
while True:
    port.open()
    if port.isOpen():
        break

# Command Class handles command output and parses input strings into a more usable form
class Command:
    cmd = ""
    args = []

    def send():
        # send converts the cmd and arg list into a valid string and sends it over serial.

        # First get the command and append the semicolon
        output = self.cmd
        output = output + ";"
        
        # Next join the list of args by a comma and append it.
        output = output + ",".join(self.args)

        # Get the validation byte and append it to the start
        validByte = chr(len(output))
        output = validByte + output

        # Convert to a byte string for serial comms
        serOut = bytes(output)

        # Send it out over the global port.
        global port
        port.write(serOut)

    def parse(input):
        # parse takes an input string and fills out the cmd and args members accordingly.

        # first we ditch the verification byte (verification byte is used for serial buffer purposes, and is already checked before it gets here)
        workingStr = input[1:]

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
            
# This fetches the next command in the serial buffer.
def fetchCmd():
    global port
    output = ""

    # Wait until a line is available and grab it
    while True:
        if (port.in_waiting > 0):
            nextLine = port.readline()
        
            # Ditch the newline characters
            nextLine = nextLine[:-2]

            # verify the length using the byte
            if (nextLine[0] != len(nextLine)):
                recieved = nextLine[0]
                expected = len(nextLine)
                print("ERROR: Verification byte invalid. Recieved {}, Expected {}.\n".format(recieved, expected))
            else:
                output = nextLine.decode("ascii")
        break

    return(output)

# Creates a standard RDY command and sends it.
def sendRdy():
    readyCommand = Command();

    readyCommand.cmd = "RDY"
    readyCommand.args = []

    readyCommand.send()

# Creates a standard ACK command and sends it.
def sendAck():
    ackCommand = Command()

    ackCommand.cmd = "ACK"
    ackCommand.args = []

    ackCommand.send()

# Creates an ERR command using err as the arg and sends it.
def sendError(err):
    errCommand = Command()

    errCommand.cmd = "ERR"
    errCommand.args = [err]

    errCommand.send()

# Creates a DAT command using data as the arg and sends it.
def sendDat(data):
    datCommand = Command()

    datCommand.cmd = "DAT"
    datCommand.args = [data]

    datCommand.send()
