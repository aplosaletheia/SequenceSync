# SequenceSync

# Zero-Dep Audio Fingerprinter

A CLI tool that acts as an acoustic fingerprinting engine (similar to Shazam). It ingests `.wav` files, manually breaks them down into frequency bands using a custom Short-Time Fourier Transform, hashes the dominant frequencies into acoustic "constellations," and matches them against an embedded custom binary database.

**All built in a single C file using nothing but `libc` and standard POSIX headers.**

## Track F (Open / Wildcard) Justification
Building an audio recognition engine normally requires a massive dependency tree:
1. `libsndfile` or `ffmpeg` to parse audio containers.
2. `FFTW` (Fastest Fourier Transform in the West) to perform the math-heavy frequency domain conversions.
3. `SQLite` or `Redis` to store and query the generated acoustic hashes.

This project implements all three layers from scratch using only standard C arrays, `<math.h>`, and binary `FILE*` operations.

## How to Run
1. Build the project:
  command - gcc main.c -o SequenceSync
2. Run the exe
  command - .\SequenceSync.exe

requirements -
1. wav file in folder.
