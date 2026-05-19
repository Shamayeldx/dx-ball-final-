// ---------- Header Files ----------
#include <windows.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- Constants ----------
#define W 800
#define H 600
#define ROWS 5
#define COLS 10

// ---------- Game State ----------
int state = 0;

// ---------- Ball Variables ----------
float bx = 400, by = 150, vx = 4, vy = 4;
int br = 10;

// ---------- Paddle Variables ----------
float px = 340, py = 30, pw = 120, ph = 15;

// ---------- Game Stats ----------
int lives = 3, score = 0, hiScore = 0, level = 1;
int startTime = 0, gameTime = 0;
int fireball = 0;

// ---------- Bricks and Colors ----------
int brick[ROWS][COLS];
float bcol[ROWS][3] = {
    {1,0.2,0.2},{1,0.6,0},{1,1,0},{0,1,0.3},{0.3,0.6,1}
};

// ---------- Drops (Power-ups) ----------
struct Drop { int active, type; float x, y; };
Drop drops[10];

// ---------- Bresenham's Line Algorithm ----------
void line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2-x1), dy = abs(y2-y1);
    int sx = x1<x2?1:-1, sy = y1<y2?1:-1;
    int err = dx-dy;
    glBegin(GL_POINTS);
    while(1) {
        glVertex2i(x1, y1);
        if(x1==x2 && y1==y2) break;
        int e2 = 2*err;
        if(e2 > -dy) { err -= dy; x1 += sx; }
        if(e2 <  dx) { err += dx; y1 += sy; }
    }
    glEnd();
}

// ---------- Midpoint Circle Algorithm ----------
void circlePts(int xc, int yc, int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(xc+x,yc+y); glVertex2i(xc-x,yc+y);
    glVertex2i(xc+x,yc-y); glVertex2i(xc-x,yc-y);
    glVertex2i(xc+y,yc+x); glVertex2i(xc-y,yc+x);
    glVertex2i(xc+y,yc-x); glVertex2i(xc-y,yc-x);
    glEnd();
}
void circle(int xc, int yc, int r) {
    int x=0, y=r, d=1-r;
    circlePts(xc,yc,x,y);
    while(x<y) {
        if(d<0) d += 2*x+3;
        else { d += 2*(x-y)+5; y--; }
        x++;
        circlePts(xc,yc,x,y);
    }
}
void fillCircle(int xc, int yc, int r) {
    for(int i=0; i<=r; i++) circle(xc,yc,i);
}

// ---------- Boundary Fill (for Bricks) ----------
void boundaryFill(int x1, int y1, int x2, int y2, float r, float g, float b) {
    glColor3f(1,1,1);
    line(x1,y1,x2,y1); line(x2,y1,x2,y2);
    line(x2,y2,x1,y2); line(x1,y2,x1,y1);
    glColor3f(r,g,b);
    for(int y=y1+1; y<y2; y++) line(x1+1, y, x2-1, y);
}

// ---------- Text Drawing ----------
void text(float x, float y, const char *s, void *f = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    while(*s) glutBitmapCharacter(f, *s++);
}

// ---------- Initialize Bricks ----------
void initBricks() {
    for(int i=0;i<ROWS;i++)
        for(int j=0;j<COLS;j++)
            brick[i][j] = 1;
}

// ---------- Reset Ball ----------
void resetBall() {
    bx = px + pw/2;
    by = py + 40;
    vx = 4 + level*0.5;
    vy = 4 + level*0.5;
    fireball = 0;
}

// ---------- New Game Setup ----------
void newGame() {
    lives = 3; score = 0; level = 1; pw = 120;
    initBricks();
    for(int i=0;i<10;i++) drops[i].active = 0;
    resetBall();
    startTime = glutGet(GLUT_ELAPSED_TIME);
    state = 1;
}

// ---------- Draw Ball ----------
void drawBall() {
    glColor3f(fireball?1:1, fireball?0.4:1, fireball?0:1);
    fillCircle((int)bx, (int)by, br);
}

// ---------- Draw Paddle ----------
void drawPaddle() {
    glColor3f(0.2,0.7,1);
    for(int y=(int)py; y<=py+ph; y++) line((int)px, y, (int)(px+pw), y);
    glColor3f(1,1,1);
    line(px,py,px+pw,py); line(px+pw,py,px+pw,py+ph);
    line(px+pw,py+ph,px,py+ph); line(px,py+ph,px,py);
}

// ---------- Draw All Bricks ----------
void drawBricks() {
    for(int i=0;i<ROWS;i++)
        for(int j=0;j<COLS;j++)
            if(brick[i][j]) {
                int x1 = 60 + j*68, y1 = 480 - i*25;
                boundaryFill(x1, y1, x1+65, y1+22, bcol[i][0], bcol[i][1], bcol[i][2]);
            }
}

// ---------- Draw Drops (Power-ups) ----------
void drawDrops() {
    for(int i=0;i<10;i++) {
        if(!drops[i].active) continue;
        const char *lbl = "?";
        if(drops[i].type==0) { glColor3f(1,0,0); lbl="LIFE"; }
        else if(drops[i].type==1) { glColor3f(1,1,0); lbl="SPEED"; }
        else if(drops[i].type==2) { glColor3f(0,1,0); lbl="WIDE"; }
        else if(drops[i].type==3) { glColor3f(1,0.4,0); lbl="FIRE"; }
        else { glColor3f(0.5,0,0.5); lbl="DEATH"; }
        int x = drops[i].x, y = drops[i].y;
        for(int yy=y-9; yy<=y+9; yy++) line(x-28, yy, x+28, yy);
        glColor3f(0,0,0);
        int offset = 4 * (int)strlen(lbl);
        text(x - offset, y - 5, lbl, GLUT_BITMAP_HELVETICA_12);
    }
}

// ---------- Draw HUD (Score, Lives, Time, Level) ----------
void drawHUD() {
    char buf[80];
    glColor3f(1,1,1);
    sprintf(buf, "SCORE: %d", score);  text(20, H-30, buf);
    sprintf(buf, "LIVES: %d", lives);  text(180, H-30, buf);
    sprintf(buf, "TIME: %d", gameTime); text(330, H-30, buf);
    sprintf(buf, "LEVEL: %d", level);  text(470, H-30, buf);
    sprintf(buf, "HI: %d", hiScore);   text(620, H-30, buf);
    glColor3f(1,0.8,0);
    text(20, H-55, "P=Pause  M=Menu  ESC=Exit  Arrows/Mouse=Move");
    if(fireball) { glColor3f(1,0.4,0); text(620, H-55, "FIRE!"); }
}

// ---------- Draw Borders ----------
void drawBorders() {
    glColor3f(0.4,0.6,1);
    line(0,0,W-1,0); line(0,H-70,W-1,H-70);
    line(0,0,0,H-1); line(W-1,0,W-1,H-1);
}

// ---------- Draw Main Menu ----------
void drawMenu() {
    glColor3f(1,0.8,0);
    text(W/2-60, H-130, "D X   B A L L", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.4,0.8,1);
    line(200, H-160, W-200, H-160);
    glColor3f(1,0.5,0); fillCircle(W/2, H-200, 18);
    glColor3f(1,1,1);
    text(W/2-90, 380, "1. Start New Game", GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-90, 340, "2. Resume", GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-90, 300, "3. High Score", GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-90, 260, "4. Help", GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-90, 220, "5. Exit", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.7,0.7,0.7);
    text(W/2-130, 150, "Press 1/2/3/4/5 to choose");
}
