#!/bin/bash
#
# Prepare env for 'curses_timer' program
#

gcc -g3 main.c -o curses_timer -lncurses
if test ! -e /opt/curve.wav; then
  sudo cp sounds/curve.wav /opt
fi
