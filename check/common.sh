#!/bin/bash

function die {
	echo $1 1>&2
	exit 1
}

BASELINE=$(mktemp /tmp/radvd-baseline.XXXXXXXXXXXXXXX)
OUTPUT=$(mktemp /tmp/radvd-output.XXXXXXXXXXXXXXX)
RADVD_CONF=$(mktemp /tmp/radvd.conf.XXXXXXXXXXXXXXX)
RADVD_LOG=$(mktemp /tmp/radvd.log.XXXXXXXXXXXXXXX)

function check {
DIFF=$(mktemp /tmp/radvd-check-diff.XXXXXXXXXXXXXXX)
if ! diff $1 $2> $DIFF ; then
	echo Expected...
	cat $1
	echo Got...
	cat $2
	echo Diff...
	cat $DIFF
	exit 1
fi
}

function trim_log {
while read data; do
	echo "$data" | sed 's/^.*): //g'
done
}

function run {
baseline > $BASELINE
output > $OUTPUT
check $BASELINE $OUTPUT
}

