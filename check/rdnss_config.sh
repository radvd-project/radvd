#!/bin/bash

. check/common.sh

function baseline {
echo "config file syntax ok."
}

function output {
cat << EOF > $RADVD_CONF 

interface eth0 {
     AdvSendAdvert on;
     MinRtrAdvInterval 20;
     MaxRtrAdvInterval 60;
     AdvLinkMTU 1472;
     prefix 2002:0000:0000::/64 {
             AdvOnLink off;
             AdvAutonomous on;
             AdvRouterAddr on; 
             AdvPreferredLifetime 90;
             AdvValidLifetime 120;
     };
     RDNSS 2001:470:20::2
     {
             AdvRDNSSLifetime 60;
     };
};

EOF

./radvd -C $RADVD_CONF -c 2>&1 | trim_logging || die "radvd failed"
}

run

