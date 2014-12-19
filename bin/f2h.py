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
    



def main():
#    print " Mini Float<->Hex converter by Adan. "
#    print " ----------------------------------\n"
#    print ("Script name: %s" % str(sys.argv[0]))
#    print "Hexadecimal: %s \n" % float2hex(float(sys.argv[1]))
    print "%s" % float2hex(float(sys.argv[1]))
          
 
          
    









if __name__ == "__main__":
    main()
#####End#########
