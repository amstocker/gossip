#!/bin/bash

((./gossip-server) & echo $! > __test_daemon.pid &)
