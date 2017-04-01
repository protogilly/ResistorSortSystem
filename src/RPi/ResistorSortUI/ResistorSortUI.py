import ResistorSorter
from time import sleep
#from pynput.keyboard import Key, Listener

#def onPressSorting(key):
#    if (key == Key.enter or key == Key.space):
#        ResistorSorter.sendNxt()
#    elif (key == Key.esc):
#        ResistorSorter.sendEnd()
#    return(key)
# This Code starts the listener using the onPressSorting handler
#with Listener(on_press=onPressSorting) as listener:
#    listener.join()

# This code stops the listener
#Listener.stop()

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
            print("Please enter a value between {} and {}\n".format(minVal, maxVal))
            sleep(2)
    
    return(ret)
        
def warnConfirm(prompt):
    print("\n")
    response = input(prompt)
    
    if (response.upper() == "Y" or response.upper() == "YES"):
        return True
    else:
        return False

# Menu lists...
mainMenu = ["1. Complete Sort",
            "2. Ranged Sort",
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

ResistorSorter.clearScreen()

print("Waiting on Ready from Mainboard...\n")
ResistorSorter.waitFor("RDY")

print("Sending Ready to Mainboard...\n")
ResistorSorter.sendRdy()

print("Waiting on handshake...\n")
ResistorSorter.waitFor("ACK")

quitSelected = False

while (not quitSelected):
    menuChoice = menuPrompt(mainMenu)
    
    if (menuChoice == 1):
        # TODO: Implement Complete Sort Algorithm here...
        sleep(1)
        
    elif (menuChoice == 2):
        # Ranged Sort should ask the user what range of values they'd like to sort
        # Next, it should let them know how many sort cycles this should take and prompt for the OK before continuing.
        sleep(1)
        
    elif (menuChoice == 3):
        # Custom Sort will ask the user what values should be accepted in each cup. 
        # This is done either as a Typical value with precision, or a discrete range.
        sleep(1)
        
    elif (menuChoice == 4):
        # Single Value Sort will ask the user what resistor they are looking for, and what precision.
        sleep(1)
        
    elif (menuChoice == 5):
        # QC mode will ask the user what the nominal values of the resistors being tested are.
        sleep(1)
        
    elif (menuChoice == 6):
        # Ohmmeter only asks for confirmation before beginning the "sort" but will report resistances back and ask the user to cycle parts manually.
        sleep(1)
        
    elif (menuChoice == 7):
        # This will open another menu with certain debug commands.
        
        retSelected = False
        
        while (not retSelected):
            debugChoice = menuPrompt(debugMenu)
            
            if (debugChoice == 1):
                # Cycle Feed
                cycleFeed = ResistorSorter.Command()
                cycleFeed.cmd = "CFD"
                cycleFeed.args = []
                cycleFeed.send()
                ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 2):
                # Move Sort Wheel
                targetPos = fetchInt("Move to which cup [1-9]? ", 1, 9)
                sortWheelCmd = ResistorSorter.Command()
                sortWheelCmd.cmd = "MTC"
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
                # Halt Board. (HCF = Halt and Catch Fire)
                
                if warnConfirm("WARNING: This WILL freeze the mainboard! This will require a power cycle! Are you sure [Y/N]?"):
                    HCF = ResistorSorter.Command()
                    HCF.cmd = "HCF"
                    HCF.args = []
                    HCF.send()
                    ResistorSorter.waitFor("ACK")
                
            elif (debugChoice == 5):
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
                                    
            elif (debugChoice == 6):
                # Flush Serial to Console
                # TODO: Implement Serial Flush
                sleep(1)
                
            elif (debugChoice == 7):
                # Force Send RDY
                ResistorSorter.sendRdy()
                
            elif (debugChoice == 8):
                # Force Send ACK
                ResistorSorter.sendAck()
                
            elif (debugChoice == 9):
                # Sets the retSelected flag to leave the debug menu.
                retSelected = True
            
    elif (menuChoice == 8):
        # Sets the quitSelected flag to leave the loop.
        quitSelected = True
        