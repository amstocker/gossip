#!/bin/bash

kill -9 `cat __test_daemon.pid` || 
  if pgrep gossip; then killall gossip; fi

rm __test_daemon.pid
