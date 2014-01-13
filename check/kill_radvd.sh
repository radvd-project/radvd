#!/bin/bash

set -u

. check/common.sh


function baseline {
cat << EOF
radvd killed
starup and main radvd PID are the same
wait success
sending stop adverts
returning from radvd main
EOF
}


function output { 
trap "ip tuntap del dev radvd0 mode tap" SIGINT SIGTERM

ip tuntap add dev radvd0 mode tap
ip link set radvd0 up 

./radvd -m logfile -l $RADVD_LOG -d 5 -n &

sleep 1

PID1=$(cat $RADVD_LOG | trim_log | grep "radvd startup PID is " | awk '{print $NF}')
PID2=$(cat $RADVD_LOG | trim_log | grep "radvd PID is " | awk '{print $NF}')
PID3=$(cat $RADVD_LOG | trim_log | grep "radvd privsep PID is " | awk '{print $NF}')

kill $PID2 && echo "radvd killed" || echo "couldn't kill $PID2"

if [[ "$PID1" == "$PID2" ]] ; then
	echo "starup and main radvd PID are the same"
else
	echo "starup and main radvd PID are different"
fi

wait $PID1 && echo "wait success" || echo "wait $PID1 failed"

cat $RADVD_LOG | trim_log | grep "sending stop adverts"
cat $RADVD_LOG | trim_log | grep "returning from radvd main"

ip tuntap del dev radvd0 mode tap
}

run
