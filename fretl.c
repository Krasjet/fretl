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
  STRING_MAX = 20,
  INTERVAL_MAX_LEN = 10,
  NOTE_MAX_LEN = 7,
};

enum note_rep {
  REP_PITCH_CLASS,
  REP_SCALE_DEGREE
};

/* get fret mark from 1 based index */
char
fret_mark(int fret)
{
  static char marks[12] =  {
    [0] = '-',
    [1] = '-',
    [2] = '.',
    [3] = '-',
    [4] = '.',
    [5] = '-',
    [6] = '.',
    [7] = '-',
    [8] = '.',
    [9] = '-',
    [10] = '-',
    [11] = ':'
  };
  return marks[(fret - 1) % 12];
}

static struct mint scale_intv[SCALE_MAX];
static struct mnote scale_notes[SCALE_MAX];
static int scale_midi[SCALE_MAX];
static int scale_st[SCALE_MAX];
static int pc_count;
static int frets = 12;
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
  int i = frets;

  printf("+--%c---", fret_mark(i--));

  for (; i > 0; --i) {
    printf("---%c---", fret_mark(i));
  }

  printf("-+\n");
}

void
print_fretnum(void){
  char num[4];
  int i;

  for (i = frets; i > 0; --i) {
    sprintf(num, "%d", i);
    fputc(' ', stdout);
    print_center(num, ' ', 6);
  }

  printf("   0\n");
}

/* convert scale degree to string representation */
void
deg_to_rep(int degree, char * out)
{
  switch (rep) {
    case REP_SCALE_DEGREE:
      mint_to_sdegree(scale_intv[degree], out);
      break;
    case REP_PITCH_CLASS:
      mnote_pc_toascii(scale_notes[degree].pc, out);
      break;
  }
}

void
print_string(struct mnote base){
  int midi = mnote_tomidi(base);
  int i, j;
  char note[NOTE_MAX_LEN];

  for (i = frets; i > 0; --i) {
    for (j = 0; j < pc_count; j++){
      if (midi_same_pc(midi + i, scale_midi[j])) {
        deg_to_rep(j, note);
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
    if (midi_same_pc(midi, scale_midi[j])) {
      deg_to_rep(j, note);
      printf("|| %s\n", note);
      break;
    }
  }
  if (j == pc_count) {
    /* no match */
    puts("||");
  }
}

void
print_fretboard(const char * tuning)
{
  struct mnote strings[STRING_MAX];
  int str_count = 0;
  print_fretnum();
  print_border();

  /* we need to reverse the order of strings */
  while((tuning = mnote_parse(tuning, &strings[str_count]))) {
    str_count++;
  }
  for (str_count--; str_count >= 0; str_count--) {
    print_string(strings[str_count]);
  }
  print_border();
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
	fprintf(stdout, "usage: %s [-r root] [-f fret] [-dh] [tuning]\n", name);
	fprintf(stdout, "       %s [-r tonic] [-f fret] [-dh] [tuning] < scale\n", name);
	fprintf(stdout, "       %s [-r root] [-f fret] [-dh] [tuning] < chord\n", name);
}

int
main(int argc, char *argv[])
{
  char intv[INTERVAL_MAX_LEN];
  char *tuning;
  int i;

  int c;
	while ((c = getopt(argc, argv, "hr:df:")) != -1) {
    switch (c) {
      case 'h':
        usage(argv[0]);
        return 0;
      case 'r':
        if (!mnote_parse(optarg, &root) || abs(root.pc.accidental) > 1) {
          die("error: invalid root note %s\n", intv);
        }
        break;
      case 'f':
        frets = atoi(optarg);
        if (frets < 1) {
          die("error: frets must be at least 1\n");
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

  /* defaults to DAEABE when no tuning given */
  if (optind < argc) {
    tuning = argv[optind];
  } else {
    tuning = "DAEABE";
  }

  print_fretboard(tuning);

  return 0;
}
