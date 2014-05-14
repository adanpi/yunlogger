#!/usr/bin/python

# ================================================================
# Marco Vedovati, 2009-03-23
#
# Original by John Kerl, 2007-05-15.
# ================================================================
# This software is released under the terms of the GNU GPL.
# Please see LICENSE.txt in the same directory as this file.
# ================================================================

from __future__ import division # 1/2 = 0.5, not 0.
import struct
import sys
import binascii
from time import sleep
import os

def float2hex(x):
    p = struct.pack ("!f", x)
    i = struct.unpack("!I", p)
    s = "%08X" % (int(i[0]))
    return s



def hex2float(x):
    return struct.unpack("!f",x)
    

def clearscreen(numlines=100):
    """Clear the console.
numlines is an optional argument used only as a fall-back.
"""
    if os.name == "posix":
        # Unix/Linux/MacOS/BSD/etc
        os.system('clear')
    elif os.name in ("nt", "dos", "ce"):
        # DOS/Windows
        os.system('CLS')
    else:
        # Fallback for other operating systems.
        print '\n' * numlines



def main():
    clearscreen()
    print " Mini Float<->Hex converter by MVe. "
    print " ----------------------------------\n"
         
    while 1 :
      choice = raw_input("1 : Float to hex [f]\n" +
                         "2 : Hex to float [h]\n" +
                         "0 : quit [quit]\n" +
                         "\n# ")
      
      if choice == "1" or choice == "f" :
        print ""
        input = raw_input("Enter float: ")
        try:
          f = float(input)
          print "Hexadecimal: %s \n" % float2hex(f)
        except:
          print "Invalid float number format."
          
      elif choice == "2" or choice == "h" :
        try:
            f=""
            bytenum = 0
            for i in range(4):
                input = raw_input("Enter byte(s) "+str(i+1)+": ")
                bytenum += len(input)/2
                f += binascii.unhexlify(input)
                if bytenum >=4:
                  break
            print ""
            if bytenum != 4:
               print "You must enter exactly 4 bytes."
            else:
              print "Hexadecimal: " + binascii.hexlify(f)
              print "Float value: %f \n" % hex2float( f )
      
        except TypeError:
            print "Wrong hex format."
        
      elif choice == "0" or choice =="quit" :
        print "Bye!"
        sleep(.5)
        sys.exit(0)
      else:
        print "Wrong choice..."
      
      raw_input("\n\nPress any key to continue...")
      clearscreen()
          
    









if __name__ == "__main__":
    main()
#####End#########
