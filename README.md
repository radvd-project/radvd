# radvd

radvd is the router advertisement daemon for IPv6. It listens to router
solicitations and sends router advertisements as described in
"Neighbor Discovery for IP Version 6 (IPv6)" (RFC 4861).

With these advertisements hosts can automatically configure their
addresses and some other parameters. They also can choose a default router
based on these advertisements.

It also implements "Neighbor Discovery Optimization for IPv6 over Low-Power
Wireless Personal Area Networks (6LoWPANs)" (RFC6775) & many other related IPv6
RFCs. Please check the source for details.

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

 1. How do I set up the router running radvd to automatically
    configure an address from the prefix advertised in Route
    Advertisements from upstream?

    -- You don't.  By the specification, routers ignore RAs.
    You'll probably need to use manual configuration.  But you
    can't use the same prefix on two links in any case unless you
    use something like proxy-ND (draft-ietf-ipv6-ndproxy-04.txt).
    You may need to re-think your topology; prefix delegation
    (e.g., manually or with RFC3633) may help.

 2. How do I set up the router running radvd to automatically
    configure the interfaces to use an EUI64-based address?

    -- You don't.  The design philosophy of radvd is that it's
    not the _router's_ configuration tool, but a route advertising
    daemon.  You'll need to set up all the addresses, routes, etc.
    yourself.  These tasks are something that system initscripts
    could possibly do instead.

 3. I have a dynamic /48 prefix.  How do I set up radvd to:
    a) set up interface addresses and routes on downstream
       interfaces, and
    b) advertise /64 prefixes from the /48 on downstream interfaces?

    -- For a), this isn't supported. For b), radvd includes special
    support for 6to4 upstream interface but assumes that the interface
    addresses/routes are set up manually.  This should probably
    be done in the initscripts or manually. (Though if someone were
    to send a patch for b), it might be incorporated.)  

 4. How do I set up radvd to do either unicast or multicast routing?

    -- You don't.  Radvd is not a routing or forwarding daemon.
    You need to set any appropriate routing/forwarding first,
    and then radvd to only advertise the prefixes to hosts as
    appropriate.
