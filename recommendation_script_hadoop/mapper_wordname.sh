#!/bin/bash


wordid=$1;

awk 'BEGIN{FS=OFS="\t";
    while(getline<"'$wordid'")e_wordid[$4]=1;
    }{
    if($1 in e_wordid){
        print $0;
    }
}END{
}' 
