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

// ---------- Draw Help Screen ----------
void drawHelp() {
    glColor3f(1,1,0); text(W/2-40, H-120, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1,1,1);
    text(100, 450, "Left/Right Arrow or Mouse : Move paddle");
    text(100, 420, "P : Pause / Resume");
    text(100, 390, "M : Back to menu");
    text(100, 360, "ESC : Exit");
    text(100, 320, "PERKS (catch with paddle):");
    glColor3f(1,0,0);   text(120, 290, "LIFE  = Extra Life");
    glColor3f(1,1,0);   text(120, 265, "SPEED = Faster Ball");
    glColor3f(0,1,0);   text(120, 240, "WIDE  = Wider Paddle");
    glColor3f(1,0.4,0); text(120, 215, "FIRE  = Fireball (pierce bricks)");
    glColor3f(0.5,0,0.5); text(120, 190, "DEATH = Lose a life");
    glColor3f(0.7,0.7,1); text(W/2-100, 100, "Press M for menu");
}

// ---------- Draw High Score Screen ----------
void drawHiScore() {
    char buf[60];
    glColor3f(1,0.8,0); text(W/2-90, H-150, "HIGH SCORE", GLUT_BITMAP_TIMES_ROMAN_24);
    sprintf(buf, "BEST: %d", hiScore);
    glColor3f(1,1,1); text(W/2-60, H/2, buf, GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.7,0.7,1); text(W/2-100, 100, "Press M for menu");
}

// ---------- Draw Pause Screen ----------
void drawPause() {
    glColor3f(1,1,0); text(W/2-50, H/2+20, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1,1,1); text(W/2-110, H/2-20, "Press P to Resume, M for Menu");
}

// ---------- Draw Game Over Screen ----------
void drawGameOver() {
    char buf[60];
    glColor3f(1,0.2,0.2); text(W/2-80, H/2+50, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
    sprintf(buf, "Score: %d", score);
    glColor3f(1,1,1); text(W/2-50, H/2+10, buf, GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-130, H/2-30, "Press R to Restart, M for Menu");
}

// ---------- Draw Win Screen ----------
void drawWin() {
    char buf[60];
    glColor3f(0.2,1,0.4); text(W/2-100, H/2+60, "LEVEL CLEARED!", GLUT_BITMAP_TIMES_ROMAN_24);
    sprintf(buf, "Score: %d  Time: %d s", score, gameTime);
    glColor3f(1,1,1); text(W/2-120, H/2+20, buf, GLUT_BITMAP_TIMES_ROMAN_24);
    text(W/2-140, H/2-20, "Press N for Next Level, M for Menu");
}

// ---------- Main Display Function ----------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if(state==0) drawMenu();
    else if(state==5) drawHelp();
    else if(state==6) drawHiScore();
    else {
        drawBorders();
        drawBricks();
        drawPaddle();
        drawBall();
        drawDrops();
        drawHUD();
        if(state==2) drawPause();
        if(state==3) drawGameOver();
        if(state==4) drawWin();
    }
    glutSwapBuffers();
}

// ---------- Spawn Drop (when brick breaks) ----------
void spawnDrop(float x, float y) {
    if(rand()%100 >= 15) return;
    for(int i=0;i<10;i++) {
        if(!drops[i].active) {
            drops[i].active = 1;
            drops[i].x = x; drops[i].y = y;
            int r = rand()%100;
            if(r < 25)      drops[i].type = 0;
            else if(r < 45) drops[i].type = 1;
            else if(r < 70) drops[i].type = 2;
            else if(r < 85) drops[i].type = 3;
            else            drops[i].type = 4;
            return;
        }
    }
}

// ---------- Apply Drop Effect ----------
void applyDrop(int t) {
    if(t==0) lives++;
    else if(t==1) { vx*=1.3; vy*=1.3; }
    else if(t==2) pw = 180;
    else if(t==3) fireball = 1;
    else if(t==4) {
        lives--;
        if(lives<=0) {
            if(score>hiScore) hiScore = score;
            state = 3;
        } else resetBall();
    }
}

// ---------- Check if All Bricks Cleared ----------
int allCleared() {
    for(int i=0;i<ROWS;i++)
        for(int j=0;j<COLS;j++)
            if(brick[i][j]) return 0;
    return 1;
}

// ---------- Game Update Loop ----------
void update(int v) {
    if(state==1) {
        gameTime = (glutGet(GLUT_ELAPSED_TIME) - startTime) / 1000;
        bx += vx; by += vy;

        // Wall collision
        if(bx-br<1) { bx=br+1; vx=-vx; }
        if(bx+br>W-1) { bx=W-br-1; vx=-vx; }
        if(by+br>H-70) { by=H-70-br; vy=-vy; }

        // Ball falls below - lose life
        if(by-br<0) {
            lives--;
            if(lives<=0) {
                if(score>hiScore) hiScore = score;
                state = 3;
            } else resetBall();
        }

        // Paddle collision
        if(by-br<=py+ph && by-br>=py && bx>=px && bx<=px+pw && vy<0) {
            vy = -vy;
            vx = ((bx - px)/pw - 0.5) * 10;
            by = py + ph + br + 1;
        }

        // Brick collision
        for(int i=0;i<ROWS;i++) {
            for(int j=0;j<COLS;j++) {
                if(!brick[i][j]) continue;
                int x1 = 60 + j*68, y1 = 480 - i*25;
                int x2 = x1+65, y2 = y1+22;
                if(bx+br>x1 && bx-br<x2 && by+br>y1 && by-br<y2) {
                    brick[i][j] = 0;
                    score += 10;
                    spawnDrop(x1+32, y1);
                    if(!fireball) vy = -vy;
                    goto done;
                }
            }
        }
        done:

        // Update drops
        for(int i=0;i<10;i++) {
            if(!drops[i].active) continue;
            drops[i].y -= 3;
            if(drops[i].y<0) drops[i].active = 0;
            else if(drops[i].y<=py+ph && drops[i].x>=px && drops[i].x<=px+pw) {
                applyDrop(drops[i].type);
                drops[i].active = 0;
            }
        }

        // Win check
        if(allCleared()) {
            if(score>hiScore) hiScore = score;
            state = 4;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ---------- Keyboard Input ----------
void keyboard(unsigned char k, int x, int y) {
    if(state==0) {
        if(k=='1') newGame();
        else if(k=='2' && lives>0) state = 1;
        else if(k=='3') state = 6;
        else if(k=='4') state = 5;
        else if(k=='5' || k==27) exit(0);
    }
    else if(state==5 || state==6) { if(k=='m'||k=='M') state = 0; }
    else if(state==1) {
        if(k=='p'||k=='P') state = 2;
        else if(k=='m'||k=='M') state = 0;
        else if(k==27) exit(0);
    }
    else if(state==2) {
        if(k=='p'||k=='P') state = 1;
        else if(k=='m'||k=='M') state = 0;
    }
    else if(state==3) {
        if(k=='r'||k=='R') newGame();
        else if(k=='m'||k=='M') state = 0;
    }
    else if(state==4) {
        if(k=='n'||k=='N') { level++; pw=120; initBricks(); resetBall(); state=1; }
        else if(k=='m'||k=='M') state = 0;
    }
}

// ---------- Arrow Key Input ----------
void special(int k, int x, int y) {
    if(state!=1) return;
    if(k==GLUT_KEY_LEFT) px -= 20;
    else if(k==GLUT_KEY_RIGHT) px += 20;
    if(px<1) px = 1;
    if(px+pw>W-1) px = W-1-pw;
}

// ---------- Mouse Input ----------
void mouse(int x, int y) {
    if(state!=1) return;
    px = x - pw/2;
    if(px<1) px = 1;
    if(px+pw>W-1) px = W-1-pw;
}

// ---------- Initialize OpenGL ----------
void init() {
    glClearColor(0.05, 0.05, 0.15, 1);
    gluOrtho2D(0, W, 0, H);
    srand(time(NULL));
    initBricks();
}

// ---------- Main Function ----------
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("DX BALL - CSE 426");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutPassiveMotionFunc(mouse);
    glutMotionFunc(mouse);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
//done