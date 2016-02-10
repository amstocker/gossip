#!/usr/bin/env python

import socket


PATH = "/tmp/gossip.sock"

server = socket.socket (socket.AF_UNIX, socket.SOCK_STREAM)
server.bind(PATH)

while True:
    data = socket.recv(4096)
    if not data:
        break
    print data

server.close()
