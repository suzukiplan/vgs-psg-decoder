#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgsspu.h"
#define SAMPLING_RATE 44100
#define BIT_RATE 8
#define CHANNEL 1

#define PSG_CHNUM 3

static int P[89] = {
    0,   1604, 1514, 1429, 1348, 1273, 1201, 1134, 1070, 1010, 954, 900, 849, 802, 757, 714, 674, 636, 601, 567, 535, 505, 477,
    450, 425,  401,  378,  357,  337,  318,  300,  283,  268,  253, 238, 225, 212, 200, 189, 179, 169, 159, 150, 142, 134, 126,
    119, 113,  106,  100,  95,   89,   84,   80,   75,   71,   67,  63,  60,  56,  53,  50,  47,  45,  42,  40,  38,  35,  33,
    32,  30,   28,   27,   25,   24,   22,   21,   20,   19,   18,  17,  16,  15,  14,  13,  13,  12,  11,  11,
};

struct VgsPsgChannel {
    int keyon; /* key-on */
    int scale; /* scale */
    int duty;  /* duty pointer */
    int vel;   /* velocity */
    int pw;    /* power level (0:min 255:max) */
    int ec;    /* envelope counter */
    int ef;    /* envelope flag */
    int ai;    /* attack interval */
    int dl;    /* decay level */
    int di;    /* decay interval */
    int ri;    /* release interval */
};

struct VgsPsgContext {
    unsigned int hz;
    struct VgsPsgChannel ch[PSG_CHNUM];
};
static struct VgsPsgContext _context;
static struct VgsPsgContext* _ctx = &_context;

void cb(void* buffer, size_t size)
{
    unsigned char* buf = (unsigned char*)buffer;
    size_t p;
    int i;
    int w, ww;
    int n, m;
    memset(buf, 0, size);
    for (p = 0; p < size; p++, _ctx->hz++, buf++) {
        for (w = 0, i = 0; i < PSG_CHNUM; i++) {
            if (_ctx->ch[i].keyon) {
                n = P[_ctx->ch[i].scale];
                m = n / _ctx->ch[i].duty;
                if (_ctx->hz % n < m) {
                    if (1 == _ctx->ch[i].keyon) {
                        if (0 == _ctx->ch[i].ef) {
                            if (0 == _ctx->ch[i].ec % _ctx->ch[i].ai) {
                                _ctx->ch[i].pw++;
                                if (255 <= _ctx->ch[i].pw) {
                                    _ctx->ch[i].pw = 255;
                                    _ctx->ch[i].ec = 0;
                                    _ctx->ch[i].ef++;
                                }
                            }
                        } else if (1 == _ctx->ch[i].ef) {
                            if (_ctx->ch[i].pw <= _ctx->ch[i].dl) {
                                _ctx->ch[i].ef++;
                                _ctx->ch[i].ec = 0;
                            } else {
                                if (0 == ++_ctx->ch[i].ec % _ctx->ch[i].di) {
                                    _ctx->ch[i].pw--;
                                }
                            }
                        }
                        ww = (_ctx->ch[i].vel * _ctx->ch[i].pw) >> 8;
                        w += ww;
                    } else if (2 == _ctx->ch[i].keyon) {
                        if (_ctx->ch[i].pw) {
                            if (0 == _ctx->ch[i].ec % _ctx->ch[i].ri) {
                                _ctx->ch[i].pw--;
                            }
                            ww = (_ctx->ch[i].vel * _ctx->ch[i].pw) >> 8;
                            w += ww;
                        } else {
                            _ctx->ch[i].keyon = 0;
                        }
                    }
                    _ctx->ch[i].ec++;
                }
            }
        }
        if (255 < w) w = 255;
        *buf = (unsigned char)w;
    }
}

int main(int argc, char* argv[])
{
    char buf[1024];
    int n;
    int scale = 47;
    void* vgsspu = vgsspu_start2(SAMPLING_RATE, BIT_RATE, CHANNEL, 2048, cb);

    _ctx->ch[0].duty = 2;
    _ctx->ch[0].vel = 100;
    _ctx->ch[0].ai = 40;
    _ctx->ch[0].dl = 200;
    _ctx->ch[0].di = 8;
    _ctx->ch[0].ri = 50;
    _ctx->ch[0].scale = scale;

    while (1) {
        printf("COMMAND: ");
        memset(buf, 0, sizeof(buf));
        if (NULL == fgets(buf, sizeof(buf) - 1, stdin)) break;
        n = atoi(buf);
        if ('q' == buf[0]) break;
        if (0 == strncmp(buf, "on", 2)) {
            _ctx->ch[0].keyon = 1;
            _ctx->ch[0].pw = 0;
            _ctx->ch[0].ec = 0;
            _ctx->ch[0].ef = 0;
        } else if (0 == strncmp(buf, "of", 2)) {
            _ctx->ch[0].keyon = 2;
        } else if (n) {
            if (n < 1) {
                n = 1;
            } else if (88 < n) {
                n = 88;
            }
            _ctx->ch[0].scale = n;
            _ctx->ch[0].keyon = 1;
            _ctx->ch[0].pw = 0;
            _ctx->ch[0].ec = 0;
            _ctx->ch[0].ef = 0;
        }
    }
    return 0;
}
