#!/bin/bash

imf_result="/app/ecom/mpp/tangye01/wise_recommendation/imf_adagrad_result/part-*"
user_buy_word_path="/app/ecom/mpp/tangye01/wise_recommendation/user_buy_word/part-*"

output_path="/app/ecom/mpp/tangye01/wise_recommendation/final_result/"
output=$output_path


/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="wise_recommendation_imf_Tangye" \
    -D stream.num.map.output.key.fields=4 \
    -D num.key.fields.for.partition=1 \
    -D mapred.job.reduce.capacity=200 \
    -D mapred.reduce.memory.limit=1000 \
    -D mapred.reduce.tasks=1000 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$imf_result" \
    -input "$user_buy_word_path" \
    -output "$output" \
    -mapper "cat" \
    -reducer "python2.7/bin/python reducer_02_filtering.py" \
    -file "reducer_02_filtering.py" \
    -cacheArchive "/share/python2.7.tar.gz#python2.7" 
echo "filtering done."

recom_result=../../data/recommend_data/hadoop_adagrad_recom_result

if [ -f $recom_result ]; then
    rm $recom_result
fi

/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -getmerge $output $recom_result 
