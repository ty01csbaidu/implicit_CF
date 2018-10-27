#!/bin/bash

begin_date=$1
end_date=$2

if [ $begin_date -gt $end_date ]; then
    echo "begin_date > end_date";
    exit -1;
fi



# each month a job
iter_date=$begin_date;
while [ $iter_date -le $end_date ]
do
    iter_begin_date=$iter_date
    last_day=`date -d "next week $iter_begin_date" +%Y%m%d`
    ((iter_date=($last_day>$end_date)?$end_date:$last_day))
    echo "begin interval: " $iter_begin_date"_"$iter_date
    ./step011_consume_word_shortdays.sh $iter_begin_date $iter_date &
    #if [ $month == "5" -o $month == "10" ]; then
    #    wait
    #fi
    echo $iter_begin_date"_"$iter_date " done"
    iter_date=`date -d "next -day $iter_date" +%Y%m%d`
done

wait


# merge
input="/app/ecom/mpp/tangye01/wise_recommendation/uid_word_match/show/*/part-*"

output_path="/app/ecom/mpp/tangye01/wise_recommendation/uid_word_match/merge/"
output=$output_path""$begin_date"_"$end_date


/home/work/mpp-rd/lipiji/tools/hadoop-client.2/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output 
/home/work/mpp-rd/lipiji/tools/hadoop-client.2/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="word_matching_"$begin_date"_"$cur_date"_Tangye" \
    -D stream.num.map.output.key.fields=4 \
    -D num.key.fields.for.partition=1 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$input" \
    -output "$output" \
    -mapper "cat" \
    -reducer "python2.7/bin/python reducer_01_merge_word.py" \
	-file "reducer_01_merge_word.py" \
    -cacheArchive "/share/python2.7.tar.gz#python2.7"
echo "$step $begin_date $cur_date done."

consume_words=../../data/prepare_data/user_consume_words

if [ -f $consume_words ]; then
    rm $consume_words
fi

/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -getmerge $output $consume_words 
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -rmr /app/ecom/mpp/tangye01/wise_recommendation/uid_word_match/show/
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -rmr $output
