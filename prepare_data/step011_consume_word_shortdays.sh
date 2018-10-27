#!/bin/bash

begin_date=$1
end_date=$2

if [ $begin_date -gt $end_date ]; then
    echo "begin_date > end_date";
    exit -1;
fi


function extractDateList(){
    begindate=`date -d "$1" +%Y%m%d`
    enddate=`date -d "$2" +%Y%m%d`
    result=$begindate
    while [ $begindate -lt $enddate ]
    do
        begindate=`date -d "1 days $begindate" +%Y%m%d`
        result=$result","$begindate
    done
    echo -n $result
}

date_list=`extractDateList $begin_date $end_date`

#fc_view="/app/dt/udw/release/app/fengchao/shitu/{$date_list}/*/part-*"
fc_view="/app/ecom/fcr-important/shitu-log-wise/222_223/{$date_list}/*/part-*"

output_path="/app/ecom/mpp/tangye01/wise_recommendation/uid_word_match/show/"
output=$output_path""$begin_date"_"$end_date


/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="wise_recommendation_adpv_trend_Tangye" \
    -D stream.num.map.output.key.fields=4 \
    -D num.key.fields.for.partition=1 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$fc_view" \
    -output "$output" \
    -mapper "python2.7/bin/python mapper_01_consume_word.py" \
    -reducer "python2.7/bin/python reducer_01_consume_word.py" \
    -file "mapper_01_consume_word.py" \
    -file "reducer_01_consume_word.py" \
    -cacheArchive "/share/python2.7.tar.gz#python2.7"
echo "$step $begin_date $end_date done."

