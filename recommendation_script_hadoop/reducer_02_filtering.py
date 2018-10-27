#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This module filtering buy words

Author: tangye(tangye01@baidu.com)
"""


import sys
import os
sys.path.append("./lib")

"""
input:
    tradeid, uid, planid, wordid, (score)
"""


def reducer(k):
    """filter buy words
    
    Returns:
        tradeid, uid, planid, wordid, score
    """

    curr_tradeid = None
    tradeid = None
    curr_uid = None
    uid = None
    curr_planid = None
    planid = None
    curr_wordid = None
    wordid = None
    curr_tag = False
    tag = False
    curr_score = None
    score = None
    count = 0

    for line in sys.stdin:
        fields = line.strip("\n").split("\t")
        tradeid = fields[0]
        uid = fields[1]
        planid = fields[2]
        wordid = fields[3]
        if len(fields) > 4:
            score = fields[4]
            tag = True
        else:
            tag = False

        if curr_tradeid == tradeid and curr_planid == planid and curr_uid == uid \
                and curr_wordid == wordid:
            curr_tag = False

        else:
            if curr_tradeid and curr_uid and curr_planid and curr_wordid and curr_score:
                if curr_tag and count < k:
                    ret = curr_tradeid + "\t" + curr_uid + "\t" + curr_planid + "\t" \
                            + curr_wordid + "\t" + curr_score
                    print>>sys.stdout, ret 

                    if curr_uid == uid and curr_planid == planid:
                        count += 1
                    else:
                        count = 0

            curr_tradeid = tradeid
            curr_planid = planid
            curr_uid = uid
            curr_wordid = wordid
            curr_tag = tag
            if len(fields) > 4:
                curr_score = score

    if curr_tradeid and curr_uid and curr_planid and curr_wordid and curr_score:
        if curr_tag and count < k:
            ret = curr_tradeid + "\t" + curr_uid + "\t" + curr_planid + "\t" \
                    + curr_wordid + "\t" + curr_score
            print>>sys.stdout, ret 


if __name__ == "__main__":
    reducer(10)
