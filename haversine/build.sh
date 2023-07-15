#!/bin/sh

cc -Wdouble-promotion -o generator json_generator.c
cc -o parser json_parser.c
