#!/usr/bin/env python

import sys
import socket


PATH = "/tmp/gossip.sock"

server = socket.socket (socket.AF_UNIX, socket.SOCK_STREAM)


try:
    server.connect (PATH)
    for i in xrange(10):
        server.send ("{} ".format(i))
except:
    print "socket connection refused"
    sys.exit(0)

try:
    data = server.recv(4096)
    print "got response from server:", data
except:
    print "socket recv failed"
    sys.exit(0)
