/*
Timer with signal, play 'sounds/curve.wav'
Input values: time format 00:00:00
*/

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <setjmp.h>
#define _name "curses_timer"

char ctime_buf[16];
sigjmp_buf env_buf;

void resize_handler()
{
  siglongjmp(env_buf, 5);
}

/* check 80x24 terminal size */
void term_size_check()
{
  if (LINES < 24 || COLS < 80) {
    endwin();
    fprintf(stderr, "%s: Please, use the 'normal' terminal \
size - 80 columns by 24 lines\n", _name);
    exit(0);
  }
}

void get_currenttime()
{
  struct timeval tval;
  struct tm *ptm;

  gettimeofday(&tval, NULL);
  ptm = localtime(&tval.tv_sec);
  strftime(ctime_buf, sizeof(ctime_buf), "%H:%M:%S", ptm);
}

int main(int argc, char *argv[])
{
  signal(SIGWINCH, resize_handler);

  char timer_buf[15];
  int ch, ach, status;
  int min = 0;
  int sec = 0;
  int hour = 0;

  if (argc > 1) {
    if (strlen(argv[1]) != 8) {
      fprintf(stderr, "%s: Timer parameter should be in format '00:00:00'\n", _name);
      exit(1);
    }

    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE); // getch not stop program
    keypad(stdscr, TRUE);  // F1..2..3
    get_currenttime();

    while (1) {
      if (sigsetjmp(env_buf, 5)) {
        endwin();
        clear();
      }
      term_size_check();

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
          mvprintw(LINES / 2, (COLS - strlen(timer_buf)) / 2, "%s", timer_buf);
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
              execl("/usr/bin/aplay", "/usr/bin/aplay", "sounds/curve.wav", NULL);
            default:
              wait(&status);
          }

          break;
        }
      } else if (ch == 's') {
        while (ach = getch()) {
          if (ach == 'c') {
            break;
          }
        }
      } else if (ch == KEY_F(10)) {
        break;
        exit(0);
      }
    }

    endwin();
  } else {
    fprintf(stderr, "%s: Missing timer parameter in format '00:00:00'\n", _name);
    exit(1);
  }

  if (WIFEXITED(status) > 0) {
    if (WEXITSTATUS(status) != 0) {
      fprintf(stderr, "%s: execl error, 'aplay' can't play sounds/curve.wav file\n", _name);
      exit(1);
    }
  }

  printf("Start timer - %s\n", ctime_buf);
  printf("Timer time  - %s\n", timer_buf);
  get_currenttime();
  printf("End timer   - %s\n", ctime_buf);

  return 0;
}
