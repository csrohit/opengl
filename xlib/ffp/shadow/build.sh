#!/bin/bash


gcc -c -o xwindow.o -I/usr/include ./xwindow.c
gcc -o xwindow -L/usr/lib/x86_64-linux-gnu xwindow.o -lX11
