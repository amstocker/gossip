#!/usr/bin/env python

import sys
import socket


PATH = "/tmp/gossip.sock"

server = socket.socket (socket.AF_UNIX, socket.SOCK_STREAM)


try:
    server.connect (PATH)
    server.send ("hello api socket!")
except:
    print "[PYTHON]", "socket connection refused"
    sys.exit(0)

try:
    data = server.recv(4096)
    print "[PYTHON]", "got response from server: \"{}\"".format(data)
except:
    print "[PYTHON]", "socket recv failed"
    sys.exit(0)
