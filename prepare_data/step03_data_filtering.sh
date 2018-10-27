#!/bin/bash

################
# plan fitering(num_words in the plan > 2)
# word fitering?(word only occur in a plan)
#
# input: userid, planid, unitid, wordid, count

user_consume_word=$1;
user_trade=$2;
final_imf_input=$3;

shrink_user_consume_word=../../data/prepare_data/shrink_user_consume_word
effective_plan=../../data/prepare_data/effective_plan
filtered_user_consume_word=../../data/prepare_data/filtered_user_consume_word
trade_user_consume_word=../../data/prepare_data/trade_user_consume_word
effective_trade=../../data/prepare_data/effective_trade
sorted_trade_user_consume_word=../../data/prepare_data/sorted_trade_user_consume_word

####################
# shrink user consume words
awk 'BEGIN{FS=OFS="\t";
    while(getline<"'$user_trade'")
        uid_trade[$1]=$3; #trade_2
}{
	if($1 in uid_trade){
        if(uid_trade[$1]=="8204"){
            if($5+0>4){
                print $0
            }
        }else if(uid_trade[$1]=="8201" || uid_trade[$1]=="8202"){
			if($5+0>3){
				print $0
			}
		}else{
			if(uid_trade[$1]=="8203"){
				if($5+0>2){
					print $0
				}
			}else{
				print $0
			}
		}	
	}
}' $user_consume_word > $shrink_user_consume_word

awk 'BEGIN{FS=OFS="\t";}{
    plan[$2]+=1; 
}END{
    for(p in plan){
        if(plan[p] > 5)
            print p
    }
}' $shrink_user_consume_word > $effective_plan


awk 'BEGIN{FS=OFS="\t";
    while(getline<"'$effective_plan'"){
        plan[$1]=1;
    }
}{
    if($2 in plan)
        print $1,$2,$3,$4;
}' $shrink_user_consume_word > $filtered_user_consume_word 


############
# add tradeid and mapping

awk 'BEGIN{FS=OFS="\t";
    while(getline<"'$user_trade'")
        uid_trade[$1]=$3; #trade_2
}{
    if($1 in uid_trade)
        print uid_trade[$1], $0;
}' $filtered_user_consume_word > $trade_user_consume_word


############
# trade filtering (num_users in trade > 1)
# sort tradeid uid planid

awk 'BEGIN{FS=OFS="\t";}{
    trade_user[$1"&"$2]=1;
}END{
    for(t_u in trade_user){
        split(t_u,a,"&");
        trade[a[1]]++;
    }
    for(t in trade){
        if(trade[t]>1)
            print t, trade[t]
    }
}' $trade_user_consume_word > $effective_trade

awk 'BEGIN{FS=OFS="\t";
    while(getline<"'$effective_trade'")
       e_trade[$1]=1; 
}{
    if($1 in e_trade)
        print $0
}' $trade_user_consume_word | sort -k1,4 > $sorted_trade_user_consume_word

python prepare_input_cf.py $sorted_trade_user_consume_word $final_imf_input 

##############
# copy to hadoop
#imf_hadoop_train=/app/ecom/mpp/tangye01/wise_recommendation/imf/train_data
#/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -rmr $imf_hadoop_train
#/home/work/tools/hadoop-client-szwg/hadoop/bin/hadoop fs -copyFromLocal $final_imf_input $imf_hadoop_train

#rm $filtered_user_consume_word
#rm $trade_user_consume_word
#rm $sorted_trade_user_consume_word
#rm $effective_trade
#rm $effective_plan
