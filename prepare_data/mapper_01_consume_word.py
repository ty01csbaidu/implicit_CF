#!/usr/bin/env python
#-*-coding=GBK-*-

"""
This module get consume words

Author: tangye(tangye01@baidu.com)
"""

import sys
import os
sys.path.append("./lib")
import re
import ConfigParser
import traceback
import urllib2
import string


def mapper():
    """consume words mapper
    
    Returns:
        uid, planid, unitid, wordid, searchid
    """

    for line in sys.stdin:
        fields = line.strip("\n").split("\t")
        if len(fields) >= 36:
            searchid = fields[6]
            clk = int(fields[1]) if fields[1].isdigit() else 0
            price = int(fields[2]) if fields[2].isdigit() else 0
            cmatch = fields[9]
            s_time = fields[35]
            uid = fields[17]
            planid = fields[16]
            unitid = fields[15]
            wordid = fields[18]

            if not searchid == "-" and not uid == "-" and not planid == "-" \
                    and not unitid == "-" and not wordid == "-" \
                    and (cmatch == "222" or cmatch == "223") and clk > 0 and price > 0:
                searchid += s_time
                ret_list = [uid, planid, unitid, wordid, searchid]
                ret = "\t".join(ret_list)
                print>>sys.stdout, ret 
        

if __name__ == "__main__":
    mapper()

