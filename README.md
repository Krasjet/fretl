fretl
=====

       9      8      7      6      5      4      3      2      1      0
    +--.-------------.-------------.-------------.------------------+
    |--C#--|------|--B---|------|--A---|--G#--|------|--F#--|------|| E
    |--G#--|------|--F#--|------|--E---|------|--D---|--C#--|------|| B
    |--F#--|------|--E---|------|--D---|--C#--|------|--B---|------|| A
    |--C#--|------|--B---|------|--A---|--G#--|------|--F#--|------|| E
    |--F#--|------|--E---|------|--D---|--C#--|------|--B---|------|| A
    |--B---|------|--A---|--G#--|------|--F#--|------|--E---|------|| D
    +--.-------------.-------------.-------------.------------------+

fretl is a command-line fretboard specifically designed for mirrored (a.k.a.
left-handed) guitars.

Unlike most fretboard diagrams you can found online, `fretl`

1. Supports mirrored guitar, since some misfortuned ones learned to reason
   about notes on guitar the opposite way.
2. Has first-class support for alternative tunings
3. Has no assumption about how many strings a guitar should have
4. Uses unambiguous representation of notes, intervals, and scales, so it can
   distinguish enharmonic equivalent notes such as C# and Db.

For example, you could print the notes in A major scale in DAEAC#E tuning up to
7th fret by

    $ fretl -f7 -rA DAEAC#E < scales/major
       7      6      5      4      3      2      1      0
    +--.-------------.-------------.------------------+
    |--B---|------|--A---|--G#--|------|--F#--|------|| E
    |--G#--|------|--F#--|------|--E---|------|--D---|| C#
    |--E---|------|--D---|--C#--|------|--B---|------|| A
    |--B---|------|--A---|--G#--|------|--F#--|------|| E
    |--E---|------|--D---|--C#--|------|--B---|------|| A
    |--A---|--G#--|------|--F#--|------|--E---|------|| D
    +--.-------------.-------------.------------------+

or alternatively display in scale degrees.

    $ fretl -d -f7 -rA DAEAC#E < scales/major
       7      6      5      4      3      2      1      0
    +--.-------------.-------------.------------------+
    |--2---|------|--1---|--7---|------|--6---|------|| 5
    |--7---|------|--6---|------|--5---|------|--4---|| 3
    |--5---|------|--4---|--3---|------|--2---|------|| 1
    |--2---|------|--1---|--7---|------|--6---|------|| 5
    |--5---|------|--4---|--3---|------|--2---|------|| 1
    |--1---|--7---|------|--6---|------|--5---|------|| 4
    +--.-------------.-------------.------------------+

Capo is also supported via the `-c` flag.

    $./fretl -c3 -f7 -rC DAEABE < scales/major
       7      6      5      4      3      2      1      0
    +---------.-------------.-------------.-----------+
    |--D---|------|--C---|--B---|------|--A---|------|| G
    |--A---|------|--G---|------|--F---|--E---|------|| D
    |--G---|------|--F---|--E---|------|--D---|------|| C
    |--D---|------|--C---|--B---|------|--A---|------|| G
    |--G---|------|--F---|--E---|------|--D---|------|| C
    |--C---|--B---|------|--A---|------|--G---|------|| F
    +---------.-------------.-------------.-----------+

You could also pass in other scales as well.

    $ fretl -f7 -rD CFCFAC < scales/minor
       7      6      5      4      3      2      1      0
    +--.-------------.-------------.------------------+
    |--G---|------|--F---|--E---|------|--D---|------|| C
    |--E---|------|--D---|------|--C---|------|--Bb--|| A
    |--C---|------|--Bb--|--A---|------|--G---|------|| F
    |--G---|------|--F---|--E---|------|--D---|------|| C
    |--C---|------|--Bb--|--A---|------|--G---|------|| F
    |--G---|------|--F---|--E---|------|--D---|------|| C
    +--.-------------.-------------.------------------+

Note that you don't need to specify which accidental to use. This is taken care
of automatically.

Build
-----

fretl is written in C99. Make sure you have a C99 compatible C compiler to
build this program.

Since I separated the parsing and processing of music notes and intervals into
separate libraries, you first need to update submodules by

    $ git submodule update --init --recursive

Then use `make` to compile and install it to system:

    $ make
    # make install

You could also use the `PREFIX` option to install for current user only:

    $ make PREFIX=~/.local install

Further reading
---------------

See

    $ man 1 fretl

for the explanation of each option.
