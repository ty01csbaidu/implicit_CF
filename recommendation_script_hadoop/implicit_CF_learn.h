/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file implicit_CF_learn.h
 * @author tangye01(com@baidu.com)
 * @date 2015/02/10 18:34:52
 * @brief
 *
 **/



#ifndef  __IMPLICIT_CF_LEARN_H_
#define  __IMPLICIT_CF_LEARN_H_


#include <cstdlib>
#include <ctime>
#include <climits>
#include "utils.h"
#include "apex_config.h"
#include "random.h"
#include <algorithm>
#include <set>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

struct TrainParam {
    /* learning rate */
    float learning_rate;
    /* learning rate decay rate */
    float decay_rate;
    /* regularization for user */
    float wd_user;
    /* regularization for item */
    float wd_item;

    TrainParam() {
        learning_rate = 0.01f;
        decay_rate = 1.0f;
        wd_user = wd_item = 0.1f;
    }

    inline void init() {
        learning_rate = 0.01f;
        decay_rate = 1.0f;
        wd_user = wd_item = 0.1f;
    }

    inline void set_param(const char* name, const char* val) {
        if (!strcmp("learning_rate", name)) {
            learning_rate = (float)atof(val);
        }

        if (!strcmp("decay_rate", name)) {
            decay_rate = (float)atof(val);
        }

        if (!strcmp("wd_user", name)) {
            wd_user = (float)atof(val);
        }

        if (!strcmp("wd_item", name)) {
            wd_item = (float)atof(val);
        }
    }

};



class ModelParam {
private:
    /* number of user */
    unsigned num_user;
    /* number of item */
    unsigned num_item;
    /* number of factor */
    unsigned num_factor;
    /* pos/neg sample ratio */
    float ratio;
    /* user latent factor */
    float** w_user;
    /* item latent factor */
    float** w_item;

    inline void alloc_space(void) {
        // allocate space for user/item factor
        //cout << "num_user: " << num_user << endl;
        //cout << "num_word: " << num_item << endl;
        //cout << "num_factor: " << num_factor << endl;
        if (num_user > 0 && num_factor > 0) {
            w_user = new float*[num_user];

            for (int i = 0; i < num_user; ++i) {
                w_user[i] = new float[num_factor];
            }
        }

        if (num_item > 0 && num_factor > 0) {
            w_item = new float*[num_item];

            for (int i = 0; i < num_item; ++i) {
                w_item[i] = new float[num_factor];
            }
        }
    }

    inline void free_space(void) {
        // free space for model
        if (w_user != NULL) {
            for (int i = 0; i < num_user; ++i) {
                delete[] w_user[i];
            }

            delete[] w_user;
            w_user = NULL;
        }

        if (w_item != NULL) {
            for (int i = 0; i < num_item; ++i) {
                delete[] w_item[i];
            }

            delete[] w_item;
            w_item = NULL;
        }

    }

    inline void rand_init(void) {
        //random initializer the model parameters
        //scaling for predict
        if (w_user != NULL) {
            for (int i = 0; i < num_user; ++i)
                for (int j = 0; j < num_factor; ++j) {
                    w_user[i][j] = ty_random::sample_normal() / sqrt(num_factor);
                }
        }

        if (w_item != NULL) {
            for (int i = 0; i < num_item; ++i)
                for (int j = 0; j < num_factor; ++j) {
                    w_item[i][j] = ty_random::sample_normal() / sqrt(num_factor);
                }
        }
    }

public:
    //friend class ImplicitCFLearning;
    ModelParam() {
        num_user = num_item = num_factor = 0;
        ratio = 1.0f;
        w_user = NULL;
        w_item = NULL;
    }

    inline void init() {
        num_user = num_item = num_factor = 0;
        ratio = 1.0f;
        w_user = NULL;
        w_item = NULL;
    }

    inline void set_param(const char* name, const char* val) {
        if (!strcmp("num_user", name)) {
            num_user = atoi(val);
        }

        if (!strcmp("num_item", name)) {
            num_item = atoi(val);
        }

        if (!strcmp("num_factor", name)) {
            num_factor = atoi(val);
        }

        if (!strcmp("ratio", name)) {
            ratio = (float)atof(val);
        }
    }
    inline void set_param_unsigned(const char* name, const unsigned val) {
        if (!strcmp("num_user", name)) {
            num_user = val;
        }

        if (!strcmp("num_item", name)) {
            num_item = val;
        }

        if (!strcmp("num_factor", name)) {
            num_factor = val;
        }
    }

    inline float get_ratio() {
        return ratio;
    }
    inline unsigned get_num_user() {
        return num_user;
    }
    inline unsigned get_num_item() {
        return num_item;
    }
    inline unsigned get_num_factor() {
        return num_factor;
    }
    inline float* get_user_factor(unsigned i) {
        if (w_user != NULL && i < num_user) {
            return w_user[i];
        } else {
            return NULL;
        }
    }
    inline float* get_item_factor(unsigned i) {
        if (w_item != NULL && i < num_item) {
            return w_item[i];
        } else {
            return NULL;
        }
    }

    inline void update_param(const char* name, unsigned i, unsigned j, float new_value) {
        if (!strcmp("w_user", name) && w_user != NULL && i < num_user && j < num_factor) {
            w_user[i][j] = new_value;
        }

        if (!strcmp("w_item", name) && w_item != NULL && i < num_item && j < num_factor) {
            w_item[i][j] = new_value;
        }
    }

    void init_model(void) {
        this->alloc_space();
        this->rand_init();
    }

    virtual ~ModelParam() {
        this->free_space();
    }

};


class ImplicitCFLearning {
private:
    int init_end;
    TrainParam* train_param;
    ModelParam* model_param;
    SparseFeatureArray* train_data;
    vector<unsigned> words_pool;
    char name_config[256];
    // whether to be silent
    int silent;
    int num_round, train_repeat, max_round;
    // config file parser
    apex_utils::ConfigSaver cfg;

    /* training file */
    char name_train[256];
    /* uid map file */
    char name_uid_map[256];
    /* wordid map file */
    char name_wordid_map[256];
    /* testing file */
    char name_test[256];

    /* target user */
    unsigned target_userid;

    inline void reset_default() {
        init_end = 0;
        strcpy(name_config, "config.conf");
        num_round = 200;
        train_repeat = 1;
        silent = 0;
        max_round = INT_MAX;
        strcpy(name_train, "train.txt");
        strcpy(name_test, "test.txt");
        strcpy(name_uid_map, "uid_map.txt");
        strcpy(name_wordid_map, "wordid_map.txt");
        target_userid = 0;
    }
public:
    ImplicitCFLearning();
    ImplicitCFLearning(unsigned, unsigned, vector<vector<pair<unsigned, unsigned> > >&,
            HashTable<unsigned, string>&, HashTable<unsigned, unsigned>&);
    virtual ~ImplicitCFLearning();
private:
    inline void set_param(const char* name, const char* val) {
        if (!strcmp(name, "max_round")) {
            max_round = atoi(val);
        }

        if (!strcmp(name, "num_round")) {
            num_round = atoi(val);
        }

        if (!strcmp(name, "silent")) {
            silent = atoi(val);
        }

        if (!strcmp(name, "train_repeat")) {
            train_repeat = atoi(val);
        }

        if (!strcmp("name_train", name)) {
            strcpy(name_train, val);
        }

        if (!strcmp("name_test", name)) {
            strcpy(name_test, val);
        }

        if (!strcmp("name_uid_map", name)) {
            strcpy(name_uid_map, val);
        }

        if (!strcmp("name_wordid_map", name)) {
            strcpy(name_wordid_map, val);
        }

        if (!strcmp(name, "target_userid")) {
            target_userid = atoi(val);
        }
    }

    inline void configure(void) {
        apex_utils::ConfigIterator itr(name_config);

        while (itr.next()) {
            cfg.push_back(itr.name(), itr.val());
        }

        cfg.before_first();

        while (cfg.next()) {
            set_param(cfg.name(), cfg.val());
        }
    }

    // configure train param
    inline void configure_train(void) {
        cfg.before_first();

        while (cfg.next()) {
            train_param->set_param(cfg.name(), cfg.val());
        }
    }

    // configure model param, must load train data first
    inline void configure_model(void) {
        // configure number of users and items
        model_param->set_param_unsigned("num_user", train_data->get_user_num());
        model_param->set_param_unsigned("num_item", train_data->get_word_num());
        // configure file
        cfg.before_first();

        while (cfg.next()) {
            model_param->set_param(cfg.name(), cfg.val());
        }
    }
    inline void get_words_pool() {
        words_pool = train_data->get_words_pool();
    }

    // load train data
    void load(void) {
        //train_data->load(name_train, name_uid_map, name_wordid_map, target_userid);
        get_words_pool();
    }

    // load test data
    void load_test(void) {
        train_data->load_test(name_test);
    }

    void init(void) {
        // configure the parameters
        this->configure();
        // configure train
        this->configure_train();
        // load data first
        this->load();
        // configure model
        this->configure_model();
        model_param->init_model();

        this->init_end = 1;
    }

public:

    inline void set_configer(int argc, char* argv[]) {
        strcpy(name_config, argv[1]);

        if (argc > 2) {
            for (int i = 2; i < argc; i++) {
                char name[256], val[256];

                if (sscanf(argv[i] , "%[^=]=%[^\n]", name , val) == 2) {
                    this->set_param(name, val);
                    model_param->set_param(name, val);
                    train_param->set_param(name, val);
                }
            }
        }
    }
    /*inline unsigned get_uid(const string &s){
        return train_data->get_uid(s);
    }*/

    inline set<unsigned> get_target_uid() {
        if (target_userid != 0) {
            return train_data->target_userid_set;
        }
    }

    // predict
    inline float predict(const unsigned uid, const unsigned wid) {
        float* user_vector = model_param->get_user_factor(uid);
        float* item_vector = model_param->get_item_factor(wid);
        unsigned num_factor = model_param->get_num_factor();

        float ret = 0;

        if (user_vector != NULL && item_vector != NULL) {
            for (int i = 0; i < num_factor; ++i) {
                ret += user_vector[i] * item_vector[i];
            }
        }

        return ret;
    }

    // negative sampling
    map<unsigned, unsigned>  negative_sample(const unsigned uid,
            const vector<pair<unsigned, unsigned> >& word_pair);

    // sgd learning
    void sgd_run(void);
    void sgd_run_adagrad(void);

    // recommend
    vector<pair<unsigned, float> > recommend(const unsigned uid, float thre, unsigned num_item,
            unsigned num_factor);

    void online_recommend(unsigned tradeid);

};




#endif  //__IMPLICIT_CF_LEARN_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
