#!/bin/bash
#time_stamp=20150308000000/wordlist_2015-03-08


yesterday=`date -d "yesterday" +%Y%m%d`;
time_stamp="time_stamp="$yesterday"000000";
word_file="wordlist_"${yesterday:0:4}"-"${yesterday:4:2}"-"${yesterday:6:2}
echo $word_file
wordid=$1;
hadoop_wordid=${wordid##*/}
out=$2;
final_result_wordname=$3;

input="/app/ecom/aries/galaxy/hive_dw/risk_data.db/fc_word_literal_day/"$time_stamp"/"$word_file;
output_path="/app/ecom/mpp/tangye01/wise_recommendation/wordid_wordname/"
output=$output_path


/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -D hadoop.job.ugi=mpp,mpp213456 -rmr $output
/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop streaming \
    -D hadoop.job.ugi="mpp,mpp213456" \
    -D mapred.job.queue.name="tmp" \
    -D mapred.job.priority=VERY_HIGH \
    -D mapred.job.name="wise_recommendation_buyword_Tangye" \
    -D stream.num.map.output.key.fields=2 \
    -D num.key.fields.for.partition=1 \
    -partitioner org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner \
    -input "$input" \
    -mapper "mapper_wordname.sh $hadoop_wordid" \
    -file "mapper_wordname.sh" \
    -output "$output" \
    -file "$wordid" \
    -cacheArchive "/share/python2.7.tar.gz#python2.7"
echo "wordname done."

rm $out

/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -getmerge $output $out 

awk 'BEGIN{FS=OFS="\t";
	while(getline<"'$out'")
		wordname[$1]=$2;
}{
	if($4 in wordname)
		print $2, wordname[$4], 31;
}' $wordid > $final_result_wordname


