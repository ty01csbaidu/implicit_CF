#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This module transform consume words to imf training format

Author: tangye(tangye01@baidu.com)
"""


import sys


def run(original_data, map_out):
    """prepare imf training data

    Args:
        original_data:
            tradeid, userid, planid, unitid, wordid

        map_out:
            tradeid, label, data
            label=1:train_data:
                uid, wordid
            label=2:uid_map:
                uid, userid, planid, unitid
            label=3:wordid_map:
                wordid, word
    """

    train_data_set = set()
    uid_map_dict = {} #(userid, planid, unitid): uid
    wordid_map_dict = {} #word:wordid
    num_uid = 0
    num_word = 0
    map_o = open(map_out, 'w')
    try:
        with open(original_data) as o_f:
            curr_tradeid = None
            trade_id = None
            for line in o_f:
                fields = line.strip("\n").split("\t")
                tradeid = fields[0]
                userid = fields[1]
                planid = fields[2]
                unitid = fields[3]
                word = fields[4]
                joinid = userid + "\t" + planid
                if curr_tradeid == tradeid:
                    if not joinid in uid_map_dict:
                        uid_map_dict[joinid] = num_uid
                        num_uid += 1
                    if not word in wordid_map_dict:
                        wordid_map_dict[word] = num_word
                        num_word += 1
                    uid = uid_map_dict[joinid]
                    wordid = wordid_map_dict[word]
                    train_data_set.add((uid, wordid))
                else:
                    if curr_tradeid:
                        uid_map_keys = uid_map_dict.items()
                        uid_map_keys = sorted(uid_map_keys, key=lambda x:x[1])
                        for item_tuple in uid_map_keys:
                            map_o.write(curr_tradeid + "\t1\t" + str(item_tuple[1]) \
                                    + "\t" + str(item_tuple[0]) + "\n")
                        wordid_map_keys = wordid_map_dict.items()
                        wordid_map_keys = sorted(wordid_map_keys, key=lambda x:x[1])
                        for item_tuple in wordid_map_keys:
                            map_o.write(curr_tradeid + "\t2\t" + str(item_tuple[1]) \
                                    + "\t" + str(item_tuple[0]) + "\n")
                        train_data_list = list(train_data_set)
                        train_data_list = sorted(train_data_list, key=lambda x:x[0])
                        for item_tuple in train_data_list:
                            map_o.write(curr_tradeid + "\t3\t" + str(item_tuple[0]) \
                                    + "\t" + str(item_tuple[1]) + "\n")
                    curr_tradeid = tradeid
                    num_uid = 0
                    num_word = 0
                    uid_map_dict.clear()
                    wordid_map_dict.clear()
                    train_data_set.clear()
                    if not joinid in uid_map_dict:
                        uid_map_dict[joinid] = num_uid
                        num_uid += 1
                    if not word in wordid_map_dict:
                        wordid_map_dict[word] = num_word
                        num_word += 1
                    uid = uid_map_dict[joinid]
                    wordid = wordid_map_dict[word]
                    train_data_set.add((uid, wordid))
            uid_map_keys = uid_map_dict.items()
            uid_map_keys = sorted(uid_map_keys, key=lambda x:x[1])
            for item_tuple in uid_map_keys:
                map_o.write(curr_tradeid + "\t1\t" + str(item_tuple[1]) + "\t" \
                        + str(item_tuple[0])+"\n")
            wordid_map_keys = wordid_map_dict.items()
            wordid_map_keys = sorted(wordid_map_keys, key=lambda x:x[1])
            for item_tuple in wordid_map_keys:
                map_o.write(curr_tradeid + "\t2\t" + str(item_tuple[1]) + "\t" \
                        + str(item_tuple[0])+"\n")
            train_data_list = list(train_data_set)
            train_data_list = sorted(train_data_list, key=lambda x:x[0])
            for item_tuple in train_data_list:
                map_o.write(curr_tradeid + "\t3\t" + str(item_tuple[0]) + "\t" \
                        + str(item_tuple[1])+"\n")
    finally:
        map_o.close()



if __name__ == '__main__':
    original_data = sys.argv[1]
    map_out = sys.argv[2]
    run(original_data, map_out)

            

