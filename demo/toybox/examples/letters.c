// Author: GPT-4-turbo

#include "../toybox.h"
#include <stdlib.h> // 用于 rand() 和 srand()
#include <time.h>   // 用于 time()

#define MAX_LETTERS 50 // 屏幕上的最大字母数量

typedef struct {
    int x, y;       // 字母的当前位置
    int dx, dy;     // 字母的移动方向
    char ch;        // 要显示的字符
} Letter;

Letter letters[MAX_LETTERS]; // 存储所有字母的数组

void init_letters(int w, int h) {
    int i;

    srand(time(NULL)); // 初始化随机数生成器
    for (i = 0; i < MAX_LETTERS; i++) {
        letters[i].x = rand() % w;
        letters[i].y = rand() % h;
        letters[i].dx = (rand() % 3) - 1; // -1, 0, 或 1
        letters[i].dy = (rand() % 3) - 1; // -1, 0, 或 1
        letters[i].ch = 'A' + (rand() % 26); // 随机字母
    }
}

void update_letters(int w, int h) {
    int i;

    for (i = 0; i < MAX_LETTERS; i++) {
        letters[i].x += letters[i].dx;
        letters[i].y += letters[i].dy;

        // 检查边界，反转方向如果需要
        if (letters[i].x < 0 || letters[i].x >= w) {
            letters[i].dx = -letters[i].dx;
            letters[i].x += letters[i].dx * 2; // 防止卡在边界
        }
        if (letters[i].y < 0 || letters[i].y >= h) {
            letters[i].dy = -letters[i].dy;
            letters[i].y += letters[i].dy * 2; // 防止卡在边界
        }
    }
}

void update(int w, int h, draw_function draw) {
    static int initialized = 0;
    int i;

    if (!initialized) {
        init_letters(w, h);
        initialized = 1;
    }

    update_letters(w, h);

    for (i = 0; i < MAX_LETTERS; i++) {
        draw(letters[i].x, letters[i].y, letters[i].ch);
    }
}

void keypress(int key) {
    // 按键处理（如果需要）
}

int main(void) {
    toybox_run(20, update, keypress);
}
