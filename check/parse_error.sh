#!/bin/bash

. check/common.sh

function baseline {
cat << EOF
syntax error, unexpected STRING, expecting '}' in $RADVD_CONF, location 4.6-4.22: MinRtrAdvinterval
error parsing or activating the config file: $RADVD_CONF
Exiting, failed to read config file.
EOF
}


function output {
cat << EOF > $RADVD_CONF 

interface eth0 {
     AdvSendAdvert on;
     MinRtrAdvinterval 20;
     MaxRtrAdvInterval 60;
     AdvLinkMTU 1472;
     prefix 2002:0000:0000::/64 {
	     AdvOnLink off;
	     AdvAutonomous on;
	     AdvRouterAddr on; 
	     AdvPreferredLifetime 90;
	     AdvValidLifetime 120;
     };
};

EOF

./radvd -C $RADVD_CONF -c 2>&1 | trim_log || die "radvd failed"
}

run

