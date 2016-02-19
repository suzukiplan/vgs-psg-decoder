#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vgsspu.h"
#include "table.h" /* S:サイン波, Q:矩形波, W:ノコギリ波, A:音階テーブル */
#define SAMPLING_RATE 44100
#define BIT_RATE 16
#define CHANNEL 1

struct VgsPsgContext {
    unsigned int hz;
    int r;
    int m;
    int scale;
};
static struct VgsPsgContext _context;
static struct VgsPsgContext* _ctx = &_context;

void cb(void* buffer, size_t size)
{
    int i, w;
    short* buf = (short*)buffer;
    int s = 0;
    int fm;
    size >>= 1;

    for (i = 0; i < size; i++, buf++, _ctx->hz++) {
        fm = _ctx->r;                 // キャリア
        fm += S[_ctx->m >> 8] << 12;  // モジュレータ1（サイン波）
        fm += Q[_ctx->m >> 8] << 12;  // モジュレータ2（矩形波）
        fm += W[_ctx->m >> 8] << 12;  // モジュレータ3（ノコギリ波）
        // バッファオーバフロー防止
        if (fm < 0) {
            fm += 4410 * 256;
        } else {
            fm %= 4410 * 256;
        }
        // 求まった値を使ってテーブルから波形を読んで書く
        fm >>= 8;
        w = S[fm] << 4;
        *buf = (short)w;
        // キャリア & モジュレータ それぞれの周波数を進める
        _ctx->r += A[_ctx->scale];
        _ctx->r %= 4410 * 256;
        _ctx->m += A[_ctx->scale];
        _ctx->m %= 4410 * 256;
    }
}

int main(int argc, char* argv[])
{
    char buf[1024];
    void* vgsspu = vgsspu_start2(SAMPLING_RATE, BIT_RATE, CHANNEL, 4096, cb);
    _ctx->scale = 40;

    while (1) {
        printf("COMMAND: ");
        memset(buf, 0, sizeof(buf));
        if (NULL == fgets(buf, sizeof(buf) - 1, stdin)) break;
        if ('q' == buf[0]) break;
        if (isdigit(buf[0])) {
            _ctx->scale = atoi(buf);
        }
    }
    return 0;
}
