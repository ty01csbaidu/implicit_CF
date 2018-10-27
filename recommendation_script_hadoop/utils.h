/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file utils.h
 * @author tangye01(com@baidu.com)
 * @date 2015/02/09 17:57:21
 * @brief
 * load user and word relationship data
 **/




#ifndef  __UTILS_H_
#define  __UTILS_H_

#include <vector>
#include <map>
#include <ext/hash_map>
#include <utility>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>

#define GCC_VERSION (__GNUC__ * 10000 \
        + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
#define HashTable std::tr1::unordered_map

#else
#include <ext/hash_map>
#define HashTable __gnu_cxx::hash_map
#endif

using namespace std;


class SparseFeatureArray {
private:
    unsigned num_user;
    unsigned num_word;
    //HashTable<string, unsigned> uid_map;
    //HashTable<unsigned, unsigned> wid_map;
    vector<vector<pair<unsigned, unsigned> > > data;
    //HashTable<unsigned, vector<unsigned> > test_data;
    map<unsigned, vector<unsigned> > test_data;
    HashTable<unsigned, string> uid_map;
    HashTable<unsigned, unsigned> wid_map;
    set<unsigned> target_userid_set;
public:
    friend class ImplicitCFLearning;
    friend class ItemBasedCF;
    SparseFeatureArray() {
        clear();
    }
    SparseFeatureArray(unsigned ex_num_user, unsigned ex_num_word,
            vector<vector<pair<unsigned, unsigned> > >& ex_data, HashTable<unsigned, string>& ex_uid_map,
            HashTable<unsigned, unsigned>& ex_wid_map) {
        num_user = ex_num_user;
        num_word = ex_num_word;
        data = ex_data;
        uid_map = ex_uid_map;
        wid_map = ex_wid_map;
    }
    ~SparseFeatureArray() {
        num_user = 0;
        num_word = 0;
        //HashTable<string, unsigned>().swap(uid_map);
        //HashTable<unsigned, unsigned>().swap(wid_map);
        vector<vector<pair<unsigned, unsigned> > >().swap(data);
    }
    inline void init() {
        clear();
    }
    inline void clear() {
        //uid_map.clear();
        //wid_map.clear();
        data.clear();
        test_data.clear();
        uid_map.clear();
        wid_map.clear();
        num_user = 0;
        num_word = 0;
        target_userid_set.clear();
    }
    void load(const char* f_data, const char* f_uid_map, const char* f_wordid_map,
            const unsigned target_userid);
    void load_test(const char* f_data);
    inline unsigned get_user_num() {
        return num_user;
    }
    inline unsigned get_word_num() {
        return num_word;
    }
    /*inline unsigned get_uid(const string &s){
        HashTable<string, unsigned>::iterator it = uid_map.find(s);
        if(it != uid_map.end())
            return uid_map[s];
        else{
            cout<< "s is not in uid_map" << endl;
            return uid_map.size()+1;
        }
    }
    inline unsigned get_wid(const unsigned w){
        HashTable<unsigned, unsigned>::iterator it = wid_map.find(w);
        if(it != wid_map.end())
            return wid_map[w];
        else{
            cout<< "w is not in wid_map" << endl;
            return wid_map.size()+1;
        }
    }*/
    inline vector<vector<pair<unsigned, unsigned> > > get_data() {
        return data;
    }

    inline HashTable<unsigned, unsigned> compute_popularity() {
        HashTable<unsigned, unsigned> item_popularity;

        for (vector<vector<pair<unsigned, unsigned> > >::iterator iter = data.begin(); iter != data.end();
                ++iter) {
            for (vector<pair<unsigned, unsigned> >::iterator p_iter = (*iter).begin(); p_iter != (*iter).end();
                    ++p_iter) {
                unsigned wid = p_iter->first;
                unsigned count = p_iter->second;
                HashTable<unsigned, unsigned>::iterator m_iter = item_popularity.find(wid);

                if (m_iter == item_popularity.end()) {
                    item_popularity[wid] = 1;
                } else {
                    item_popularity[wid] += 1;// could use show/acp info
                }
            }
        }

        return item_popularity;
    }
    inline vector<unsigned> get_words_pool() {
        vector<unsigned> words_pool;
        HashTable<unsigned, unsigned> item_popularity = compute_popularity();

        // simple repeat elements multi-times, could use interval sampling
        for (HashTable<unsigned, unsigned>::iterator iter = item_popularity.begin();
                iter != item_popularity.end(); ++iter) {
            for (int i = 0; i < (iter->second); ++i) {
                words_pool.push_back(iter->first);
            }
        }

        return words_pool;
    }

};


void SparseFeatureArray::load_test(const char* f_data) {
    cout << "load test" << endl;
    ifstream infile(f_data);
    //unsigned uid, date, planid, unitid, wordid, count;
    unsigned uid, wordid;
    //while(infile >> uid >> date >> planid >> unitid >> wordid >> count){
    map<unsigned, vector<unsigned> >::iterator iter;
    unsigned test_size = 0;

    while (infile >> uid >> wordid) {
        //cout << "read lines" << endl;
        iter = test_data.find(uid);

        if (iter == test_data.end()) {
            vector<unsigned> tmp_word_list;
            tmp_word_list.push_back(wordid);
            test_data[uid] = tmp_word_list;
        } else {
            test_data[uid].push_back(wordid);
        }

        test_size++;
    }

    cout << "test_size: " << test_size << endl;
}


void SparseFeatureArray::load(const char* f_data, const char* f_uid_map, const char* f_wordid_map,
        const unsigned target_userid) {
    this->clear();

    unsigned count_user = 0;
    unsigned count_word = 0;

    //cout << "load uid map" << endl;
    ifstream in_uid_map(f_uid_map);
    unsigned uid, userid, planid, unitid;
    string joinid;

    while (in_uid_map >> uid >> userid >> planid) {
        stringstream ss;
        //ss << userid << "\t" << planid << "\t" << unitid;
        ss << userid << "\t" << planid;
        joinid = ss.str();
        uid_map[uid] = joinid;
        count_user++;

        if (userid == target_userid) {
            target_userid_set.insert(uid);
        }
    }

    //cout << "load word map" << endl;
    ifstream in_word_map(f_wordid_map);
    unsigned wordid;
    unsigned word;

    while (in_word_map >> wordid >> word) {
        wid_map[wordid] = word;
        count_word++;
    }

    num_user = count_user;
    num_word = count_word;
    //cout << "num_user: " << num_user << endl;
    //cout << "num_item: " << num_word << endl;

    //cout << "load train" << endl;
    ifstream indata(f_data);
    //resize data
    data.resize(num_user);

    //unsigned uid, date, planid, unitid, wordid, count;
    //while(infile >> uid >> date >> planid >> unitid >> wordid >> count){
    unsigned train_size = 0;

    while (indata >> uid >> wordid) {
        data[uid].push_back(make_pair(wordid, 0));
        train_size++;
    }

    //cout << "train_size: " << train_size << endl;
}

#endif  //__UTILS_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
