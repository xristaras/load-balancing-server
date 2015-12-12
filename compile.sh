#!/bin/bash

gcc main.c server.c load_balancers.c -lcurl -lpthread -ggdb
