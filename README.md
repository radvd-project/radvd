# radvd

radvd is the router advertisement daemon for IPv6. It listens to router
solicitations and sends router advertisements as described in
"Neighbor Discovery for IP Version 6 (IPv6)" (RFC 4861).

With these advertisements hosts can automatically configure their
addresses and some other parameters. They also can choose a default router
based on these advertisements.

It also implements "Neighbor Discovery Optimization for IPv6 over Low-Power
Wireless Personal Area Networks (6LoWPANs)" (RFC6775) & many other related IPv6
RFCs. Please check the source & below table for details.

# Installation:

Run `configure`, e.g.

```shell
./configure --prefix=/usr/local --sysconfdir=/etc --mandir=/usr/share/man
```

See configure --help for additional command line arguments.

Run 'make' and 'make install'.  On BSD, you may need to use 'gmake'.

# Configuration:

See `INTRO.html`, `radvd.conf(8)` and `radvd.conf.example`.

# Frequently Asked Questions:

Setting up radvd is very simple, so the most frequently asked
questions have been about what radvd _doesn't_ do...

 1. **How do I set up the router running radvd to automatically
    configure an address from the prefix advertised in Route
    Advertisements from upstream?**
    
    **You don't.**  By the specification, routers ignore RAs.
    You'll probably need to use manual configuration.  But you
    can't use the same prefix on two links in any case unless you
    use something like proxy-ND (draft-ietf-ipv6-ndproxy-04.txt).
    You may need to re-think your topology; prefix delegation
    (e.g., manually or with RFC3633) may help.
    

 2. **How do I set up the router running radvd to automatically
    configure the interfaces to use an EUI64-based address?**

    **You don't.**  The design philosophy of radvd is that it's
    not the _router's_ configuration tool, but a route advertising
    daemon.  You'll need to set up all the addresses, routes, etc.
    yourself.  These tasks are something that system initscripts
    could possibly do instead.

 3. **I have a dynamic /48 prefix.  How do I set up radvd to:  
    a) set up interface addresses and routes on downstream  
       interfaces, and  
    b) advertise /64 prefixes from the /48 on downstream interfaces?**

    For a), this isn't supported.  
    For b), radvd includes special
    support for 6to4 upstream interface but assumes that the interface
    addresses/routes are set up manually.  This should probably
    be done in the initscripts or manually. (Though if someone were
    to send a patch for b), it might be incorporated.)  

 5. **How do I set up radvd to do either unicast or multicast routing?**

    **You don't.**  Radvd is not a routing or forwarding daemon.
    You need to set any appropriate routing/forwarding first,
    and then radvd to only advertise the prefixes to hosts as
    appropriate.

# RFC Implementation key
As of 2026/07/04.

Implementation Legend:
 - COMPLETED: Every part of the RFC is implemented
 - MOST: The majority of the RFC is implemented, see exceptions list
 - PARTIAL: minimal or only parts of the RFC are implemented, see details
 - WONTFIX: The RFC will not be implemented, see details
 - NONE: The RFC is NOT implemented, but could be
 - OBSOLETE: The RFC has been supecede by a newer RFC.
 - DUMP: only supported by `radvddump`
 - UNKNOWN: Implementation status unknown.

## RFCs with RA options or modifying RA behaviors
| RFC | Status | Title | Note Summary |
|---|---|---|---|
| RFC2461 | OBSOLETE | Neighbor Discovery for IP Version 6 (IPv6) | Obsoleted by RFC 4861 |
| RFC3775 | OBSOLETE | Mobility Support in IPv6 | Obsoleted by RFC 6275 |
| RFC3971 | NONE | SEcure Neighbor Discovery (SEND) | |
| RFC4068 | UNKNOWN | Fast Handovers for Mobile IPv6 | |
| RFC4191 | DUMP | Default Router Preferences and More-Specific Routes | |
| RFC4389 | NONE | Neighbor Discovery Proxies (ND Proxy) | |
| RFC4861 | MOST | Neighbor Discovery for IP version 6 (IPv6) | |
| RFC4862 | | IPv6 Stateless Address Autoconfiguration (SLAAC) | |
| RFC5006 | OBSOLETE | IPv6 Router Advertisement Option for DNS Configuration | Obsoleted by RFC 6106, RFC 8106 |
| RFC5175 | COMPLETE | IPv6 Router Advertisement Flags Option | |
| RFC5380 | UNKNOWN | Hierarchical Mobile IPv6 (HMIPv6) Mobility Management | |
| RFC6104 | | Rogue IPv6 Router Advertisement Problem Statement | |
| RFC6105 | | IPv6 Router Advertisement Guard | |
| RFC6106 | OBSOLETE | IPv6 Router Advertisement Options for DNS Configuration | Obsoleted by RFC 8106 | |
| RFC6275 | PARTIAL | Mobility Support in IPv6 | |
| RFC6494 | NONE | Certificate Profile and Certificate Management for SEcure Neighbor Discovery (SEND) | |
| RFC6495 | NONE | Subject Key Identifier (SKI) SEcure Neighbor Discovery (SEND) Name Type Fields | |
| RFC6496 | NONE | Secure Proxy ND Support for SEND | |
| RFC6775 | PARTIAL | Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs) | |
| RFC6980 | | Security Implications of IPv6 Fragmentation with IPv6 Neighbor Discovery | |
| RFC7113 | | Implementation Advice for IPv6 Router Advertisement Guard (RA-Guard) | |
| RFC7222 | MOST | IPv6 Node Requirements | Obsoleted by RFC 8504 |
| RFC7400 | NONE | 6LoWPAN-GHC: Generic Header Compression for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs) | |
| RFC7559 | | Packet-Loss Resiliency for Router Solicitations | |
| RFC7710 | OBSOLETE | Captive-Portal Identification Using DHCP or Router Advertisements | Obsoleted by RFC 8910 |
| RFC7772 | MOST | Reducing Energy Consumption of Router Advertisements | |
| RFC8028 | | First-Hop Routing Configuration in IPv6 | |
| RFC8106 | MOST | IPv6 Router Advertisement Options for DNS Configuration | |
| RFC8273 | | Unique IPv6 Prefix per Host | |
| RFC8504 | PARTIAL | IPv6 Node Requirements | |
| RFC8781 | MOST | Discovering PREF64 in Router Advertisements | |
| RFC8910 | PARTIAL | Captive-Portal Identification in DHCP and Router Advertisements | |
| RFC8981 | | Privacy Extensions for Stateless Address Autoconfiguration in IPv6 | |
| RFC9099 | | Operational Security Considerations for IPv6 Networks | |

## Other RFCs
| RFC | Status | Title | Note Summary |
|---|---|---|---|
| RFC6273 | UNKNOWN | The Secure Neighbor Discovery (SEND) Hash Threat Analysis | |
| RFC4286 | WONTFIX | Multicast Router Discovery | Unrelated protocol |

## Notes: RFC3971, RFC6496 & related SEND
None of the Secure Neighbor Discovery (SEND) options are implemented.
This would be a substantial future project, and is a good candidate for
corporate support: directly contributed code, funded and/or sponsored.

## Notes: RFC4861
The following parts of RFC4861 are not implemented:
  - Support for few protocol constants defined in RFC 4861 Sec 10 is missing.
  - section 6.2.5: when AdvSendAdvertisements changes to FALSE, radvd should send
    a final RA with zero Router Lifetime (radvd presently sends during shutdown).
    (SHOULD)
  - section 6.2.8: if the link-local address of the router changes, it should
    multicast a few RAs from the old address with zero router lifetime, and a
    few from the new address. (SHOULD).

## Notes: RFC7222
The following parts of RFC7222 are not implemented:
  - Section 5.1.3: Networks that serve battery-powered devices SHOULD NOT send
    multicast RAs too frequently (see Section 4) unless the information in the RA
    packet has substantially changed.  If there is a desire to ensure that hosts
    pick up configuration changes quickly, those networks MAY send frequent
    Router Advertisements for a limited period of time (e.g., not more than one
    minute) immediately after a configuration change.
