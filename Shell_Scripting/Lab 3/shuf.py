#!/usr/bin/env python3

"""
Output lines selected randomly from a file

Copyright 2005, 2007 Paul Eggert.
Copyright 2010 Darrell Benjamin Carbajal.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Please see <http://www.gnu.org/licenses/> for a copy of the license.

$Id: randline.py,v 1.4 2010/04/05 20:04:43 eggert Exp $
"""

import random, sys
from optparse import OptionParser

class randline:
    def __init__(self, filename, numberRange, isRepeatable):
        self.isRepeat = isRepeatable
        if filename=="-":
            self.lines = sys.stdin.readlines()
        elif len(numberRange) != 0:
            self.lines = numberRange
        elif filename=="":
            self.lines = sys.stdin.readlines()
        else:
            f = open(filename, 'r')
            self.lines = f.readlines()
            f.close()

    def chooseline(self):
        randIdx = 0
        if len(self.lines) > 1:
            randIdx = random.randint(0, len(self.lines)-1)
        chosenWord = self.lines[randIdx]
        if self.isRepeat==False:
           self.lines.pop(randIdx)    
        return chosenWord

    def lineSize(self):
        return len(self.lines)

def main():
    version_msg = "%prog 2.0"
    usage_msg = """%prog [OPTION]... FILE
       %prog -i LO-HI [OPTION]
Write a random permutation of the input lines to standard output."""

    parser = OptionParser(version=version_msg,
                          usage=usage_msg)
    parser.add_option("-n", "--head-count",
                      action="store", dest="count", type="int",  default=0,
                      help="output at most COUNT lines")
    parser.add_option("-i", "--input-range",
              action="store", dest="lohi",metavar="LO-HI", default="",
              help="terat each number LO through HI as an input line")
    parser.add_option("-r", "--repeat",
              action="store_true", dest="isRepeatable", default=False,
              help="output line can be repeated")

    options, args = parser.parse_args(sys.argv[1:])

    try:
        rnge=options.lohi.split("-")
        length=len(rnge)
        count=options.count
    except:
        parser.error("invalid NUMLINES: {0}".
                     format(options.count))

    #Count can't be negative
    if count < 0:
        parser.error("negative count: {0}".
                     format(count))
  
    #If -i is inputed, it must contain a range of int
    if len(options.lohi) != 0:
        if not rnge[0].isdigit() or not rnge[1].isdigit():
            parser.error("invalid input range: {0}".
			format(options.lohi))
        if length!=2:
            parser.error("invalid input range: {0}".
                        format(options.lohi))
        if rnge[0] >= rnge[1]:
            parser.error("invalid input range: {0}".
			format(options.lohi))

    input_file=""
    #Make sure only 1 argument is inputted per option
    if len(args) > 1:
        parser.error("Too many operands")
    #If both a number range and file are inputted, segmentation fault
    elif len(options.lohi) != 0 and len(args) != 0:
        parser.error("Segmentation Fault")
    #If only a file is inputted
    elif len(args)!=0:
        input_file=args[0]
    
    try:
        numberRange=[]
	#If a number range was passed in, create list of number range
        if length == 2:
            for i in range(int(rnge[0]),int(rnge[1])+1):     
                numberRange.append(str(i)+'\n')

        generator = randline(input_file, numberRange, options.isRepeatable)
        linesize = generator.lineSize()
    
        if count == 0:
            shouldOutput=True
            while shouldOutput==True:
                for index in range(linesize):
                    sys.stdout.write(str(generator.chooseline()))
                shouldOutput=options.isRepeatable
        else:
            if linesize <= count:
                if options.isRepeatable==True:
                     for index in range(count):
                         sys.stdout.write(str(generator.chooseline()))
                else:
                    for index in range(linesize):
                        sys.stdout.write(str(generator.chooseline()))
            else:
                for index in range(count):
                    sys.stdout.write(str(generator.chooseline()))

    except IOError as err:
        parser.error("I/O error({0}): {1}".
                     format(err.errno, err.args[0]))

if __name__ == "__main__":
    main()
