#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include "mnote/mint.h"
#include "mnote/mnote.h"
#include "mnote/transpose.h"

enum {
  SCALE_MAX = 12,
  FRET_MAX = 13,
  STRING_MAX = 20,
  INTERVAL_MAX_LEN = 10,
  NOTE_MAX_LEN = 7,
};

enum note_rep {
  REP_PITCH_CLASS,
  REP_SCALE_DEGREE
};

static char fret_mark[FRET_MAX] =  {
  [0] = '-',
  [1] = '-',
  [2] = '-',
  [3] = '.',
  [4] = '-',
  [5] = '.',
  [6] = '-',
  [7] = '.',
  [8] = '-',
  [9] = '.',
  [10] = '-',
  [11] = '-',
  [12] = ':'
};

static struct mint scale_intv[SCALE_MAX];
static struct mnote scale_notes[SCALE_MAX];
static int scale_midi[SCALE_MAX];
static int scale_st[SCALE_MAX];
static int pc_count;
static enum note_rep rep = REP_PITCH_CLASS;

static struct mnote root = {
  .octave = 0,
  .pc = {
    .letter = MNOTE_A,
    .accidental = 0
  }
};

static struct mint major_scale[] = {
  { .size = 0, .qoffset = 0 },
  { .size = 1, .qoffset = 0 },
  { .size = 2, .qoffset = 0 },
  { .size = 3, .qoffset = 0 },
  { .size = 4, .qoffset = 0 },
  { .size = 5, .qoffset = 0 },
  { .size = 6, .qoffset = 0 }
};

int
abs(int x)
{
  return x > 0 ? x : -x;
}

int
midi_same_pc(int x, int y)
{
  return (x - y) % 12 == 0;
}

void
die(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	exit(1);
}


/*
 * since our scales is represented using scale degrees, we could easily convert
 * interval, a relative concept, to scale degree (relative to major), an
 * absolute concept.
 */
void
mint_to_sdegree(struct mint interval, char *out)
{
  int offset = interval.qoffset;

  if (offset > 0) {
    while(offset-- > 0){
      *out++ = '#';
    }
  } else if (offset < 0) {
    while(offset++ < 0){
      *out++ = 'b';
    }
  }
  sprintf(out, "%d", interval.size + 1);
}

void
print_center(const char *s, char pad, int space) {
  int len = strlen(s);
  int padlen = (space - len) / 2;
  int i;

  for (i = 0; i < padlen; ++i) {
    fputc(pad, stdout);
  }
  fputs(s, stdout);
  for (i = 0; i < space - padlen - len; ++i) {
    fputc(pad, stdout);
  }
}

void
print_border(void){
  int i = FRET_MAX - 1;

  printf("+--%c---", fret_mark[i--]);

  for (; i >= 1; --i) {
    printf("---%c---", fret_mark[i]);
  }

  printf("-+\n");
}

void
print_fretnum(void){
  char num[3];
  int i = FRET_MAX - 1;

  for (; i >= 1; --i) {
    sprintf(num, "%d", i);
    fputc(' ', stdout);
    print_center(num, ' ', 6);
  }

  printf("   0\n");
}

void
print_string(struct mnote base){
  int midi = mnote_tomidi(base);
  int i = FRET_MAX - 1, j;
  char note[NOTE_MAX_LEN];

  for (; i >= 1; --i) {
    for (j = 0; j < pc_count; j++){
      if (midi_same_pc(midi+i, scale_midi[j])) {
        switch (rep) {
          case REP_SCALE_DEGREE:
            mint_to_sdegree(scale_intv[j], note);
            break;
          case REP_PITCH_CLASS:
            mnote_pc_toascii(scale_notes[j].pc, note);
            break;
        }
        fputc('|', stdout);
        print_center(note, '-', 6);
        break;
      }
    }
    if (j == pc_count) {
      /* no match */
      fputs("|------", stdout);
    }
  }

  for (j = 0; j < pc_count; j++){
    if (midi_same_pc(midi+i, scale_midi[j])) {
        switch (rep) {
          case REP_SCALE_DEGREE:
            mint_to_sdegree(scale_intv[j], note);
            break;
          case REP_PITCH_CLASS:
            mnote_pc_toascii(scale_notes[j].pc, note);
            break;
        }
      printf("|| %s\n", note);
      break;
    }
  }
  if (j == pc_count) {
    /* no match */
    puts("||");
  }
}

/* returns 0 when failed */
int
readw(char *out, size_t bufsiz)
{
  int ch;
  size_t count = 0;

  while ((ch = fgetc(stdin)) != EOF) {
    if (count >= bufsiz) {
      return 0;
    }
    if (isspace(ch)) {
      if (count == 0) {
        continue;
      } else {
        *out = '\0';
        return 1;
      }
    }
    *out++ = ch;
    count++;
  }

  return 0;
}

static void
usage(const char *name) {
	fprintf(stdout, "usage: %s -r ROOT [tuning]\n", name);
}

int
main(int argc, char *argv[])
{
  char intv[INTERVAL_MAX_LEN];
  struct mnote strings[STRING_MAX];
  int str_count = 0;
  char * s;
  int i;

  int c;
	while ((c = getopt(argc, argv, "hr:d")) != -1) {
    switch (c) {
      case 'h':
        usage(argv[0]);
        return 0;
      case 'r':
        if (!mnote_parse(optarg, &root) || abs(root.pc.accidental) > 1) {
          die("error: invalid root note %s\n", intv);
        }
        break;
      case 'd':
        rep = REP_SCALE_DEGREE;
        break;
      default:
        usage(argv[0]);
        return 1;
    }
	}

  if (isatty(fileno(stdin))) {
    /* use major scale when no input given */
    memcpy(scale_intv, major_scale, 7 * sizeof(struct mint));
    pc_count = 7;
  } else {
    while (readw(intv, INTERVAL_MAX_LEN) && pc_count < SCALE_MAX) {
      scale_intv[pc_count] = mint_parse(intv);

      if (scale_intv[pc_count].size < 0) {
        die("error: invalid interval size %s\n", intv);
      }
      pc_count++;
    }
  }

  /* build metadata for matching */
  for (i = 0; i < pc_count; i++) {
    scale_st[i] = mint_to_st(scale_intv[i]);
    scale_notes[i] = transpose_mnote(root, scale_intv[i], DIR_UPWARD);
    scale_midi[i] = mnote_tomidi(scale_notes[i]);
  }

  /* parsing string tuning one by one */
  if (optind < argc) {
    s = argv[optind];
  } else {
    s = "DAEABE";
  }
  print_fretnum();
  print_border();

  /* we need to reverse the order of strings */
  while((s = mnote_parse(s, &strings[str_count]))) {
    if (s)
      str_count++;
  }
  for (str_count--; str_count >= 0; str_count--) {
    print_string(strings[str_count]);
  }
  print_border();

  return 0;
}
