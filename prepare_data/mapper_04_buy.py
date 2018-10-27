#!/usr/bin/env python
#-*-coding=GBK-*-

"""
This module get buy words

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


def mapper(user_trade):
    """buy words mapper
    Args:
        user_trade: uid trade file
    Returns:
        tradeid, uid, planid, wordid 
    """

    uid_trade_dict = {}
    with open(user_trade) as u_t:
        for line in u_t:
            fields = line.strip("\n").split("\t")
            if len(fields) >= 4:
                uid = fields[0]
                trade = fields[2]
                uid_trade_dict[uid] = trade
                
    for line in sys.stdin:
        fields = line.strip("\n").split("\t")
        if len(fields) >= 21: 
            unitid = fields[1]
            planid = fields[2]
            uid = fields[3]
            wordid = fields[4]
            wstat1 = fields[11]
            wstat2 = fields[12]
            wstat3 = fields[13]
            wstat4 = fields[14]
            isdel = fields[20]


            if not uid == "-" and not planid == "-" and not unitid == "-" \
                    and not wordid == "-" and wstat1 == "0" and wstat2 == "0" \
                    and wstat3 == "0" and wstat4 == "0" and isdel == "0" and uid in uid_trade_dict:
                ret_list = [uid_trade_dict[uid], uid, planid, wordid]
                ret = "\t".join(ret_list)
                print>>sys.stdout, ret 
        

if __name__ == "__main__":
    user_trade = sys.argv[1]
    mapper(user_trade)

