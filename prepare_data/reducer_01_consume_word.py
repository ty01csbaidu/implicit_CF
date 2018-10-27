#!/usr/bin/env python
#-*-coding=GBK-*-

"""
This module get consume words

Author: tangye(tangye01@baidu.com)
"""


import sys
import os
sys.path.append("./lib")

"""
ret_list = [uid, planid, unitid, wordid, searchid]
"""


def reducer():
    """consume words reducer

    Returns:
        uid, planid, unitid, wordid, count 
    """

    curr_uid = None
    uid = None
    curr_planid = None
    planid = None
    curr_unitid = None
    unitid = None
    curr_wordid = None
    wordid = None
    count = 0

    for line in sys.stdin:
        fields = line.strip("\n").split("\t")
        uid = fields[0]
        planid = fields[1]
        unitid = fields[2]
        wordid = fields[3]
        searchid = fields[4]

        if curr_uid == uid and curr_planid == planid and curr_unitid == unitid \
                and curr_wordid == wordid:
            if not curr_searchid == searchid:
                count += 1
        else:
            if curr_uid and curr_planid and curr_unitid and curr_wordid and count > 0:
                ret = curr_uid + "\t" + curr_planid + "\t" + curr_unitid + "\t" \
                        + curr_wordid + "\t" + str(count)
                print>>sys.stdout, ret 

            curr_uid = uid
            curr_planid = planid
            curr_unitid = unitid
            curr_wordid = wordid
            curr_searchid = searchid
            count = 1

    if curr_uid == uid and curr_planid == planid and curr_unitid == unitid \
            and curr_wordid == wordid:
        if curr_uid and curr_planid and curr_unitid and curr_wordid and count > 0:
            ret = curr_uid + "\t" + curr_planid + "\t" + curr_unitid + "\t" \
                    + curr_wordid + "\t" + str(count)
            print>>sys.stdout, ret 


if __name__ == "__main__":
    reducer()
