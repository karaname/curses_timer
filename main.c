/*
Timer with signal, which should be in /opt/laser.wav
Input values: time format 00:00:00
Ncurses library used
*/

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#define _name "curses_timer"

static char ctime_buf[16];

static void
get_currenttime()
{
  struct timeval tval;
  struct tm *ptm;

  gettimeofday(&tval, NULL);
  ptm = localtime(&tval.tv_sec);
  strftime(ctime_buf, sizeof(ctime_buf), "%H:%M:%S", ptm);
}

int main(int argc, char *argv[])
{
  char timer_buf[15];
  int row, col, ch, status;
  int min = 0;
  int sec = 0;
  int hour = 0;

  if (argc > 1) {
    if (strlen(argv[1]) != 8) {
      fprintf(stderr, "%s: Timer parameter should be in format '00:00:00'\n", _name);
      exit(EXIT_FAILURE);
    }

    initscr();
    noecho();
    curs_set(0);
    getmaxyx(stdscr, row, col);
    nodelay(stdscr, TRUE); // getch not stop program
    keypad(stdscr, TRUE);  // F1..2..3
    get_currenttime();

    while (1) {
      if ((ch = getch()) == -1) {
          if (strcmp(timer_buf, argv[1]) != 0) {
            sec++;
            if (sec == 60) {
              sec = 0;
              min++;
              if (min == 60) {
                min = 0;
                hour++;
                if (hour == 24) {
                  hour = 0;
                }
              }
            }

            sprintf(timer_buf, "%02d:%02d:%02d", hour, min, sec);
            mvprintw(row / 2, (col - strlen(timer_buf)) / 2, "%s", timer_buf);
            mvprintw(0, 0, "Key press S - stop timer");
            mvprintw(1, 0, "Key press C - continue timer");
            mvprintw(2, 0, "Key press F10 - exit");
            mvprintw(4, 0, "Start time - %s", ctime_buf);
            mvprintw(5, 0, "Timer time - %s", argv[1]);
            refresh();
            sleep(1);
          } else {
            pid_t aplay_child;
            switch (aplay_child = fork()) {
              case -1:
                exit(EXIT_FAILURE);
              case 0:
                close(2);
                execl("/usr/bin/aplay", "/usr/bin/aplay", "/opt/curve.wav", NULL);
              default:
                wait(&status);
            }

            break;
          }
      } else if (ch == 's') {
        while (ch = getch()) {
          if (ch == 'c') break;
        }
      } else if (ch == 274) {
        break;
        exit(EXIT_SUCCESS);
      }
    }

    endwin();
  } else {
    fprintf(stderr, "%s: Missing timer parameter in format '00:00:00'\n", _name);
    exit(EXIT_FAILURE);
  }

  if (WIFEXITED(status) > 0) {
    if (WEXITSTATUS(status) != 0) {
      fprintf(stderr, "%s: execl error, 'aplay' can't play /opt/laser.wav file\n", _name);
      exit(EXIT_FAILURE);
    }
  }

  printf("Start time - %s\n", ctime_buf);
  printf("Timer done - %s\n", timer_buf);
  return 0;
}
