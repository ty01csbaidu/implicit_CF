#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This module get buy words

Author: tangye(tangye01@baidu.com)
"""


import sys
import os
sys.path.append("./lib")

"""
ret_list = [tradeid, uid, planid, wordid]
output: tradeid, uid, planid, wordid
"""


def reducer():
    """buy words reducer
    Returns:
       tradeid, uid, planid, wordid 
    """

    curr_tradeid = None
    tradeid = None
    curr_uid = None
    uid = None
    curr_planid = None
    planid = None
    curr_wordid = None
    wordid = None

    for line in sys.stdin:
        fields = line.strip("\n").split("\t")
        tradeid = fields[0]
        uid = fields[1]
        planid = fields[2]
        wordid = fields[3]

        if not(curr_tradeid == tradeid and curr_uid == uid and curr_planid == planid \
                and curr_wordid == wordid):
            if curr_tradeid and curr_uid and curr_planid and curr_wordid:
                ret = curr_tradeid + "\t" + curr_uid + "\t" + curr_planid + "\t" + curr_wordid 
                print>>sys.stdout, ret 

            curr_tradeid = tradeid
            curr_uid = uid
            curr_planid = planid
            curr_wordid = wordid

    if curr_tradeid == tradeid and curr_uid == uid and curr_planid == planid \
            and curr_wordid == wordid:
        if curr_tradeid and curr_uid and curr_planid and curr_wordid:
            ret = curr_tradeid + "\t" + curr_uid + "\t" + curr_planid + "\t" + curr_wordid 
            print>>sys.stdout, ret 


if __name__ == "__main__":
    reducer()
