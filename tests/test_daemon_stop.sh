#!/bin/bash

kill -9 `cat __test_daemon.pid` || 
  if pgrep gossip-server; then killall gossip-server; fi

rm __test_daemon.pid
