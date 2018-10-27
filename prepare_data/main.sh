#!/bin/bash



step01_end_date=`date -d "last day" +%Y%m%d`
#echo $step01_end_date
step01_begin_date=`date -d "last month $step01_end_date" +%Y%m%d`
#echo $step01_begin_date
step01_begin_date=`date -d "next day $step01_begin_date" +%Y%m%d`
#echo $step01_begin_date
./step01_consume_word.sh $step01_begin_date $step01_end_date

user_consume_words=../../data/prepare_data/user_consume_words
usrTrade=../../data/prepare_data/usrTrade
final_imf_input=../../data/prepare_data/final_imf_input

./step02_divide_users.sh $user_consume_words

./step03_data_filtering.sh $user_consume_words $usrTrade $final_imf_input

./step04_uid_buy_word.sh $usrTrade


