#!/bin/bash


user_trade=$1;
hadoop_user_trade=${user_trade##*/}
#############
# yesterday
time_stamp=`date -d "yesterday" +%Y%m%d`;

input="/app/ecom/aries/galaxy/hive_dw/risk_data.db/fc_wordinfo_day/time_stamp="$time_stamp"000000/*"

output_path="/app/ecom/mpp/tangye01/wise_recommendation/user_buy_word/"
output=$output_path


/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="wise_recommendation_buyword_Tangye" \
    -D stream.num.map.output.key.fields=4 \
    -D num.key.fields.for.partition=1 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$input" \
    -output "$output" \
    -mapper "python2.7/bin/python mapper_04_buy.py $hadoop_user_trade" \
    -reducer "python2.7/bin/python reducer_04_buy.py" \
    -file "mapper_04_buy.py" \
    -file "reducer_04_buy.py" \
    -file "$user_trade" \
    -cacheArchive "/share/python2.7.tar.gz#python2.7"
echo "$step $begin_date $end_date done."

