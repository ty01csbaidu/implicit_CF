#!/bin/bash

./step01_imf_hadoop.sh

./step02_filter_buy_words.sh


recom_result=../../data/recommend_data/hadoop_adagrad_recom_result
wordname=../../data/recommend_data/wordname
final_result_wordname=../../data/recommend_data/final_result_wordname

./step03_add_wordname.sh $recom_result $wordname $final_result_wordname 
