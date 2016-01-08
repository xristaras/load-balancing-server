#! /bin/bash

rand_num=$((RANDOM%100))

if [ $rand_num -lt 10 ]
then
   echo stressing heavily
   stress --cpu 8 --io 4 --vm 2 --hdd 4 --timeout 1m
fi
if [ $rand_num -ge 10 -a $rand_num -lt 30 ]
then
   echo stressing slightly
   stress --cpu 4 --io 2 --vm 1 --hdd 2 --timeout 1m
fi
