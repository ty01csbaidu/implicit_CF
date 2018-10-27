#!/bin/bash

imf_train_data="/app/ecom/mpp/tangye01/wise_recommendation/imf/train_data"

output_path="/app/ecom/mpp/tangye01/wise_recommendation/imf_adagrad_result/"
output=$output_path


/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="wise_recommendation_imf_Tangye" \
    -D stream.num.map.output.key.fields=2 \
    -D num.key.fields.for.partition=1 \
    -D mapred.job.reduce.capacity=200 \
    -D mapred.reduce.memory.limit=10000 \
    -D mapred.reduce.tasks=1000 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$imf_train_data" \
    -output "$output" \
    -mapper "cat" \
    -reducer "./execute imf.conf" \
    -file "imf.conf" \
    -file "execute" 
echo "imf done."

