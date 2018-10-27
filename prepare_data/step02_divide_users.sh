#!/bin/bash

usrList=$1

today=`date -d "-1 days" +%Y%m%d`

newUsrTrade=../../data/prepare_data/newUsrTrade
usrTrade=../../data/prepare_data/usrTrade

wget m1-dr-hangye01.m1.baidu.com:/home/work/sf-herring/newtrade_db_in/output/db_new_user_trade.txt.$today -O $newUsrTrade


awk -F "\t" 'BEGIN{OFS="\t";
	while(getline<"'$usrList'")
		users[$1]=1;
}{
	if( $1 in users)
	{
		print $1, $2, $3, $4;
	}
}
' $newUsrTrade > $usrTrade 
