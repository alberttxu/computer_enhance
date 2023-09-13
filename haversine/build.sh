#!/bin/sh

cc -Wdouble-promotion -o generator json_generator.c
cc -o parser json_parser.c

c++ -std=c++11 -O3 -o test test.cpp
c++ -std=c++11 -O3 -o parser_profileON my_listing79_parser_profileON.cpp
c++ -std=c++11 -O3 -o parser_profileOFF my_listing79_parser_profileOFF.cpp
