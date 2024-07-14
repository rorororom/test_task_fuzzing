#!/bin/bash

CFLAGS="-fsanitize=address,undefined -fno-sanitize-recover=address,undefined -O2"
AFL_COMPILER="../../AFLplusplus/afl-clang-fast"

# Компиляция fuzz_dec_decode_frame
$AFL_COMPILER $CFLAGS -o fuzz_dec_decode_frame fuzz_dec_decode_frame.c

# Компиляция fuzz_mp3dec_ex_read
$AFL_COMPILER $CFLAGS -o fuzz_mp3dec_ex_read fuzz_mp3dec_ex_read.c

 
