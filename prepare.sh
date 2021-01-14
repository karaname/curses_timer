#!/bin/bash
#
# Prepare env for 'curses_timer' program
#

gcc -g3 main.c -o curses_timer -lncurses
if test ! -e /opt/laser.wav; then
  sudo cp sounds/laser.wav /opt
fi
