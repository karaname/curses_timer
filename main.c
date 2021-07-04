#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <setjmp.h>
#include <error.h>

sigjmp_buf env_buf;
void sigwinch_handler()
{
  siglongjmp(env_buf, 1);
}

void error_wrap(const char *s)
{
  endwin();
  error(0, 0, s);
  exit(EXIT_FAILURE);
}

void get_curtime(char *t, size_t max)
{
  struct timeval tval;
  struct tm *ptm;

  gettimeofday(&tval, NULL);
  ptm = localtime(&tval.tv_sec);
  strftime(t, max, "%H:%M:%S", ptm);
}

int main(int argc, char *argv[])
{
  signal(SIGWINCH, sigwinch_handler);
  signal(SIGINT, SIG_IGN);

  char str_time[16], cur_time[16], end_time[16];
  int ch, cch, status, min = 0, sec = 0, hour = 0;

  if (argc > 1) {
    if (strlen(argv[1]) != 8) {
      error(0, 0, "Timer parameter should be in format '00:00:00'");
      exit(EXIT_FAILURE);
    }

    if (sigsetjmp(env_buf, 1)) {
      endwin();
      clear();
    }

    if (!initscr()) {
      error(0, 0, "Error initialising ncurses");
      exit(EXIT_FAILURE);
    }

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    get_curtime(cur_time, sizeof(cur_time));

    while ((ch = getch())) {
      switch (ch) {
        case KEY_F(10):
          endwin();
          exit(EXIT_SUCCESS);
        case 's':
          while ((cch = getch()))
            if (cch == 'c')
              break;
      }

      if (strcmp(str_time, argv[1]) != 0) {
        if (sec == 60) {
          sec = 0;
          min++;
          if (min == 60) {
            min = 0;
            hour++;
            if (hour == 24)
              hour = 0;
          }
        }

        sprintf(str_time, "%02d:%02d:%02d", hour, min, sec);
        mvprintw(LINES / 2, (COLS - strlen(str_time)) / 2, "%s", str_time);
        mvprintw(0, 0, "Key press S - stop timer");
        mvprintw(1, 0, "Key press C - continue timer");
        mvprintw(2, 0, "Key press F10 - exit");
        mvprintw(4, 0, "Start time - %s", cur_time);
        mvprintw(5, 0, "Timer time - %s", argv[1]);
        refresh();
        sleep(1);
        sec++;
      } else {
        get_curtime(end_time, sizeof(end_time));
        pid_t aplay_child;
        switch (aplay_child = fork()) {
          case -1:
            error_wrap("fork() return -1");
            break;
          case 0:
            close(2);
            execlp("aplay", "aplay", "sounds/curve.wav", NULL);
            break;
          default:
            wait(&status);
            if (WIFEXITED(status) > 0) {
              if (WEXITSTATUS(status) != 0)
                error_wrap("'aplay' can't play sounds/curve.wav");
            }
        }
        break;
      }
    }
    endwin();
    printf("Start timer - %s\n", cur_time);
    printf("Timer time  - %s\n", argv[1]);
    printf("End timer   - %s\n", end_time);
  } else {
    error(0, 0, "Missing timer parameter in format '00:00:00'");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
