#!/bin/sh

cc -Wdouble-promotion -o generator json_generator.c
cc -o parser json_parser.c

c++ -std=c++11 -O3 -o test test.cpp
c++ -std=c++11 -O3 -o parser my_listing78.cpp
