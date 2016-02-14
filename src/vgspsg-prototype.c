#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgsspu.h"
#define SAMPLING_RATE 44100
#define BIT_RATE 8
#define CHANNEL 1

#define PSG_CHNUM 3

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
    memset(buf, 0, size);
    for (p = 0; p < size; p++, _ctx->hz++, buf++) {
        for (w = 0, i = 0; i < PSG_CHNUM; i++) {
            if (1 == _ctx->ch[i].keyon && _ctx->hz % _ctx->ch[i].scale < _ctx->ch[i].duty) {
                switch (_ctx->ch[i].ef) {
                    case 0:
                        if (0 == ++_ctx->ch[i].ec % _ctx->ch[i].ai) {
                            _ctx->ch[i].pw++;
                            if (255 <= _ctx->ch[i].pw) {
                                _ctx->ch[i].pw = 255;
                                _ctx->ch[i].ec = 0;
                                _ctx->ch[i].ef++;
                                break;
                            }
                        }
                        break;
                    case 1:
                        if (_ctx->ch[i].pw <= _ctx->ch[i].dl) {
                            _ctx->ch[i].ef++;
                            _ctx->ch[i].ec = 0;
                            break;
                        }
                        if (0 == ++_ctx->ch[i].ec % _ctx->ch[i].di) {
                            _ctx->ch[i].pw--;
                        }
                        break;
                }
                ww = (_ctx->ch[i].vel * _ctx->ch[i].pw) >> 8;
                w += ww;
            } else if (2 == _ctx->ch[i].keyon && _ctx->hz % _ctx->ch[i].scale < _ctx->ch[i].duty) {
                if (_ctx->ch[i].pw) {
                    if (0 == ++_ctx->ch[i].ec % _ctx->ch[i].ri) {
                        _ctx->ch[i].pw--;
                    }
                    ww = (_ctx->ch[i].vel * _ctx->ch[i].pw) >> 8;
                    w += ww;
                } else {
                    _ctx->ch[i].keyon = 0;
                }
            }
            *buf = (char)w;
        }
    }
}

int main(int argc, char* argv[])
{
    char buf[1024];
    int n;
    int scale = 72;
    void* vgsspu = vgsspu_start2(SAMPLING_RATE, BIT_RATE, CHANNEL, 2048, cb);
    while (1) {
        printf("COMMAND: ");
        memset(buf, 0, sizeof(buf));
        if (NULL == fgets(buf, sizeof(buf) - 1, stdin)) break;
        n = atoi(buf);
        if ('q' == buf[0]) break;
        if (0 == strncmp(buf, "on", 2)) {
            _ctx->ch[0].keyon = 1;
            _ctx->ch[0].scale = scale;
            _ctx->ch[0].duty = scale / 2;
            _ctx->ch[0].vel = 127;
            _ctx->ch[0].pw = 0;
            _ctx->ch[0].ec = 0;
            _ctx->ch[0].ef = 0;
            _ctx->ch[0].ai = 8;
            _ctx->ch[0].dl = 200;
            _ctx->ch[0].di = 10;
            _ctx->ch[0].ri = 20;
        } else if (0 == strncmp(buf, "of", 2)) {
            _ctx->ch[0].keyon = 2;
        } else if (n) {
            _ctx->ch[0].scale = n;
            _ctx->ch[0].duty = n / 2;
            printf("scale = %.4f Hz\n", (double)SAMPLING_RATE / n);
        }
    }
    return 0;
}
