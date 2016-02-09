#!/usr/bin/env python

import socket


PATH = "/tmp/gossip.sock"

server = socket.socket (socket.AF_UNIX, socket.SOCK_STREAM)


try:
    server.connect (PATH)
    server.send ("hello, server!")
except:
    print "socket connection refused"
finally:
    server.close()
