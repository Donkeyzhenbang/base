// Author: Claude-3-Opus

#include "../toybox.h"
#include <vector>
using namespace std;

const int W = 60;
const int H = 20;

int shipX, shipY;  // 飞船坐标
vector<pair<int,int>> bullets; // 子弹坐标
vector<pair<int,int>> targets; // 目标坐标

void update(int w, int h, draw_function draw) {
    // 清屏
    for(int i = 0; i < W; i++) 
        for(int j = 0; j < H; j++)
            draw(i, j, ' ');
    
    // 绘制飞船
    draw(shipX, shipY, 'A'); 
    
    // 绘制子弹并更新子弹位置
    for(auto &b : bullets) {
        draw(b.first, b.second, '#');
        b.first++;
    }
    
    // 绘制目标
    for(auto &t : targets) {
        draw(t.first, t.second, 'X');
    }
    
    // 碰撞检测：移除被子弹击中的目标
    for(auto it = targets.begin(); it != targets.end();) {
        auto t = *it;
        bool hit = false;
        for(auto &b : bullets) {
            if(t == b) {
                hit = true;
                break;
            }
        }
        if(hit)
            it = targets.erase(it);
        else
            ++it;
    }
    
    // 移除超出屏幕的子弹
    for(auto it = bullets.begin(); it != bullets.end();) {
        if(it->first >= W) 
            it = bullets.erase(it);
        else
            ++it;
    }
}

void keypress(int key) {
    switch(key) {
        case 'a':
            if(shipX > 0) shipX--;
            break;
        case 'd':
            if(shipX < W-1) shipX++;
            break;
        case 'w':
            if(shipY > 0) shipY--;
            break;
        case 's':
            if(shipY < H-1) shipY++;
            break;
        case ' ': 
            bullets.push_back({shipX+1, shipY}); // 发射子弹
            break;
    }
}

int main() {
    shipX = shipY = 0;
    for(int i = 0; i < 5; i++)
        targets.push_back({W-1, rand()%H});
    toybox_run(20, update, keypress);
}
