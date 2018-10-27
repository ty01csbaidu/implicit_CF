/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file implicit_CF_learn.cpp
 * @author tangye01(com@baidu.com)
 * @date 2015/02/12 11:01:28
 * @brief
 *
 **/


#include "implicit_CF_learn.h"


float compute_l2_norm(const float* t_vector, const unsigned n_d) {
    if (n_d <= 0) {
        return 0;
    }

    float ret = 0;

    for (unsigned i = 0; i < n_d; ++i) {
        ret += t_vector[i] * t_vector[i];
    }

    ret = sqrt(ret);
    return ret;
}

ImplicitCFLearning::ImplicitCFLearning() {
    train_param = new TrainParam();
    model_param = new ModelParam();
    train_data = new SparseFeatureArray();
    this->reset_default();
}

ImplicitCFLearning::ImplicitCFLearning(unsigned ex_num_user, unsigned ex_num_word,
        vector<vector<pair<unsigned, unsigned> > >& ex_data, HashTable<unsigned, string>& ex_uid_map,
        HashTable<unsigned, unsigned>& ex_wid_map) {
    train_param = new TrainParam();
    model_param = new ModelParam();
    train_data = new SparseFeatureArray(ex_num_user, ex_num_word, ex_data, ex_uid_map, ex_wid_map);
    this->reset_default();
}

ImplicitCFLearning::~ImplicitCFLearning() {
    if (init_end) {
        delete train_param;
        delete model_param;
        delete train_data;
    }
}

// negative sampling
map<unsigned, unsigned>  ImplicitCFLearning::negative_sample(const unsigned uid,
        const vector<pair<unsigned, unsigned> >& word_pair) {
    float ratio = model_param->get_ratio();
    map<unsigned, unsigned>  item_sample;

    // get all positve sample for this user
    for (vector<pair<unsigned, unsigned> >::const_iterator iter = word_pair.begin();
            iter != word_pair.end(); ++iter) {
        //cout << "pos item: " << iter->first << endl;
        item_sample[iter->first] = 1;
    }

    unsigned num_positive_sample = item_sample.size();
    unsigned num_negative_sample = 0;
    unsigned pool_size = words_pool.size();

    //cerr << "words_pool size: " << pool_size << endl;
    if (pool_size > 0) {
        for (int i = 0; i < 3 * num_positive_sample; ++i) {
            //unsigned random_index = ty_random::next_int(pool_size);
            //cerr << "random_index: " << random_index << endl;
            unsigned word = words_pool[ty_random::next_int(pool_size)];
            map<unsigned, unsigned>::iterator sample_iter = item_sample.find(word);

            if (sample_iter == item_sample.end()) {
                item_sample[word] = 0;
                num_negative_sample++;

                if (num_negative_sample % 10 == 0 && num_negative_sample > 0) {
                    if ((float)num_positive_sample / num_negative_sample < ratio) {
                        break;
                    }
                }
            }

        }
    }

    return item_sample;
}


// sgd learning
void ImplicitCFLearning::sgd_run() {
    init();

    if (!silent) {
        //printf("initial end, start updating");
    }

    time_t start = time(NULL);
    unsigned long elapsed = 0;
    unsigned num_factor = model_param->get_num_factor();
    //vector<Entry> data = train_data->get_data();

    float loss = 0;

    for (int i = 0; i < num_round; ++i) {
        time_t start = time(NULL);
        loss = 0;

        //for(vector<Entry>::iterator iter = data.begin(); iter != data.end(); ++iter){
        for (vector<vector<pair<unsigned, unsigned> > >::iterator iter = (train_data->data).begin();
                iter != (train_data->data).end(); ++iter) {
            unsigned uid = iter - (train_data->data).begin();
            //cout << "uid: " << uid << endl;
            map<unsigned, unsigned> samples = negative_sample(uid, *iter);

            for (map<unsigned, unsigned>::iterator s_iter = samples.begin(); s_iter != samples.end();
                    ++s_iter) {
                unsigned wid = s_iter->first;
                //cout << "wid: " << wid << endl;
                float rui = (float)s_iter->second;
                float* user_vector = model_param->get_user_factor(uid);
                float* item_vector = model_param->get_item_factor(wid);
                float predict_ui = 0;

                //cout << "step0" << endl;
                if (user_vector != NULL && item_vector != NULL) {
                    for (int f = 0; f < num_factor; ++f) {
                        predict_ui += user_vector[f] * item_vector[f];
                    }

                    float eui = rui - predict_ui;

                    //cout << "step1" << endl;
                    //cout << eui << endl;
                    float norm_user_vector = compute_l2_norm(user_vector, num_factor);
                    float norm_item_vector = compute_l2_norm(item_vector, num_factor);

                    for (int f = 0; f < num_factor; ++f) {
                        //    cout << user_vector[f] << endl;
                        float new_value = (norm_user_vector > 0) ? (user_vector[f] + train_param->learning_rate *
                                          (eui * item_vector[f] - train_param->wd_user * user_vector[f])) / norm_user_vector :
                                          (user_vector[f] + train_param->learning_rate * (eui * item_vector[f] - train_param->wd_user *
                                                  user_vector[f]));
                        model_param->update_param("w_user", uid, f, new_value);
                        new_value = (norm_item_vector > 0) ? (item_vector[f] + train_param->learning_rate *
                                    (eui * user_vector[f] - train_param->wd_item * item_vector[f])) / norm_item_vector :
                                    (item_vector[f] + train_param->learning_rate * (eui * user_vector[f] - train_param->wd_item *
                                            item_vector[f]));
                        model_param->update_param("w_item", wid, f, new_value);
                        //  cout << user_vector[f] << endl;
                    }
                }

            }

            float* user_vector = model_param->get_user_factor(uid);

            for (map<unsigned, unsigned>::iterator s_iter = samples.begin(); s_iter != samples.end();
                    ++s_iter) {
                unsigned wid = s_iter->first;
                float rui = (float)s_iter->second;
                float* item_vector = model_param->get_item_factor(wid);
                float predict_ui = 0;

                if (user_vector != NULL && item_vector != NULL) {
                    for (int f = 0; f < num_factor; ++f) {
                        predict_ui += user_vector[f] * item_vector[f];
                    }

                    float eui = rui - predict_ui;
                    loss += eui * eui;
                }

            }

        }

        //cout << "loss: " << loss << endl;
        train_param->learning_rate *= 0.9;
        time_t end = time(NULL);
        //printf("The pause used %f seconds.\n",difftime(end,start));
    }

}


void ImplicitCFLearning::sgd_run_adagrad() {
    init();

    if (!silent) {
        //printf("initial end, start updating");
    }

    // gradient accumulators
    unsigned num_user = model_param->get_num_user();
    unsigned num_item = model_param->get_num_item();
    unsigned num_factor = model_param->get_num_factor();
    float** user_acc;

    if (num_user > 0 && num_factor > 0) {
        user_acc = new float*[num_user];

        for (int i = 0; i < num_user; ++i) {
            user_acc[i] = new float[num_factor];
            memset(user_acc[i], 0, sizeof(float)*num_factor);
        }
    }

    float** item_acc;

    if (num_item > 0 && num_factor > 0) {
        item_acc = new float*[num_item];

        for (int i = 0; i < num_item; ++i) {
            item_acc[i] = new float[num_factor];
            memset(item_acc[i], 0, sizeof(float)*num_factor);
        }
    }


    //time_t start = time(NULL);
    //unsigned long elapsed = 0;
    //vector<Entry> data = train_data->get_data();

    float loss = 0;
    float pre_loss = 0;
    float delta = 0;
    float local_gradient = 0;
    float new_value = 0;

    for (int i = 0; i < num_round; ++i) {
        //time_t start = time(NULL);
        pre_loss = loss;
        loss = 0;

        for (vector<vector<pair<unsigned, unsigned> > >::iterator iter = (train_data->data).begin();
                iter != (train_data->data).end(); ++iter) {
            unsigned uid = iter - (train_data->data).begin();
            //cout << "uid: " << uid << endl;
            map<unsigned, unsigned> samples = negative_sample(uid, *iter);

            for (map<unsigned, unsigned>::iterator s_iter = samples.begin(); s_iter != samples.end();
                    ++s_iter) {
                unsigned wid = s_iter->first;
                //cout << "wid: " << wid << endl;
                float rui = (float)s_iter->second;
                float* user_vector = model_param->get_user_factor(uid);
                float* item_vector = model_param->get_item_factor(wid);
                float predict_ui = 0;

                //cout << "step0" << endl;
                if (user_vector != NULL && item_vector != NULL) {
                    for (int f = 0; f < num_factor; ++f) {
                        predict_ui += user_vector[f] * item_vector[f];
                    }

                    float eui = rui - predict_ui;

                    //cout << "step1" << endl;
                    //cout << eui << endl;
                    float norm_user_vector = compute_l2_norm(user_vector, num_factor);
                    float norm_item_vector = compute_l2_norm(item_vector, num_factor);

                    for (int f = 0; f < num_factor; ++f) {
                        //    cout << user_vector[f] << endl;
                        // negative gradient
                        local_gradient = eui * item_vector[f] - train_param->wd_user * user_vector[f];
                        user_acc[uid][f] += local_gradient * local_gradient;
                        new_value = (user_acc[uid][f] > 0) ? (user_vector[f] + train_param->learning_rate *
                                    local_gradient / sqrt(user_acc[uid][f])) : user_vector[f];
                        new_value = (norm_user_vector > 0) ? (new_value) / norm_user_vector : new_value;
                        model_param->update_param("w_user", uid, f, new_value);
                        local_gradient = eui * user_vector[f] - train_param->wd_item * item_vector[f];
                        item_acc[wid][f] += local_gradient * local_gradient;
                        new_value = (item_acc[wid][f] > 0) ? (item_vector[f] + train_param->learning_rate *
                                    local_gradient / sqrt(item_acc[wid][f])) : item_vector[f];
                        new_value = (norm_item_vector > 0) ? new_value / norm_item_vector : new_value;
                        model_param->update_param("w_item", wid, f, new_value);
                        //  cout << user_vector[f] << endl;
                    }
                }

            }

            float* user_vector = model_param->get_user_factor(uid);

            for (map<unsigned, unsigned>::iterator s_iter = samples.begin(); s_iter != samples.end();
                    ++s_iter) {
                unsigned wid = s_iter->first;
                float rui = (float)s_iter->second;
                float* item_vector = model_param->get_item_factor(wid);
                float predict_ui = 0;

                if (user_vector != NULL && item_vector != NULL) {
                    for (int f = 0; f < num_factor; ++f) {
                        predict_ui += user_vector[f] * item_vector[f];
                    }

                    float eui = rui - predict_ui;
                    loss += eui * eui;
                }

            }

        }

        delta = abs(pre_loss - loss);

        if (delta < 1) {
            break;
        }

        //cout << "loss: " << loss << endl;
        //train_param->learning_rate *= 0.9;
        //time_t end = time(NULL);
        //printf("The pause used %f seconds.\n",difftime(end,start));
    }//for (int i = 0; i < num_round; ++i)


    // vector normalization
    for (int i = 0; i < num_user; ++i) {
        float* user_vector = model_param->get_user_factor(i);
        float norm_user_vector = compute_l2_norm(user_vector, num_factor);

        if (norm_user_vector > 0) {
            for (int f = 0; f < num_factor; ++f) {
                model_param->update_param("w_user", i, f, user_vector[f] / norm_user_vector);
            }
        }
    }

    for (int i = 0; i < num_item; ++i) {
        float* item_vector = model_param->get_item_factor(i);
        float norm_item_vector = compute_l2_norm(item_vector, num_factor);

        if (norm_item_vector > 0) {
            for (int f = 0; f < num_factor; ++f) {
                model_param->update_param("w_item", i, f, item_vector[f] / norm_item_vector);
            }
        }
    }


    //delete gradient accumulator
    if (user_acc != NULL) {
        for (int i = 0; i < num_user; ++i) {
            delete[] user_acc[i];
        }

        delete[] user_acc;
        user_acc = NULL;
    }

    if (item_acc != NULL) {
        for (int i = 0; i < num_item; ++i) {
            delete[] item_acc[i];
        }

        delete[] item_acc;
        item_acc = NULL;
    }

}


bool pairCompare(const pair<unsigned, float>& first_element,
        const pair<unsigned, float>& second_element) {
    return first_element.second > second_element.second;
}


vector<pair<unsigned, float> > ImplicitCFLearning::recommend(const unsigned uid, float thre,
        unsigned num_item, unsigned num_factor) {
    /*if (uid >= (train_data->data).size()) {
        cerr << "uid exceed! " << endl;
        exit(-1);
    }*/

    //set<unsigned> positive_item;
    //set<unsigned>::iterator s_iter;
    //for(vector<pair<unsigned, unsigned> >::iterator iter = (train_data->data[uid]).begin(); iter != (train_data->data[uid]).end(); ++iter){
    //cout << iter->first << endl;
    //positive_item.insert(iter->first);
    //cout << predict(uid,iter->first) <<endl;
    //}


    vector<pair<unsigned, float> > recom_ret;
    float* user_vector = model_param->get_user_factor(uid);

    if (user_vector != NULL) {
        //float user_vector_norm = compute_l2_norm(user_vector, num_factor);

        //time_t start = time(NULL);
        //if (user_vector_norm > 0) {
        for (unsigned i = 0; i < num_item; i++) {
            float predict_ui = 0;
            float* item_vector = model_param->get_item_factor(i);

            if (item_vector != NULL) {
                //float item_vector_norm = compute_l2_norm(item_vector, num_factor);

                //if (item_vector_norm > 0) {
                for (int f = 0; f < num_factor; ++f) {
                    predict_ui += user_vector[f] * item_vector[f];
                }

                //predict_ui = predict_ui / (user_vector_norm * item_vector_norm);
                //}
            }

            if (predict_ui > thre) {
                recom_ret.push_back(make_pair(i, predict_ui));
            }
        }

        std::sort(recom_ret.begin(), recom_ret.end(), pairCompare); //reverse sort
        //}//if(user_vector_norm > 0)

        //time_t end = time(NULL);
        //printf("The product used %f seconds.\n",difftime(end,start));

        //start = time(NULL);
        //end = time(NULL);
        //printf("The sort used %f seconds.\n",difftime(end,start));

    }//if(user_vector != NULL)

    return recom_ret;
}


void SplitString(const string& s, vector<string>& v, const string& c) {
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;

    while (string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);

    }

    if (pos1 != s.length()) {
        v.push_back(s.substr(pos1));
    }
}

void ImplicitCFLearning::online_recommend(unsigned tradeid) {
    HashTable<unsigned, string>::iterator uid_iter;
    unsigned num_item = model_param->get_num_item();
    unsigned num_factor = model_param->get_num_factor();

    for (vector<vector<pair<unsigned, unsigned> > >::iterator iter = (train_data->data).begin();
            iter != (train_data->data).end(); ++iter) {

        unsigned uid = iter - (train_data->data).begin();
        uid_iter = (train_data->uid_map).find(uid);

        if (uid_iter != (train_data->uid_map).end()) {
            vector<pair<unsigned, float> > tmp_recommend_result = recommend(uid, 0.1, num_item, num_factor);

            for (vector<pair<unsigned, float> >::iterator wid_iter = tmp_recommend_result.begin();
                    wid_iter != tmp_recommend_result.end(); ++wid_iter) {
                cout << tradeid << "\t" << train_data->uid_map[uid] << "\t";
                cout << train_data->wid_map[wid_iter->first] << "\t" << wid_iter->second << endl;
            }
        }
    }
}




void reduce(int argc, char* argv[]) {
    //read data for cin
    //run im_cf_leanring
    //wirte result to cout
    //
    //input: tradeid, label  1: uid, userid, planid
    //                       2: wordid, word
    //                       3: uid, wordid
    //                       4: userid, planid, word
    //output: tradeid, userid, planid, word, score


    //variables for local im_cf_learning class
    unsigned num_user;
    unsigned num_word;
    vector<vector<pair<unsigned, unsigned> > > data;
    HashTable<unsigned, string> uid_map;
    HashTable<unsigned, unsigned> wid_map;

    //variables for local trade
    unsigned count_user = 0;
    unsigned count_word = 0;
    unsigned count_train = 0;
    unsigned pre_tradeid = 0;
    unsigned tradeid = 0;
    unsigned label = 0;

    unsigned uid, userid, planid, unitid, wordid, word;
    string joinid;
    //reading
    string line;

    while (getline(cin, line)) {
        //cerr << "1" << endl;
        vector<string> fields;
        SplitString(line, fields, "\t");

        if (fields.size() > 2) {
            tradeid = atoi(fields[0].c_str());
            label = atoi(fields[1].c_str());

            if (label == 1) {
                if (fields.size() == 5) {
                    uid = atoi(fields[2].c_str());
                    userid = atoi(fields[3].c_str());
                    planid = atoi(fields[4].c_str());
                }
            }

            if (label == 2) {
                if (fields.size() == 4) {
                    wordid = atoi(fields[2].c_str());
                    word = atoi(fields[3].c_str());
                }
            }

            if (label == 3) {
                if (fields.size() == 4) {
                    uid = atoi(fields[2].c_str());
                    wordid = atoi(fields[3].c_str());
                }
            }

            if (pre_tradeid == tradeid && tradeid != 0) {
                if (label == 1) {
                    stringstream ss;
                    ss << userid << "\t" << planid;
                    joinid = ss.str();
                    uid_map[uid] = joinid;
                    //cout << "tradeid: " << tradeid << " uid: " << uid << ", joinid: " << joinid << endl;
                    count_user++;
                }

                if (label == 2) {
                    wid_map[wordid] = word;
                    //cout << "tradeid: " << tradeid << " wordid: " << wordid << ", word: " << word << endl;
                    count_word++;
                }

                if (label == 3) {
                    if (data.size() < count_user) {
                        data.resize(count_user);
                    }

                    data[uid].push_back(make_pair(wordid, 0));
                    //cout << "tradeid: " << tradeid << " uid: " << uid << ", wordid: " << wordid << endl;
                    count_train++;
                }

            } else if (pre_tradeid != tradeid) {
                if (pre_tradeid != 0) {
                    // one trade data reading finish, run imf
                    //cout << "train size: " << count_train << endl;
                    //cerr<< "2" << endl;
                    ty_random::seed(10);
                    ImplicitCFLearning im_cf_learning(count_user, count_word, data, uid_map, wid_map);
                    //im_cf_learning.set_configer( argv[1] );
                    im_cf_learning.set_configer(argc, argv);
                    //training
                    im_cf_learning.sgd_run_adagrad();
                    //recommending
                    im_cf_learning.online_recommend(pre_tradeid);


                    //reset initial variables for next trade reading
                    //variables for local im_cf_learning class
                    num_user = num_word = 0;
                    data.clear();
                    uid_map.clear();
                    wid_map.clear();


                    //variables for local trade
                    count_user = count_word = count_train = 0;

                }

                pre_tradeid = tradeid;

                if (label == 1) {
                    stringstream ss;
                    ss << userid << "\t" << planid;
                    joinid = ss.str();
                    uid_map[uid] = joinid;
                    //cout << "tradeid: " << tradeid << " uid: " << uid << ", joinid: " << joinid << endl;
                    count_user++;
                }

                if (label == 2) {
                    wid_map[wordid] = word;
                    //cout << "tradeid: " << tradeid << " wordid: " << wordid << ", word: " << word << endl;
                    count_word++;
                }

                if (label == 3) {
                    if (data.size() < count_user) {
                        data.resize(count_user);
                    }

                    data[uid].push_back(make_pair(wordid, 0));
                    //cout << "tradeid: " << tradeid << " uid: " << uid << ", wordid: " << wordid << endl;
                    count_train++;
                }


            }// else if(pre_tradeid != tradeid)
        }// if(fields.size()>2)
    }//while(getline(cin, line))

    //final trade, run imf
    //cerr << "3" << endl;
    //cerr << "uid size: " << uid_map.size() << endl;
    //cerr << "word size: " << wid_map.size() << endl;
    //cerr << "train size: " << count_train << endl;
    //cerr << "tradeid: " << pre_tradeid << endl;
    //cout << "train size: " << count_train<< endl;
    if ( pre_tradeid != 0 ){
        ty_random::seed(10);
        ImplicitCFLearning im_cf_learning(count_user, count_word, data, uid_map, wid_map);
        //im_cf_learning.set_configer( argv[1] );
        im_cf_learning.set_configer(argc, argv);
        //training
        im_cf_learning.sgd_run_adagrad();
        //recommending
        im_cf_learning.online_recommend(pre_tradeid);
        //cerr << "4" << endl;
    }
}

int main(int argc, char* argv[]) {
    reduce(argc, argv);
}




/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */

