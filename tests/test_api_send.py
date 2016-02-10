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
