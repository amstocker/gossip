#!/bin/bash

kill -9 `cat __test_daemon.pid`
rm __test_daemon.pid
