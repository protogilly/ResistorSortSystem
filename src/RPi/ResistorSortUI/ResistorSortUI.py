import ResistorSorter
from time import sleep
import re

def menuPrompt(menuList):
    # Generates a menu prompt given a list of options. Automatically handles ValueErrors and returns the option selected.
    
    validResponse = False
    while (not validResponse):
        
        # Start with a default screen and show the menu list.
        ResistorSorter.clearScreen()
        print("\n".join(menuList))
        print("\n\n")
        
        # Prompt for input
        print("Make a selection and press [Enter]\n")
        response = input('>')
        
        # Try to convert to int
        try:
            ret = int(response)
        
        # On failure, let them know they're being cheeky.
        except ValueError:
            print("Please enter a value between 1 and {}\n".format(len(menuList)))
            sleep(2)
            ret = -1
        
        # If it's in range, return it. Otherwise lightly scold the user.
        if (ret > 0 and ret <= len(menuList)):
            validResponse = True
        else:
            # If we've got a -1, we already scolded the user. Don't bother them twice.
            if (ret != -1):
                print("Please enter a value between 1 and {}\n".format(len(menuList)))
                sleep(2)

    return(ret)

def fetchInt(prompt, minVal, maxVal):
    # Attempts to fetch an integer from the user with the prompt given
    
    validResponse = False
    while (not validResponse):
        
        print("\n")
        response = input(prompt)
        
        # try to convert to int
        try:
            ret = int(response)
            
        # On failure, let them know they're being cheeky.
        except ValueError:
            print("Please enter a value between {} and {}\n".format(minVal, maxVal))
            sleep(2)
            ret = -1
        
        # Verify it is in range
        if (ret >= minVal and ret <= maxVal):
            validResponse = True
        else:
            # Don't bother them twice if we already scolded the once.
            if (ret != -1):
                print("Please enter a value between {} and {}\n".format(minVal, maxVal))
                sleep(2)
    
    return(ret)

def fetchResistance(prompt):
    # Attempts to fetch a resistance value from user input. Can accept multiple formats.
    # Format 1: Simple number.          Example: 4700
    # Format 2: Float with metric.      Example: 4.7k
    # Format 3: Metric as decimal.      Example: 4k7
    
    resFormat = 0
    result = 0.0
    response = ""
    
    while (format == 0):
        print("\n")
        response = input(prompt)
        
        # Regular expressions to the rescue!
        m1RegEx = '^\d+(\.\d+)?$'           # Format [Digits](Optionally a period and more digits)
        m2RegEx = '^\d+(\.\d+)?[RkKM]$'     # Format [Digits](Optionally a period and more digits)[R, k, K, or M]
        m3RegEx = '^\d+[RkKM]\d+$'          # Format [Digits][R, k, K, or M][Digits]
        preMetricRegEx = '^\d+[RkKM](?=\d+$)' # Gets the first part including metric symbol for format 3
        metricRegEx = '[RkKM]'                # Used when replacing the metric with a decimal for format 3
        
        if   (bool(re.search(m1RegEx, response))):
            resFormat = 1
        elif (bool(re.search(m2RegEx, response))):
            resFormat = 2
        elif (bool(re.search(m3RegEx, response))):
            resFormat = 3
        else:
            # Did not match any standard format.
            print ("Please enter a valid resistance (Examples: 10000, 4.7k, 6R8...).");
            sleep(3)
            
    # Now that we know the format and we have a response, we can parse it.
    if (resFormat == 1):
        # Mode 1 is the easiest, it's just an integer or a float.
        result = float(response)
    
    elif (resFormat == 2):
        # Mode 2 is a little more difficult, but still not too hard. First we pull the metric out.
        metric = response[-1]
        response = response[:-1]
        result = float(response)
        
        # And then multiply by the appropriate value
        if (metric == 'k' or metric == 'K'):
            result = result * 1000
        
        if (metric == 'M'):
            result = result * 1000000
        
    elif (resFormat == 3):
        # Mode 3 is probably the trickiest. We have to first find the metric and store it
        match = re.search(preMetricRegEx, response)
        metric = match.group(0)
        metric = metric[-1]
        
        # Next we replace the metric with a period and convert it to float.
        result = float(re.sub(metricRegEx, '.', response))
        
        # Then multiply by the appropriate value.
        if (metric == 'k' or metric == 'K'):
            result = result * 1000
        
        if (metric == 'M'):
            result = result * 1000000
        
    return(result)
    
        
def warnConfirm(prompt):
    print("\n")
    response = input(prompt)
    
    if (response.upper() == "Y" or response.upper() == "YES"):
        return True
    else:
        return False

# Menu lists...
mainMenu = ["1. Major Division Split",
            "2. Range Sort",
            "3. Custom Sort",
            "4. Find Single Value",
            "5. Quality Check",
            "6. Ohmmeter",
            "7. Debug Commands",
            "8. Quit"]

debugMenu = ["1.  Cycle Feed",
             "2.  Move Sort Wheel",
             "3.  Cycle Dispense Arm",
             "4.  Test Measurement",
             "5.  Force Mainboard Halt",
             "6.  Reset Mainboard",
             "7.  Flush Serial to Console",
             "8.  Send Ready",
             "9.  Send ACK",
             "10. Back to Main"]

print("Sending Ready to Mainboard...\n")
ResistorSorter.sendRdy()

print("Waiting on handshake...\n")
ResistorSorter.waitFor("ACK")

quitSelected = False
sortSetup = False

ResistorSorter.clearScreen()

while (not quitSelected):
    menuChoice = menuPrompt(mainMenu)
    
    if (menuChoice == 1):
        # Send the MAJ command, wait for an ACK, then send SRT and begin the sort loop.
        sortMode = ResistorSorter.Command()
        sortMode.cmd = "MAJ"
        
        precision = fetchInt("What is the nominal precision for this set of resistors (Enter a whole number)? ", 1, 100)
        sortMode.args = [str(precision)]
        sortMode.send();
        
        ResistorSorter.waitFor("ACK")
        
        if (warnConfirm("Would you like to begin this sort [Y/N]? ")):
            ResistorSorter.sort()
        
    elif (menuChoice == 2):
        # Ranged Sort should ask the user what range of values they'd like to sort
        # Next, it should let them know how many sort cycles this should take and prompt for the OK before continuing.
        print("Range Sort not yet implemented. Please use a Custom Sort to proceed.")
        sleep(4)
        
    elif (menuChoice == 3):
        # Custom Sort will ask the user what values should be accepted in each cup. 
        # This is done either as a Typical value with precision, or a discrete range.
        sortSettings = ResistorSorter.Command()
        sortSettings.cmd = "SSR"
        sortSettings.args = ['','','','','','','','','','']
        
        precision = fetchInt("What is the nominal precision for this set of resistors (Enter a whole number)? ", 1, 100)
        sortSettings.args[0] = str(precision)
        
        for x in range (1, 9):
            prompt = "Please enter a nominal resistance for Cup " + str(x) + ". "
            cupNom = str(fetchResistance(prompt))
            sortSettings.args[x] = cupNom
            
        sortSettings.send()
        ResistorSorter.waitFor("ACK")
        
        if (warnConfirm("Would you like to begin this sort [Y/N]? ")):
            ResistorSorter.sort()
        
    elif (menuChoice == 4):
        # Single Value Sort will ask the user what resistor they are looking for, and what precision.
        sortSettings = ResistorSorter.Command()
        sortSettings.cmd = "SGL"
        sortSettings.args = ['','']
        
        precision = fetchInt("What is the nominal precision for this set of resistors (Enter a whole number)? ", 1, 100)
        sortSettings.args[0] = str(precision)
        
        cupNom = str(fetchResistance("Please enter a nominal resistance. "))
        sortSettings.args[1] = cupNom
            
        sortSettings.send()
        ResistorSorter.waitFor("ACK")
        
        if (warnConfirm("Would you like to begin this sort [Y/N]? ")):
            ResistorSorter.sort()
        
    elif (menuChoice == 5):
        # QC mode will ask the user what the nominal values of the resistors being tested are.
        sortSettings = ResistorSorter.Command()
        sortSettings.cmd = "QCR"
        sortSettings.args = ['','']
        
        precision = fetchInt("What is the nominal precision of the desired resistor (Enter a whole number)? ", 1, 100)
        sortSettings.args[0] = str(precision)
        
        cupNom = str(fetchResistance("Please enter a nominal resistance. "))
        sortSettings.args[1] = cupNom
        
        sortSettings.send()
        ResistorSorter.waitFor("ACK")
        
        if (warnConfirm("Would you like to begin this sort [Y/N]? ")):
            ResistorSorter.sort()
        
    elif (menuChoice == 6):
        # Ohmmeter only asks for confirmation before beginning the "sort" but will report resistances back and ask the user to cycle parts manually.
        sortSettings = ResistorSorter.Command()
        sortSettings.cmd = "OHM"
        sortSettings.args = []
        
        sortSettings.send()
        ResistorSorter.waitFor("ACK")
        
        if (warnConfirm("Would you like to begin measurements [Y/N]? ")):
            ResistorSorter.sort()
        
    elif (menuChoice == 7):
        # This will open another menu with certain debug commands.
        
        retSelected = False
        
        while (not retSelected):
            debugChoice = menuPrompt(debugMenu)
            
            if (debugChoice == 1):
                # Cycle Feed
                numCycles = fetchInt("How many cycles [1-4]?", 1, 4)
                cycleFeed = ResistorSorter.Command()
                cycleFeed.cmd = "CFD"
                cycleFeed.args = [str(numCycles)]
                cycleFeed.send()
                ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 2):
                # Move Sort Wheel
                targetPos = fetchInt("Move to which cup [1-10]? ", 1, 10)
                sortWheelCmd = ResistorSorter.Command()
                sortWheelCmd.cmd = "MSW"
                sortWheelCmd.args = [str(targetPos)]
                sortWheelCmd.send()
                ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 3):
                # Cycle Dispense Arm
                cycleArm = ResistorSorter.Command()
                cycleArm.cmd = "CDA"
                cycleArm.args = []
                cycleArm.send()
                ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 4):
                # Test Measurement
                testMeasure = ResistorSorter.Command()
                testMeasure.cmd = "TME"
                testMeasure.args = []
                testMeasure.send()
                ResistorSorter.waitFor("ACK")
                mesData = ResistorSorter.Command()
                mesData = ResistorSorter.waitFor("MES")
                
                # Read back the measurement and wait for input.
                print("Cup: " + mesData.args[0] + ", Resistance: " + mesData.args[1] + "\n")
                ResistorSorter.sendAck()
                input()
                
            elif (debugChoice == 5):
                # Halt Board. (HCF = Halt and Catch Fire)
                
                if warnConfirm("WARNING: This WILL freeze the mainboard! This will require a power cycle! Are you sure [Y/N]?"):
                    HCF = ResistorSorter.Command()
                    HCF.cmd = "HCF"
                    HCF.args = []
                    HCF.send()
                    ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 6):
                # Force Mainboard Reset
                
                if warnConfirm("WARNING: This will reset the mainboard. Are you sure [Y/N]?"):
                    reset = ResistorSorter.Command()
                    reset.cmd = "RST"
                    reset.args = []
                    reset.send()
                    ResistorSorter.waitFor("ACK")
                    
                    print("Please Restart this script.")
                    while (True):
                        pass
                                    
            elif (debugChoice == 7):
                # Flush Serial to Console
                # TODO: Implement Serial Flush
                sleep(1)
                
            elif (debugChoice == 8):
                # Force Send RDY
                ResistorSorter.sendRdy()
                
            elif (debugChoice == 9):
                # Force Send ACK
                ResistorSorter.sendAck()
                
            elif (debugChoice == 10):
                # Sets the retSelected flag to leave the debug menu.
                retSelected = True
            
    elif (menuChoice == 8):
        # Sets the quitSelected flag to leave the loop.
        quitSelected = True
        