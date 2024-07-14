#define MINIMP3_IMPLEMENTATION
#include "../minimp3_ex.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main() {
    unsigned char buf[4096];
    mp3dec_ex_t dec;

#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
    while (__AFL_LOOP(1000))
#endif
    {
        int nbuf = fread(buf, 1, sizeof(buf), stdin);
        if (nbuf > 0) {
            if (mp3dec_ex_open_buf(&dec, buf, nbuf, MP3D_SEEK_TO_SAMPLE)) {
                int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
                size_t samples = mp3dec_ex_read(&dec, pcm, MINIMP3_MAX_SAMPLES_PER_FRAME);
                mp3dec_ex_close(&dec);
            }
        }
    }

    return 0;
}
