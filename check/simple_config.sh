#!/bin/bash

. check/common.sh

function baseline {
echo "config file syntax ok."
}

function output {
cat << EOF > $RADVD_CONF 

interface eth0 {
     AdvSendAdvert on;
     prefix 2002:0000:0000::/64;
};

EOF

./radvd -C $RADVD_CONF -c 2>&1 | trim_logging || die "radvd failed"
}

run

