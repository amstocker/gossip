#!/bin/bash

((./gossip) & echo $! > __test_daemon.pid &)
