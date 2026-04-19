#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h> 
#endif

#include <iostream>
#include <cmath>
using namespace std;

const int WIDTH  = 1400;
const int HEIGHT = 900;

const int BASE_Y1 = 250; 
const int BASE_Y2 = 285;

bool heliShow = false;  
enum HeliState {
    HELI_HIDDEN = 0,
    HELI_ENTERING,
    HELI_LANDING,
    HELI_LANDED,
    HELI_TAKEOFF_UP,
    HELI_EXITING
};

HeliState heliState = HELI_HIDDEN;

void drawTextBigStroke(const char* text, float x, float y, float scale)
{
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(scale, scale, 1.0f); 
        glLineWidth(5.0f);     
        for(const char* p=text; *p; p++){
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
        }
    glPopMatrix();
}

void drawPixel(float x, float y){
    glBegin(GL_POINTS);
        glVertex2f(x, y);
    glEnd();
}

void plot8(int xc, int yc, int x, int y){
    drawPixel(xc + x, yc + y);
    drawPixel(xc - x, yc + y);
    drawPixel(xc + x, yc - y);
    drawPixel(xc - x, yc - y);

    drawPixel(xc + y, yc + x);
    drawPixel(xc - y, yc + x);
    drawPixel(xc + y, yc - x);
    drawPixel(xc - y, yc - x);
}

void plotUpper4(int xc, int yc, int x, int y){
    drawPixel(xc + x, yc + y);
    drawPixel(xc - x, yc + y);
    drawPixel(xc + y, yc + x);
    drawPixel(xc - y, yc + x);
}

void fillSpanFull(int xc, int yc, int x, int y){
    glBegin(GL_LINES);
        glVertex2f(xc - x, yc + y);
        glVertex2f(xc + x, yc + y);

        glVertex2f(xc - x, yc - y);
        glVertex2f(xc + x, yc - y);

        glVertex2f(xc - y, yc + x);
        glVertex2f(xc + y, yc + x);

        glVertex2f(xc - y, yc - x);
        glVertex2f(xc + y, yc - x);
    glEnd();
}

void fillSpanUpper(int xc, int yc, int x, int y){
    glBegin(GL_LINES);
        glVertex2f(xc - x, yc + y);
        glVertex2f(xc + x, yc + y);

        glVertex2f(xc - y, yc + x);
        glVertex2f(xc + y, yc + x);
    glEnd();
}


void midpointCircle(int xc, int yc, int r, bool upperHalf=false, bool filled=false){
    int x = 0;
    int y = r;
    int pk = 1 - r;

    if(filled){
        if(upperHalf) fillSpanUpper(xc,yc,x,y);
        else          fillSpanFull(xc,yc,x,y);
    } else {
        if(upperHalf) plotUpper4(xc,yc,x,y);
        else          plot8(xc,yc,x,y);
    }

    while(x < y){
        x++;
        if(pk < 0){
            pk = pk + 2*x + 1;
        } else {
            y--;
            pk = pk + 2*x - 2*y + 1;
        }

        if(filled){
            if(upperHalf) fillSpanUpper(xc,yc,x,y);
            else          fillSpanFull(xc,yc,x,y);
        } else {
            if(upperHalf) plotUpper4(xc,yc,x,y);
            else          plot8(xc,yc,x,y);
        }
    }
}


void drawDomeMidpoint(float domeCenterX, float domeBaseY, float domeRadius)
{
    int xc = (int)domeCenterX;
    int yc = (int)domeBaseY;
    int r  = (int)domeRadius;

    glColor3f(0.82f, 0.82f, 0.84f);
    midpointCircle(xc, yc, r, true, true);

    for(int k = 1; k <= 4; k++){
        float rr = r * (1.0f - k * 0.18f); 
        glColor3f(0.70f, 0.70f, 0.73f);   
        glPointSize(2.0f);
        midpointCircle(xc, yc, (int)rr, true, false);
    }

    glColor3f(0.65f, 0.65f, 0.68f);
    glLineWidth(1.5f);

    int ribs = 9;
    float startA = M_PI;  
    float endA   = 0.0f; 

    glBegin(GL_LINES);
    for(int i=0;i<=ribs;i++){
        float a = startA + (endA-startA) * i / ribs;

        float xTop = xc + r * cosf(a);
        float yTop = yc + r * sinf(a);

        float xBase = xc + (r*0.10f) * cosf(a);
        float yBase = yc + (r*0.10f) * sinf(a);

        glVertex2f(xBase, yBase);
        glVertex2f(xTop,  yTop);
    }
    glEnd();

    glColor3f(0.55f, 0.55f, 0.58f);
    for(int ty = 1; ty <= 3; ty++){
        float rr = r * (1.0f - ty * 0.25f);
        int dots = 8 + ty*2;

        for(int j=0;j<dots;j++){
            float a = startA + (endA-startA) * j / dots;
            int px = (int)(xc + rr * cosf(a));
            int py = (int)(yc + rr * sinf(a));
            midpointCircle(px, py, 2, false, true);
        }
    }

    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(3.0f);
    midpointCircle(xc, yc, r, true, false);
}

void drawLineBresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        drawPixel((float)x1, (float)y1);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}


void ddaLine(float x1,float y1,float x2,float y2){
    float dx = x2-x1, dy = y2-y1;
    float steps = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy);
    float xInc = dx/steps, yInc = dy/steps;
    float x=x1, y=y1;

    glBegin(GL_POINTS);
    for(int i=0;i<=steps;i++){
        glVertex2f(x,y);
        x+=xInc; y+=yInc;
    }
    glEnd();
}




//==============================SUN===============================
static float smooth01(float x){
    if(x < 0) x = 0;
    if(x > 1) x = 1;
    return x * x * (3.0f - 2.0f * x);
}

void getSkyColor(float t, float &r, float &g, float &b)
{
    if(t < 0) t = 0;
    if(t > 1) t = 1;

    float r1=0.55f, g1=0.78f, b1=0.98f;  

    float r2=0.40f, g2=0.70f, b2=0.98f;  

    float rMid=0.95f, gMid=0.75f, bMid=0.55f; 

    float r3=0.06f, g3=0.06f, b3=0.14f;  

    if(t <= 0.45f){
        float u = smooth01(t / 0.45f);
        r = r1 + (r2 - r1) * u;
        g = g1 + (g2 - g1) * u;
        b = b1 + (b2 - b1) * u;
    }
    else if(t <= 0.65f){
        float u = smooth01((t - 0.45f) / 0.20f);
        r = r2 + (rMid - r2) * u;
        g = g2 + (gMid - g2) * u;
        b = b2 + (bMid - b2) * u;
    }
    else {
        float u = smooth01((t - 0.65f) / 0.35f);
        r = rMid + (r3 - rMid) * u;
        g = gMid + (g3 - gMid) * u;
        b = bMid + (b3 - bMid) * u;
    }
}

float drawSunMidpointAnimated()
{
    static float sunR = 45.0f;

    static float startX = 1500.0f;
    static float endX   = -50.0f; 
    static float baseY  = 740.0f; 
    static float peakY  = 820.0f; 

    static float t = 0.0f;       
    static float speedT = 0.00015f;

    t += speedT;
    if (t > 1.0f) t = 0.0f;

    float sunX = startX + (endX - startX) * t;        
    float sunY = baseY + (peakY - baseY) * sinf(t * M_PI); 

    glPushMatrix();
        glTranslatef(sunX, sunY, 0.0f);

        glColor3f(1.0f, 0.85f, 0.15f);
        midpointCircle(0, 0, (int)sunR, false, true);

        glColor3f(1.0f, 0.6f, 0.05f);
        glPointSize(2.0f);
        midpointCircle(0, 0, (int)sunR, false, false);

        glColor3f(1.0f, 0.7f, 0.1f);
        glLineWidth(2.0f);
        int rays = 12;
        float rayLen = sunR + 18.0f;

        glBegin(GL_LINES);
        for(int i=0;i<rays;i++){
            float a = (2.0f * M_PI * i) / rays;
            float x1 = sunR * cosf(a);
            float y1 = sunR * sinf(a);
            float x2 = rayLen * cosf(a);
            float y2 = rayLen * sinf(a);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        }
        glEnd();
    glPopMatrix();

    return t;
}
//======================================================

//======================HUMAN=============================================

void thickLine(float x1,float y1,float x2,float y2, float thickness){
    float dx = x2-x1, dy = y2-y1;
    float len = sqrtf(dx*dx + dy*dy);
    if(len==0) return;
    float ox = -(dy/len) * (thickness*0.5f);
    float oy =  (dx/len) * (thickness*0.5f);

    ddaLine(x1+ox, y1+oy, x2+ox, y2+oy);
    ddaLine(x1-ox, y1-oy, x2-ox, y2-oy);
}

void drawHumanRealisticSwing(float x, float y, float scale,float shirtR, float shirtG, float shirtB,float pantR,  float pantG,  float pantB,float swing){
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float headR   = 12.0f;
    float headCY  = 68.0f;

    float neckW = 8.0f, neckH = 6.0f;

    float shoulderY = 50.0f;
    float hipY      = 25.0f;

    float torsoTopW = 26.0f;
    float torsoBotW = 18.0f;

    float armLen = 8.0f;
    float legLen = 28.0f;
    float limbThick = 3.5f;

    // Head
    glColor3f(1.0f, 0.85f, 0.70f);
    midpointCircle(0, (int)headCY, (int)headR, false, true);

    glColor3f(0.10f, 0.10f, 0.10f);
    midpointCircle(0, (int)(headCY+2), (int)(headR+1), true, true);

    glColor3f(0.0f,0.0f,0.0f);
    midpointCircle(-4, (int)(headCY+2), 2, false, true);
    midpointCircle( 4, (int)(headCY+2), 2, false, true);

    glColor3f(0.15f,0.15f,0.15f);
    glPointSize(2.0f);
    midpointCircle(0, (int)headCY, (int)headR, false, false);

    // Neck 
    glColor3f(0.95f, 0.80f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(-neckW/2, headCY-headR-1);
        glVertex2f( neckW/2, headCY-headR-1);
        glVertex2f( neckW/2, headCY-headR-1-neckH);
        glVertex2f(-neckW/2, headCY-headR-1-neckH);
    glEnd();

    //Torso (shirt)
    glColor3f(shirtR, shirtG, shirtB);
    glBegin(GL_QUADS);
        glVertex2f(-torsoTopW/2, shoulderY);
        glVertex2f( torsoTopW/2, shoulderY);
        glVertex2f( torsoBotW/2, hipY);
        glVertex2f(-torsoBotW/2, hipY);
    glEnd();

    glColor3f(0.95f, 0.80f, 0.65f);
    float armSwing = swing * 8.0f;

    thickLine(-torsoTopW/2, shoulderY-2,
              -torsoTopW/2-armLen, shoulderY-10 + armSwing,
              limbThick);

    thickLine( torsoTopW/2, shoulderY-2,
               torsoTopW/2+armLen, shoulderY-10 - armSwing,
               limbThick);

    midpointCircle((int)(-torsoTopW/2-armLen),
                   (int)(shoulderY-10 + armSwing), 3, false, true);
    midpointCircle((int)( torsoTopW/2+armLen),
                   (int)(shoulderY-10 - armSwing), 3, false, true);


    glColor3f(pantR, pantG, pantB);
    float legSwing = swing * 6.0f;

    thickLine(-6, hipY,
              -10, hipY-legLen - legSwing,
              limbThick);

    thickLine( 6, hipY,
               10, hipY-legLen + legSwing,
               limbThick);

    glColor3f(0.05f,0.05f,0.05f);
    midpointCircle(-10, (int)(hipY-legLen - legSwing), 4, false, true);
    midpointCircle( 10, (int)(hipY-legLen + legSwing), 4, false, true);

    glPopMatrix();
}



void drawHumanWalkingH(
    float startX, float groundY, float scale,
    float shirtR, float shirtG, float shirtB,
    float pantR,  float pantG,  float pantB,
    float speed,
    float phaseOffset,
    bool dirRight = true
){
    static float walkX = startX;
    static float walkT = 0.0f;

    walkX += (dirRight ? speed : -speed);
    walkT += 0.08f;

    if(dirRight && walkX > WIDTH + 40) walkX = -40;
    if(!dirRight && walkX < -40)       walkX = WIDTH + 40;

    float swing = sinf(walkT + phaseOffset);

    glPushMatrix();
        glTranslatef(walkX, groundY, 0.0f);
        glScalef(scale, scale, 1.0f);
        if(!dirRight){
            glScalef(-1.0f, 1.0f, 1.0f);
        }
        drawHumanRealisticSwing(0, 0, 1.0f,
            shirtR, shirtG, shirtB,
            pantR, pantG, pantB,
            swing
        );

    glPopMatrix();
}



void drawHumanWalkingV(
    int id,                 
    float posX, float startY, float endY, float scale,
    float shirtR, float shirtG, float shirtB,
    float pantR,  float pantG,  float pantB,
    float speed = 0.45f,
    float phaseOffset = 0.0f,
    bool goUpInitially = true
){
    const int MAX_HUMANS = 64; 

    static float yPos[MAX_HUMANS];
    static bool  goingUp[MAX_HUMANS];
    static bool  initialized[MAX_HUMANS] = {false};
    static float tWalk[MAX_HUMANS] = {0};

    if(id < 0) id = 0;
    if(id >= MAX_HUMANS) id = MAX_HUMANS - 1;

    if(!initialized[id]){
        goingUp[id] = goUpInitially;
        yPos[id] = goingUp[id] ? startY : endY;
        initialized[id] = true;
    }

    if(goingUp[id]){
        yPos[id] += speed;
        if(yPos[id] >= endY){
            yPos[id] = endY;
            goingUp[id] = false;   
        }
    } else {
        yPos[id] -= speed;
        if(yPos[id] <= startY){
            yPos[id] = startY;
            goingUp[id] = true;    
        }
    }

    tWalk[id] += 0.08f;
    float swing = sinf(tWalk[id] + phaseOffset);

    drawHumanRealisticSwing(posX, yPos[id], scale,
                            shirtR, shirtG, shirtB,
                            pantR, pantG, pantB,
                            swing);
}

//===============================================================



// ==========================Single Lamp Post ===================
void drawLampPost(float x, float y, float scale, bool lightOn)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    glColor3f(0.12f, 0.12f, 0.12f); 
    glLineWidth(2.0f);
    ddaLine(0, 0, 0, 60);
    ddaLine(0, 55, 18, 55);

    glColor3f(0.05f, 0.05f, 0.05f);
    glPointSize(2.0f);
    midpointCircle(18, 55, 6, false, false);

    if(lightOn)
        glColor3f(1.0f, 0.95f, 0.60f);  
    else
        glColor3f(0.65f, 0.65f, 0.70f); 

    midpointCircle(18, 55, 4, false, true);

    glColor3f(0.0f, 0.0f, 0.0f);
    midpointCircle(0, 0, 5, false, false);

    glPopMatrix();
}

void drawLampPostMirrorX(float x, float y, float scale, bool lightOn, float mirrorCenterX)
{
    glPushMatrix();

    glTranslatef(2*mirrorCenterX - x, y, 0);
    glScalef(-1.0f, 1.0f, 1.0f);

    drawLampPost(0, 0, scale, lightOn); 

    glPopMatrix();
}

//======================================================================


//===========================Tree==============================

void drawTree(float x, float y, float scale = 1.0f)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float trunkW = 22.0f;
    float trunkH = 70.0f;

    glColor3f(0.45f, 0.25f, 0.08f);
    glBegin(GL_QUADS);
        glVertex2f(-trunkW*0.5f, 0.0f);
        glVertex2f( trunkW*0.5f, 0.0f);
        glVertex2f( trunkW*0.5f, trunkH);
        glVertex2f(-trunkW*0.5f, trunkH);
    glEnd();
\
    glColor3f(0.55f, 0.32f, 0.12f);
    glBegin(GL_QUADS);
        glVertex2f(-trunkW*0.15f, 5.0f);
        glVertex2f( trunkW*0.05f, 5.0f);
        glVertex2f( trunkW*0.05f, trunkH-5.0f);
        glVertex2f(-trunkW*0.15f, trunkH-5.0f);
    glEnd();
    
    glColor3f(0.25f, 0.12f, 0.05f);
    glPointSize(2.0f);

    drawLineBresenham((int)(-trunkW*0.5f), 0, (int)( trunkW*0.5f), 0);
    drawLineBresenham((int)( trunkW*0.5f), 0, (int)( trunkW*0.5f), (int)trunkH);
    drawLineBresenham((int)( trunkW*0.5f), (int)trunkH, (int)(-trunkW*0.5f), (int)trunkH);
    drawLineBresenham((int)(-trunkW*0.5f), (int)trunkH, (int)(-trunkW*0.5f), 0);

    drawLineBresenham(0, (int)(trunkH-5),  -18, (int)(trunkH+15));
    drawLineBresenham(0, (int)(trunkH-5),   18, (int)(trunkH+15));

    glColor3f(0.05f, 0.55f, 0.18f);
    midpointCircle(0,  trunkH + 25, 38, false, true);
    midpointCircle(-30, trunkH + 15, 30, false, true);
    midpointCircle(30,  trunkH + 15, 30, false, true);

    glColor3f(0.10f, 0.70f, 0.25f);
    midpointCircle(0,   trunkH + 45, 30, false, true);
    midpointCircle(-22, trunkH + 40, 22, false, true);
    midpointCircle(22,  trunkH + 40, 22, false, true);

    glColor3f(0.25f, 0.85f, 0.35f);
    midpointCircle(0, trunkH + 58, 18, false, true);

    glPopMatrix();
}


void drawTreePine(float x, float y, float scale = 1.0f)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float trunkW = 18.0f;
    float trunkH = 50.0f;

    glColor3f(0.45f, 0.25f, 0.08f);
    glBegin(GL_QUADS);
        glVertex2f(-trunkW*0.5f, 0.0f);
        glVertex2f( trunkW*0.5f, 0.0f);
        glVertex2f( trunkW*0.5f, trunkH);
        glVertex2f(-trunkW*0.5f, trunkH);
    glEnd();

    float baseY = trunkH;

    glColor3f(0.04f, 0.50f, 0.15f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f,  baseY + 95.0f);
        glVertex2f(-70.0f, baseY);
        glVertex2f(70.0f,  baseY);
    glEnd();

    glColor3f(0.06f, 0.60f, 0.18f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f,  baseY + 75.0f);
        glVertex2f(-55.0f, baseY + 20.0f);
        glVertex2f(55.0f,  baseY + 20.0f);
    glEnd();

    glColor3f(0.08f, 0.70f, 0.22f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f,  baseY + 55.0f);
        glVertex2f(-40.0f, baseY + 38.0f);
        glVertex2f(40.0f,  baseY + 38.0f);
    glEnd();

    glPopMatrix();
}



void drawTreePalm(float x, float y, float scale = 1.0f)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glColor3f(0.50f, 0.28f, 0.10f);
    glLineWidth(6.0f);

    glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(8.0f, 30.0f);

        glVertex2f(8.0f, 30.0f);
        glVertex2f(4.0f, 65.0f);

        glVertex2f(4.0f, 65.0f);
        glVertex2f(12.0f, 95.0f);
    glEnd();

    float topX = 12.0f, topY = 95.0f;

    glColor3f(0.05f, 0.60f, 0.18f);

    glBegin(GL_TRIANGLES);
        glVertex2f(topX, topY);
        glVertex2f(topX - 70.0f, topY - 10.0f);
        glVertex2f(topX - 25.0f, topY - 40.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
        glVertex2f(topX, topY);
        glVertex2f(topX - 55.0f, topY + 10.0f);
        glVertex2f(topX - 15.0f, topY - 25.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
        glVertex2f(topX, topY);
        glVertex2f(topX + 70.0f, topY - 10.0f);
        glVertex2f(topX + 25.0f, topY - 40.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
        glVertex2f(topX, topY);
        glVertex2f(topX + 55.0f, topY + 10.0f);
        glVertex2f(topX + 15.0f, topY - 25.0f);
    glEnd();


    glColor3f(0.10f, 0.75f, 0.25f);
    glBegin(GL_TRIANGLES);
        glVertex2f(topX, topY + 20.0f);
        glVertex2f(topX - 20.0f, topY - 5.0f);
        glVertex2f(topX + 20.0f, topY - 5.0f);
    glEnd();

    glPopMatrix();
}


//========================== CLOUD ================================

float drawSunMidpointAnimated();

float getCloudShade(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float shade;
    if (t <= 0.5f) {
        float u = t / 0.5f;                 
        shade = 1.0f - 0.10f * u;          
    } else {
        float u = (t - 0.5f) / 0.5f;        
        shade = 0.90f * (1.0f - u) + 0.30f * u;
    }
    return shade;
}

void drawCloud(float x, float y, float scale, float shade)
{
    glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(scale, scale, 1.0f);

        float r = 0.95f * shade;
        float g = 0.95f * shade;
        float b = 0.97f * shade;

        glColor3f(r, g, b);

        midpointCircle(0,   0,  28, false, true);
        midpointCircle(30,  8,  35, false, true);
        midpointCircle(68,  0,  28, false, true);
        midpointCircle(42, 22,  30, false, true);
        midpointCircle(32, -8,  25, false, true);
    glPopMatrix();
}

void drawCloud(float x, float y, float scale)
{
    drawCloud(x, y, scale, 1.0f);
}


void drawCloudAnimated(float t)
{
    static float c1x = 200.0f;
    static float c2x = 700.0f;
    static float c3x = 1050.0f;

    const float speed1 = 0.15f;
    const float speed2 = 0.10f;
    const float speed3 = 0.18f;

    c1x += speed1;
    c2x += speed2;
    c3x += speed3;

    if (c1x > WIDTH + 80) c1x = -80;
    if (c2x > WIDTH + 80) c2x = -80;
    if (c3x > WIDTH + 80) c3x = -80;

    float shade = getCloudShade(t);

    drawCloud(c1x, 780.0f, 1.0f, shade);
    drawCloud(c2x, 820.0f, 1.3f, shade);
    drawCloud(c3x, 760.0f, 0.9f, shade);
}




//===============================Bus==============================

void drawBus(float x, float y, float scale = 1.0f)
{

    float busW = 320.0f * scale;
    float busH = 90.0f  * scale;

    float wheelR = 18.0f * scale;
    float wheelY = y + wheelR; 

    // Body main
    glColor3f(0.10f, 0.65f, 0.20f);
    glBegin(GL_QUADS);
        glVertex2f(x,y + wheelR);         
        glVertex2f(x + busW,y + wheelR);
        glVertex2f(x + busW,y + wheelR + busH);
        glVertex2f(x,y + wheelR + busH);
    glEnd();

    //Roof
    glColor3f(0.05f, 0.55f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(x,          y + wheelR + busH - 12*scale);
        glVertex2f(x + busW,   y + wheelR + busH - 12*scale);
        glVertex2f(x + busW,   y + wheelR + busH);
        glVertex2f(x,          y + wheelR + busH);
    glEnd();

    glColor3f(0.10f, 0.60f, 0.18f);
    glBegin(GL_QUADS);
        glVertex2f(x + busW - 55*scale, y + wheelR);
        glVertex2f(x + busW,            y + wheelR);
        glVertex2f(x + busW,            y + wheelR + busH);
        glVertex2f(x + busW - 30*scale, y + wheelR + busH);
    glEnd();


    float winY1 = y + wheelR + busH*0.40f;
    float winY2 = y + wheelR + busH*0.85f;

    glColor3f(0.70f, 0.90f, 1.00f); 
    int windows = 6;
    float marginL = 25.0f * scale;
    float marginR = 70.0f * scale; 
    float gap = 8.0f * scale;

    float winAreaW = busW - marginL - marginR;
    float eachW = (winAreaW - gap*(windows-1)) / windows;

    for(int i=0;i<windows;i++){
        float wx1 = x + marginL + i*(eachW + gap);
        float wx2 = wx1 + eachW;
        glBegin(GL_QUADS);
            glVertex2f(wx1, winY1);
            glVertex2f(wx2, winY1);
            glVertex2f(wx2, winY2);
            glVertex2f(wx1, winY2);
        glEnd();
    }

    glBegin(GL_QUADS);
        glVertex2f(x + busW - 50*scale, winY1);
        glVertex2f(x + busW - 8*scale,  winY1);
        glVertex2f(x + busW - 8*scale,  winY2);
        glVertex2f(x + busW - 35*scale, winY2);
    glEnd();

    // Door 
    glColor3f(0.05f, 0.45f, 0.12f);
    float doorW = 35.0f * scale;
    float doorH = 60.0f * scale;
    float doorX1 = x + busW*0.35f;
    float doorY1 = y + wheelR;
    glBegin(GL_QUADS);
        glVertex2f(doorX1,          doorY1);
        glVertex2f(doorX1+doorW,    doorY1);
        glVertex2f(doorX1+doorW,    doorY1+doorH);
        glVertex2f(doorX1,          doorY1+doorH);
    glEnd();

    //Outline 
    glColor3f(0.0f, 0.2f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,        y + wheelR);
        glVertex2f(x+busW,   y + wheelR);
        glVertex2f(x+busW,   y + wheelR + busH);
        glVertex2f(x,        y + wheelR + busH);
    glEnd();

    // Wheels  
    float w1x = x + 70.0f * scale;
    float w2x = x + busW - 90.0f * scale;

    glColor3f(0.0f, 0.0f, 0.0f);
    midpointCircle((int)w1x, (int)wheelY, (int)wheelR, false, true);
    midpointCircle((int)w2x, (int)wheelY, (int)wheelR, false, true);

    glColor3f(0.6f, 0.6f, 0.6f);
    midpointCircle((int)w1x, (int)wheelY, (int)(wheelR*0.55f), false, true);
    midpointCircle((int)w2x, (int)wheelY, (int)(wheelR*0.55f), false, true);

    glColor3f(1.0f, 1.0f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(x + busW - 8*scale, y + wheelR + 18*scale);
        glVertex2f(x + busW + 6*scale, y + wheelR + 18*scale);
        glVertex2f(x + busW + 6*scale, y + wheelR + 30*scale);
        glVertex2f(x + busW - 8*scale, y + wheelR + 30*scale);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);   

    float textX = x + busW * 0.45f; 
    float textY = y + wheelR + busH * 0.35f;

    drawTextBigStroke("DIU", textX, textY,0.3f);
}



void drawRoadSegment(float x1, float x2, float y1, float y2)
{
    glColor3f(0.20f, 0.20f, 0.20f);
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(x1, y1 + 3);
        glVertex2f(x2, y1 + 3);
        glVertex2f(x1, y2 - 3);
        glVertex2f(x2, y2 - 3);
    glEnd();

    glColor3f(0.95f, 0.85f, 0.20f);
    glLineWidth(3.0f);
    float centerY = (y1 + y2) * 0.5f;

    glBegin(GL_LINES);
        float dashLen = 35.0f;
        float gapLen  = 20.0f;
        float x = x1;
        while (x < x2) {
            glVertex2f(x, centerY);
            glVertex2f(x + dashLen, centerY);
            x += dashLen + gapLen;
        }
    glEnd();
}



//=======================Central Field===================

void drawGoalPost(float x, float y1, float y2, bool isLeft)
{
    float goalW = 18.0f;                 
    float postT = 4.0f;                
    float crossT = 4.0f;

    float goalH = (y2 - y1) * 0.40f;
    float goalY1 = y1 + (y2 - y1 - goalH) * 0.5f;
    float goalY2 = goalY1 + goalH;

    float gx1, gx2;
    if(isLeft){
        gx1 = x - goalW; 
        gx2 = x;
    } else {
        gx1 = x;
        gx2 = x + goalW; 
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    // left post
    glBegin(GL_QUADS);
        glVertex2f(x - postT/2, goalY1);
        glVertex2f(x + postT/2, goalY1);
        glVertex2f(x + postT/2, goalY2);
        glVertex2f(x - postT/2, goalY2);
    glEnd();

    // crossbar
    glBegin(GL_QUADS);
        glVertex2f(x - postT/2, goalY2 - crossT/2);
        glVertex2f(x + (isLeft ? -goalW : goalW) + postT/2, goalY2 - crossT/2);
        glVertex2f(x + (isLeft ? -goalW : goalW) + postT/2, goalY2 + crossT/2);
        glVertex2f(x - postT/2, goalY2 + crossT/2);
    glEnd();

    float backX = isLeft ? gx1 : gx2;
    glBegin(GL_QUADS);
        glVertex2f(backX - postT/2, goalY1);
        glVertex2f(backX + postT/2, goalY1);
        glVertex2f(backX + postT/2, goalY2);
        glVertex2f(backX - postT/2, goalY2);
    glEnd();

    glColor3f(0.85f, 0.85f, 0.88f);
    glLineWidth(1.0f);

    int netCols = 5;
    int netRows = 5;

    glBegin(GL_LINES);
    for(int i=0;i<=netCols;i++){
        float nx = gx1 + i * ((gx2-gx1)/netCols);
        glVertex2f(nx, goalY1);
        glVertex2f(nx, goalY2);
    }
    glEnd();

    glBegin(GL_LINES);
    for(int j=0;j<=netRows;j++){
        float ny = goalY1 + j * (goalH/netRows);
        glVertex2f(gx1, ny);
        glVertex2f(gx2, ny);
    }
    glEnd();
}


//======================Draw Football====================
void drawFootball(float cx, float cy, float r)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);

    glColor3f(0.98f, 0.98f, 0.98f);
    midpointCircle(0, 0, (int)r, false, true);

    glColor3f(0.05f, 0.05f, 0.05f);
    glPointSize(2.0f);
    midpointCircle(0, 0, (int)r, false, false);

    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_POLYGON);
        glVertex2f(0.0f,   r*0.35f);
        glVertex2f(-r*0.30f, r*0.10f);
        glVertex2f(-r*0.18f,-r*0.30f);
        glVertex2f( r*0.18f,-r*0.30f);
        glVertex2f( r*0.30f, r*0.10f);
    glEnd();

    glColor3f(0.15f, 0.15f, 0.15f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
        glVertex2f(0, r*0.35f);     glVertex2f(0, r*0.95f);
        glVertex2f(-r*0.30f, r*0.10f); glVertex2f(-r*0.75f, r*0.45f);
        glVertex2f( r*0.30f, r*0.10f); glVertex2f( r*0.75f, r*0.45f);

        glVertex2f(-r*0.18f,-r*0.30f); glVertex2f(-r*0.65f,-r*0.55f);
        glVertex2f( r*0.18f,-r*0.30f); glVertex2f( r*0.65f,-r*0.55f);
    glEnd();

    glPopMatrix();
}




//===================================Central Field========================

void drawCentralField(float x1, float x2, float y1, float y2)
{

    glColor3f(0.12f, 0.60f, 0.20f); 
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    float stripeH = (y2 - y1) / 6.0f;
    for(int i=0;i<6;i++){
        if(i%2==0) glColor3f(0.15f, 0.70f, 0.25f);
        else       glColor3f(0.10f, 0.55f, 0.18f);

        float sy1 = y1 + i*stripeH;
        float sy2 = sy1 + stripeH;

        glBegin(GL_QUADS);
            glVertex2f(x1, sy1);
            glVertex2f(x2, sy1);
            glVertex2f(x2, sy2);
            glVertex2f(x1, sy2);
        glEnd();
    }

    glColor3f(1,1,1);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    float midY = (y1+y2)*0.5f;
    glBegin(GL_LINES);
        glVertex2f(x1, midY);
        glVertex2f(x2, midY);
    glEnd();

    float cx = (x1+x2)*0.5f;
    float cy = midY;
    float r  = (x2-x1)*0.06f;   

    glColor3f(0.12f, 0.60f, 0.20f);  
    midpointCircle((int)cx, (int)cy, (int)r, false, true);

    glColor3f(1,1,1);
    glPointSize(2.0f);
    midpointCircle((int)cx, (int)cy, (int)r, false, false);
     drawFootball(cx, cy, r * 0.35f);
}


//========================Draw Grass================
void drawGrassPatch()
{

    glColor3f(0.08f, 0.55f, 0.18f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(0, 18);
        glVertex2f(0, 0);
        glVertex2f(-6, 14);
        glVertex2f(0, 0);
        glVertex2f(6, 14);
        glVertex2f(-2, 0);
        glVertex2f(-9, 10);

        glVertex2f(2, 0);
        glVertex2f(9, 10);
    glEnd();
}


void placeGrassPatches(float x1, float x2, float y1, float y2)
{

    for(float gx = x1 + 20; gx < x2; gx += 60)
    {
        glPushMatrix();
            glTranslatef(gx, y1 + 8, 0);
            float s1 = 0.8f + (rand()%30)/100.0f;  // 0.8..1.1
            glScalef(s1, s1, 1);
            drawGrassPatch();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(gx + 25, y1 + 25, 0);
            float s2 = 0.6f + (rand()%25)/100.0f;  // 0.6..0.85
            glScalef(s2, s2, 1);
            drawGrassPatch();
        glPopMatrix();
    }
}


//==============================Helipad====================================
void drawHelipad(float cx, float cy, float r, float scale = 1.0f)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glScalef(scale, scale, 1.0f);

    float R = r;
    glColor3f(0.45f, 0.45f, 0.48f);
    midpointCircle(0, 0, (int)R, false, true);

    glColor3f(1,1,1);
    glPointSize(2.0f);
    midpointCircle(0, 0, (int)R, false, false);
    glPointSize(2.0f);
    midpointCircle(0, 0, (int)(R*0.75f), false, false);
    glColor3f(1,1,1);
    float hH = R*0.9f;   
    float hW = R*0.7f;   
    float thick = 4.0f;

    thickLine(-hW*0.5f, -hH*0.5f,  -hW*0.5f,  hH*0.5f, thick);
    thickLine( hW*0.5f, -hH*0.5f,   hW*0.5f,  hH*0.5f, thick);
    thickLine(-hW*0.5f, 0,  hW*0.5f, 0, thick);

    glPopMatrix();
}


// =============================Helicopter object =======================
void drawHelicopterReal(float x, float y, float scale, float rotorAngle)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // base body
    glColor3f(0.20f, 0.22f, 0.25f); 
    glBegin(GL_QUADS);
        glVertex2f(-75, -14);
        glVertex2f( 40, -14);
        glVertex2f( 40,  18);
        glVertex2f(-75,  18);
    glEnd();

    glColor3f(0.26f, 0.28f, 0.31f);
    midpointCircle(40, 2, 19, false, true);

    glColor3f(0.18f, 0.20f, 0.23f);
    midpointCircle(-75, 2, 16, false, true);

    glColor3f(0.32f, 0.35f, 0.38f);
    glBegin(GL_QUADS);
        glVertex2f(-70, 8);
        glVertex2f( 30, 8);
        glVertex2f( 25, 18);
        glVertex2f(-65, 18);
    glEnd();

    glColor3f(0.12f, 0.13f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(-70, -14);
        glVertex2f( 30, -14);
        glVertex2f( 25,  -7);
        glVertex2f(-65,  -7);
    glEnd();

    // ENGINE HUMP
    glColor3f(0.30f, 0.32f, 0.35f);
    glBegin(GL_QUADS);
        glVertex2f(-25, 18);
        glVertex2f( 18, 18);
        glVertex2f( 10, 36);
        glVertex2f(-32, 36);
    glEnd();

    glColor3f(0.10f,0.10f,0.12f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        for(int i=0;i<4;i++){
            glVertex2f(-20 + i*8, 28);
            glVertex2f(-16 + i*8, 33);
        }
    glEnd();

    // COCKPIT GLASS
    glColor3f(0.70f, 0.90f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f( 2,  0);
        glVertex2f(35,  0);
        glVertex2f(38, 16);
        glVertex2f( 5, 16);
    glEnd();
    glColor3f(0.75f, 0.92f, 1.0f);
    midpointCircle(20, 14, 12, true, true); 
    glColor3f(0.08f,0.08f,0.10f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f( 2,  0);
        glVertex2f(35,  0);
        glVertex2f(38, 16);
        glVertex2f( 5, 16);
    glEnd();

    glBegin(GL_LINES);
        glVertex2f( 5,  0);
        glVertex2f(12, 16);
    glEnd();

    glColor3f(0.72f, 0.88f, 0.98f);
    glBegin(GL_QUADS);
        glVertex2f(-18,  2);
        glVertex2f( -2,  2);
        glVertex2f( -2, 10);
        glVertex2f(-18, 10);
    glEnd();

    // DOOR + STRIPE
    glColor3f(0.17f,0.18f,0.20f);
    glBegin(GL_QUADS);
        glVertex2f(-8, -10);
        glVertex2f(10, -10);
        glVertex2f(10,  10);
        glVertex2f(-8,  10);
    glEnd();

    glColor3f(0.05f,0.05f,0.05f);
    glBegin(GL_LINES);
        glVertex2f(1, -10);
        glVertex2f(1,  10);
    glEnd();

    glColor3f(0.85f, 0.20f, 0.20f); 
    glBegin(GL_QUADS);
        glVertex2f(-55, -2);
        glVertex2f( 20, -2);
        glVertex2f( 18,  2);
        glVertex2f(-53,  2);
    glEnd();

    // TAIL BOOM 
    glColor3f(0.20f, 0.22f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(-90,  6);
        glVertex2f(-150, 4);
        glVertex2f(-150, 10);
        glVertex2f(-90, 12);
    glEnd();

    glColor3f(0.25f,0.27f,0.30f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-150, 10);
        glVertex2f(-165, 10);
        glVertex2f(-150, 26);
    glEnd();

    glColor3f(0.05f,0.05f,0.06f);
    midpointCircle(-160, 14, 4, false, true);

    glColor3f(0.92f,0.92f,0.92f);
    thickLine(-160, 14, -176, 14, 2.0f);
    thickLine(-160, 14, -160, 30, 2.0f);

    // LANDING SKIDS
    glColor3f(0.05f,0.05f,0.05f);

    thickLine(-50, -22, 38, -22, 3.0f);
    thickLine(-45, -27, 42, -27, 3.0f);

    thickLine(-32, -14, -38, -28, 2.5f);
    thickLine( 25, -14,  30, -28, 2.5f);

    thickLine(-10, -14, -15, -28, 2.5f);
    thickLine(  5, -14,  10, -28, 2.5f);

    //MAST
    glColor3f(0.07f,0.07f,0.08f);
    thickLine(-2, 36, -2, 50, 3.0f);

    // MAIN ROTOR (4 blades)
    glPushMatrix();
        glTranslatef(-2, 50, 0);
        glRotatef(rotorAngle, 0,0,1);

        glColor3f(0.02f,0.02f,0.02f);
        thickLine(-95,  0,  95,  0, 3.2f);
        thickLine(  0, -95,   0, 95, 3.2f);

        // hub
        glColor3f(0.18f,0.18f,0.18f);
        midpointCircle(0, 0, 6, false, true);
        glColor3f(0.05f,0.05f,0.05f);
        midpointCircle(0, 0, 6, false, false);
    glPopMatrix();

    glPopMatrix();
}



//==================== Helicopter animation controller ================
void animateHelicopter()
{
    float fieldX2 = 800.0f;
    float fieldY1 = 10.0f;
    float fieldY2 = 140.0f;

    float padCx = fieldX2 + 180.0f;             
    float padCy = (fieldY1 + fieldY2) * 0.5f;  

    static float hx = -160.0f;  
    static float hy = HEIGHT + 120.0f;
    static float rotorAngle = 0.0f;

    // rotor spin
    if(heliState != HELI_HIDDEN){
        rotorAngle += 8.0f;         
        if(rotorAngle > 360.0f) rotorAngle -= 360.0f;
    }

    // STATES
    if(heliState == HELI_ENTERING){
        float targetX = padCx;
        float targetY = padCy + 120.0f; 

        hx += (targetX - hx) * 0.005f; 
        hy += (targetY - hy) * 0.005f;

        if(fabs(hx-targetX) < 2.5f && fabs(hy-targetY) < 2.5f){
            heliState = HELI_LANDING;
        }
    }
    else if(heliState == HELI_LANDING){
        float landY = padCy + 35.0f;    

        hy += (landY - hy) * 0.015f;   

        if(fabs(hy - landY) < 1.0f){
            heliState = HELI_LANDED;
        }
    }
    else if(heliState == HELI_LANDED){
        hy += sinf(rotorAngle * 0.02f) * 0.05f;
    }
    else if(heliState == HELI_TAKEOFF_UP){
        float takeoffY = padCy + 140.0f; 

        hy += (takeoffY - hy) * 0.010f;

        if(fabs(hy - takeoffY) < 2.0f){
            heliState = HELI_EXITING;
        }
    }
    else if(heliState == HELI_EXITING){
        float targetX = WIDTH + 170.0f;
        float targetY = HEIGHT + 130.0f;

        hx += (targetX - hx) * 0.005f; 
        hy += (targetY - hy) * 0.005f;

        if(hx > WIDTH + 140.0f && hy > HEIGHT + 100.0f){
            heliState = HELI_HIDDEN;
            heliShow  = false;
        }
    }

    if(heliState != HELI_HIDDEN){
        drawHelicopterReal(hx, hy, 0.85f, rotorAngle);
    }
}




void keyboard(unsigned char key, int x, int y)
{
    if(key=='h' || key=='H'){
        if(heliState == HELI_HIDDEN){
            heliShow  = true;
            heliState = HELI_ENTERING;
        }
        else if(heliState == HELI_LANDED){
            heliState = HELI_TAKEOFF_UP;
        }
    }

    glutPostRedisplay();
}
//=====================================================================



//============================Ground====================


void drawGroundAndRoad()
{
    float grassTop = 150.0f;

    glColor3f(0.3f, 0.7f, 0.3f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(WIDTH, 0.0f);
        glVertex2f(WIDTH, grassTop);
        glVertex2f(0.0f, grassTop);
    glEnd();

    placeGrassPatches(0.0f, WIDTH, 0.0f, grassTop);

    float roadY1 = grassTop;
    float roadY2 = grassTop + 60.0f;

    drawRoadSegment(0.0f, WIDTH, roadY1, roadY2);

    float vRoadW   = 310.0f;
    float vRoadLen = roadY2-120;
    float vXc      = WIDTH - 100.0f;

    glPushMatrix();
        glTranslatef(vXc, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        drawRoadSegment(-vRoadW * 0.5f, vRoadW * 0.5f, 0.0f, vRoadLen);
    glPopMatrix();

    float frontGreenY1 = roadY2;
    float frontGreenY2 = BASE_Y1;

    glColor3f(0.28f, 0.68f, 0.28f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, frontGreenY1);
        glVertex2f(WIDTH, frontGreenY1);
        glVertex2f(WIDTH, frontGreenY2);
        glVertex2f(0.0f, frontGreenY2);
    glEnd();

    placeGrassPatches(0.0f, WIDTH, frontGreenY1, frontGreenY2);

    float fieldX1 = 150.0f;
    float fieldX2 = 800.0f;

    float fieldY1 = 10.0f;
    float fieldY2 = 140.0f;

    drawCentralField(fieldX1, fieldX2, fieldY1, fieldY2);
    drawGoalPost(fieldX1, fieldY1, fieldY2, true); 
    drawGoalPost(fieldX2, fieldY1, fieldY2, false);

    //Helipad
    float padCx = fieldX2 + 180.0f;
    float padCy = (fieldY1 + fieldY2) * 0.5f;
    drawHelipad(padCx, padCy, 50.0f, 1.0f);
}



// ===================== Elevator (Main, big) =====================

void drawElevatorSection(int centerX, int baseY1, int baseY2, int floors)
{
    int floorHeight = baseY2 - baseY1;

    int bottom = baseY1 - 5;
    int top    = baseY1 + floors * floorHeight + 5;

    int radius = 30;
    int x1 = centerX - radius;
    int x2 = centerX + radius;

    // SHAFT BODY
    glColor3f(0.70f, 0.85f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2i(x1, bottom + radius);
        glVertex2i(x2, bottom + radius);
        glVertex2i(x2, top    - radius);
        glVertex2i(x1, top    - radius);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.80f, 0.90f, 1.0f);
        glVertex2i(x1,      bottom + radius);
        glVertex2i(centerX, bottom + radius);
        glVertex2i(centerX, top    - radius);
        glVertex2i(x1,      top    - radius);

        glColor3f(0.60f, 0.80f, 1.0f);
        glVertex2i(centerX, bottom + radius);
        glVertex2i(x2,      bottom + radius);
        glVertex2i(x2,      top    - radius);
        glVertex2i(centerX, top    - radius);
    glEnd();

    glColor3f(0.70f, 0.85f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(centerX, top - radius);
        for(int i=0;i<=40;i++){
            float ang = (float)M_PI * i / 40.0f;
            float xx = radius*cosf(ang);
            float yy = radius*sinf(ang);
            glVertex2f(centerX+xx, (top-radius)+yy);
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(centerX, bottom + radius);
        for(int i=0;i<=40;i++){
            float ang = (float)M_PI + (float)M_PI*i/40.0f;
            float xx = radius*cosf(ang);
            float yy = radius*sinf(ang);
            glVertex2f(centerX+xx, (bottom+radius)+yy);
        }
    glEnd();

    glColor3f(0.30f,0.35f,0.40f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2i(x1, bottom + radius);
        glVertex2i(x1, top    - radius);
        for(int i=0;i<=40;i++){
            float ang = (float)M_PI * i / 40.0f;
            float xx = radius*cosf(ang);
            float yy = radius*sinf(ang);
            glVertex2f(centerX+xx, (top-radius)+yy);
        }
        glVertex2i(x2, top - radius);
        glVertex2i(x2, bottom + radius);
        for(int i=0;i<=40;i++){
            float ang = (float)M_PI + (float)M_PI*i/40.0f;
            float xx = radius*cosf(ang);
            float yy = radius*sinf(ang);
            glVertex2f(centerX+xx, (bottom+radius)+yy);
        }
    glEnd();

    // center rod
    glColor3f(0.60f,0.70f,0.80f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2i(centerX, bottom + 8);
        glVertex2i(centerX, top    - 8);
    glEnd();

    //MOVING CABIN
    static float cabinT = 0.0f;
    cabinT += 0.0015f;
    if(cabinT > 2*M_PI) cabinT = 0.0f;

    float cabinW = radius * 1.10f;
    float cabinH = floorHeight * 1.4f;

    float cabinMinY = bottom + radius + 6;
    float cabinMaxY = top - radius - cabinH - 6;

    float u = (sinf(cabinT)+1.0f)/2.0f;
    float cabinY = cabinMinY + (cabinMaxY - cabinMinY)*u;

    float cX1 = centerX - cabinW/2;
    float cX2 = centerX + cabinW/2;
    float cY1 = cabinY;
    float cY2 = cabinY + cabinH;

    // cabin body
    glColor3f(0.92f,0.92f,0.94f);
    glBegin(GL_QUADS);
        glVertex2f(cX1,cY1); glVertex2f(cX2,cY1);
        glVertex2f(cX2,cY2); glVertex2f(cX1,cY2);
    glEnd();

    // cabin glass
    glColor3f(0.72f,0.86f,1.0f);
    glBegin(GL_QUADS);
        glVertex2f(cX1+4, cY1+6);
        glVertex2f(cX2-4, cY1+6);
        glVertex2f(cX2-4, cY2-6);
        glVertex2f(cX1+4, cY2-6);
    glEnd();

    // cabin border
    glColor3f(0.25f,0.25f,0.30f);
    glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cX1,cY1); glVertex2f(cX2,cY1);
        glVertex2f(cX2,cY2); glVertex2f(cX1,cY2);
    glEnd();

    glColor3f(0.75f,0.75f,0.78f);
    glBegin(GL_QUADS);
        glVertex2f(cX1, cY1);
        glVertex2f(cX2, cY1);
        glVertex2f(cX2, cY1+3);
        glVertex2f(cX1, cY1+3);
    glEnd();

    // FLOOR SHADOW SLOTS
    float innerL = centerX - (radius - 9);
    float innerR = centerX + (radius - 9);

    for(int i=0;i<floors;i++){
        float yFloor1 = baseY1 + i*floorHeight;
        float yFloor2 = yFloor1 + floorHeight;

        glColor3f(0.55f, 0.65f, 0.75f);
        glBegin(GL_QUADS);
            glVertex2f(innerL, yFloor2 - 6);
            glVertex2f(innerR, yFloor2 - 6);
            glVertex2f(innerR, yFloor2 - 1);
            glVertex2f(innerL, yFloor2 - 1);
        glEnd();

        glColor3f(0.65f, 0.75f, 0.85f);
        glBegin(GL_QUADS);
            glVertex2f(innerL, yFloor1 + floorHeight*0.45f);
            glVertex2f(innerR, yFloor1 + floorHeight*0.45f);
            glVertex2f(innerR, yFloor1 + floorHeight*0.50f);
            glVertex2f(innerL, yFloor1 + floorHeight*0.50f);
        glEnd();
    }
}



// ===================== Small Elevator (for central wing only) =====================

void drawSmallElevatorSection(int centerX, int baseY1, int baseY2, int floors)
{
    int floorHeight = baseY2 - baseY1;

    int bottom = baseY1 - 3;
    int top    = baseY1 + floors * floorHeight + 3;

    int radius = 20;
    int x1 = centerX - radius;
    int x2 = centerX + radius;

    glColor3f(0.70f, 0.85f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2i(x1, bottom + radius);
        glVertex2i(x2, bottom + radius);
        glVertex2i(x2, top    - radius);
        glVertex2i(x1, top    - radius);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.80f, 0.90f, 1.0f);
        glVertex2i(x1,      bottom + radius);
        glVertex2i(centerX, bottom + radius);
        glVertex2i(centerX, top    - radius);
        glVertex2i(x1,      top    - radius);

        glColor3f(0.60f, 0.80f, 1.0f);
        glVertex2i(centerX, bottom + radius);
        glVertex2i(x2,      bottom + radius);
        glVertex2i(x2,      top    - radius);
        glVertex2i(centerX, top    - radius);
    glEnd();

    glColor3f(0.70f,0.85f,1.0f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(centerX, top-radius);
        for(int i=0;i<=40;i++){
            float ang=(float)M_PI*i/40.0f;
            glVertex2f(centerX+radius*cosf(ang),(top-radius)+radius*sinf(ang));
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(centerX, bottom+radius);
        for(int i=0;i<=40;i++){
            float ang=(float)M_PI+(float)M_PI*i/40.0f;
            glVertex2f(centerX+radius*cosf(ang),(bottom+radius)+radius*sinf(ang));
        }
    glEnd();

    // outline
    glColor3f(0.30f,0.35f,0.40f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_STRIP);
        glVertex2i(x1, bottom+radius);
        glVertex2i(x1, top-radius);
        for(int i=0;i<=40;i++){
            float ang=(float)M_PI*i/40.0f;
            glVertex2f(centerX+radius*cosf(ang),(top-radius)+radius*sinf(ang));
        }
        glVertex2i(x2, top-radius);
        glVertex2i(x2, bottom+radius);
        for(int i=0;i<=40;i++){
            float ang=(float)M_PI+(float)M_PI*i/40.0f;
            glVertex2f(centerX+radius*cosf(ang),(bottom+radius)+radius*sinf(ang));
        }
    glEnd();

    //MOVING CABIN
    static float cabinT = 0.0f;
    cabinT += 0.0030f;
    if(cabinT > 2*M_PI) cabinT = 0.0f;

    float cabinW = radius * 0.95f;
    float cabinH = floorHeight * 1.0f;

    float cabinMinY = bottom + radius + 4;
    float cabinMaxY = top - radius - cabinH - 4;

    float u = (sinf(cabinT)+1.0f)/2.0f;
    float cabinY = cabinMinY + (cabinMaxY-cabinMinY)*u;

    float cX1 = centerX - cabinW/2;
    float cX2 = centerX + cabinW/2;
    float cY1 = cabinY;
    float cY2 = cabinY + cabinH;

    glColor3f(0.93f,0.93f,0.95f);
    glBegin(GL_QUADS);
        glVertex2f(cX1,cY1); glVertex2f(cX2,cY1);
        glVertex2f(cX2,cY2); glVertex2f(cX1,cY2);
    glEnd();

    glColor3f(0.72f,0.86f,1.0f);
    glBegin(GL_QUADS);
        glVertex2f(cX1+3,cY1+4);
        glVertex2f(cX2-3,cY1+4);
        glVertex2f(cX2-3,cY2-4);
        glVertex2f(cX1+3,cY2-4);
    glEnd();

    glColor3f(0.25f,0.25f,0.30f);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cX1,cY1); glVertex2f(cX2,cY1);
        glVertex2f(cX2,cY2); glVertex2f(cX1,cY2);
    glEnd();

    glColor3f(0.78f,0.78f,0.80f);
    glBegin(GL_QUADS);
        glVertex2f(cX1,cY1);
        glVertex2f(cX2,cY1);
        glVertex2f(cX2,cY1+2);
        glVertex2f(cX1,cY1+2);
    glEnd();

    // FLOOR SHADOWS (6 floors)
    float innerL = centerX - (radius - 6);
    float innerR = centerX + (radius - 6);

    for(int i=0;i<floors;i++){
        float yFloor1 = baseY1 + i*floorHeight;
        float yFloor2 = yFloor1 + floorHeight;

        glColor3f(0.55f,0.65f,0.75f);
        glBegin(GL_QUADS);
            glVertex2f(innerL, yFloor2-4);
            glVertex2f(innerR, yFloor2-4);
            glVertex2f(innerR, yFloor2-1);
            glVertex2f(innerL, yFloor2-1);
        glEnd();

        glColor3f(0.65f,0.75f,0.85f);
        glBegin(GL_QUADS);
            glVertex2f(innerL, yFloor1 + floorHeight*0.45f);
            glVertex2f(innerR, yFloor1 + floorHeight*0.45f);
            glVertex2f(innerR, yFloor1 + floorHeight*0.50f);
            glVertex2f(innerL, yFloor1 + floorHeight*0.50f);
        glEnd();
    }
}



// ===================== Roof =====================

void drawRoofSegment(int x1, int x2, int roofY) {
    int roofHeight = 10;
    int overhang   = 6;

    int rx1 = x1 - overhang;
    int rx2 = x2 + overhang;
    int ry1 = roofY;
    int ry2 = roofY + roofHeight;

    glColor3f(0.75f, 0.75f, 0.78f);
    glBegin(GL_QUADS);
        glVertex2i(rx1, ry1);
        glVertex2i(rx2, ry1);
        glVertex2i(rx2, ry2);
        glVertex2i(rx1, ry2);
    glEnd();

    glColor3f(0.9f, 0.9f, 0.94f);
    glBegin(GL_QUADS);
        glVertex2i(rx1, ry2 - 3);
        glVertex2i(rx2, ry2 - 3);
        glVertex2i(rx2, ry2);
        glVertex2i(rx1, ry2);
    glEnd();

    glColor3f(0.3f, 0.3f, 0.35f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2i(rx1, ry1);
        glVertex2i(rx2, ry1);
        glVertex2i(rx2, ry2);
        glVertex2i(rx1, ry2);
    glEnd();
}

// ===================== Top Glass Floor =====================

void drawTopGlassFloor(int x1, int x2, int baseY1, int baseY2, int floorsTotal) {
    int floorHeight = baseY2 - baseY1;

    int topIndex = floorsTotal - 1;

    int y1 = baseY1 + topIndex * floorHeight;
    int y2 = baseY2 + topIndex * floorHeight;

    glColor3f(0.85f, 0.9f, 0.98f);
    glBegin(GL_QUADS);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y2);
    glEnd();

    glColor3f(0.7f, 0.8f, 0.95f);
    int bandHeight = 10;
    glBegin(GL_QUADS);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y1 + bandHeight);
        glVertex2i(x1, y1 + bandHeight);
    glEnd();

    glColor3f(0.4f, 0.45f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2i(x1, y2);
        glVertex2i(x2, y2);
    glEnd();

    glColor3f(0.75f, 0.82f, 0.9f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        int sections = 6;
        for (int i = 1; i < sections; i++) {
            int px = x1 + i * ((x2 - x1) / sections);
            glVertex2i(px, y1 + 3);
            glVertex2i(px, y2 - 3);
        }
    glEnd();
}

// ===================== Middle Core =====================

void drawMiddleCore(int x1, int x2, int baseY1, int baseY2, int floors) {
    int floorHeight = baseY2 - baseY1;

    for (int i = 0; i < floors; i++) {
        int y1 = baseY1 + i * floorHeight;
        int y2 = baseY2 + i * floorHeight;

        glColor3f(0.45f, 0.45f, 0.5f);
        glBegin(GL_QUADS);
            glVertex2i(x1, y1);
            glVertex2i(x2, y1);
            glVertex2i(x2, y2);
            glVertex2i(x1, y2);
        glEnd();

        glColor3f(0.7f, 0.8f, 0.9f);
        int w1 = x1 + 3;
        int w2 = x2 - 3;
        int wy1 = y1 + 5;
        int wy2 = y2 - 5;
        glBegin(GL_QUADS);
            glVertex2i(w1, wy1);
            glVertex2i(w2, wy1);
            glVertex2i(w2, wy2);
            glVertex2i(w1, wy2);
        glEnd();
    }
}


// ===================== Small Plant / Tree =====================

void drawSmallPlant(float x, float y, float scale = 1.0f)
{

    float potW   = 8.0f * scale;
    float potH   = 5.0f * scale;
    float trunkW = 3.0f * scale;
    float trunkH = 8.0f * scale;

    float potX1 = x - potW * 0.5f;
    float potX2 = x + potW * 0.5f;
    float potY1 = y;
    float potY2 = y + potH;

    glColor3f(0.55f, 0.27f, 0.07f);  
    glBegin(GL_QUADS);
        glVertex2f(potX1, potY1);
        glVertex2f(potX2, potY1);
        glVertex2f(potX2, potY2);
        glVertex2f(potX1, potY2);
    glEnd();

    glColor3f(0.35f, 0.18f, 0.05f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(potX1, potY1);
        glVertex2f(potX2, potY1);
        glVertex2f(potX2, potY2);
        glVertex2f(potX1, potY2);
    glEnd();

    float trunkX1 = x - trunkW * 0.5f;
    float trunkX2 = x + trunkW * 0.5f;
    float trunkY1 = potY2;
    float trunkY2 = potY2 + trunkH;

    glColor3f(0.40f, 0.23f, 0.10f);
    glBegin(GL_QUADS);
        glVertex2f(trunkX1, trunkY1);
        glVertex2f(trunkX2, trunkY1);
        glVertex2f(trunkX2, trunkY2);
        glVertex2f(trunkX1, trunkY2);
    glEnd();

    float baseY  = trunkY2;
    float h1     = 14.0f * scale;  
    float w1     = 18.0f * scale;   
    float h2     = 10.0f * scale;  
    float w2     = 12.0f * scale;

    glColor3f(0.0f, 0.6f, 0.2f);   
    glBegin(GL_TRIANGLES);
        glVertex2f(x,          baseY + h1);  
        glVertex2f(x - w1/2.0f, baseY);       
        glVertex2f(x + w1/2.0f, baseY);       
    glEnd();

    glColor3f(0.0f, 0.7f, 0.25f);
    float baseY2 = baseY + h1 * 0.4f;
    glBegin(GL_TRIANGLES);
        glVertex2f(x,          baseY2 + h2);
        glVertex2f(x - w2/2.0f, baseY2);
        glVertex2f(x + w2/2.0f, baseY2);
    glEnd();

    glColor3f(0.6f, 0.9f, 0.6f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glVertex2f(x - w2*0.15f, baseY2 + h2*0.7f);
        glVertex2f(x,            baseY2 + h2*0.95f);
    glEnd();
}


//===============================CAR 2D================================

void drawCar2D(float x, float y, float scale,
               float bodyR, float bodyG, float bodyB,
               bool faceRight = true)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    if(!faceRight) glScalef(-1, 1, 1);

    float carW = 70;
    float carH = 25;

    //body
    glColor3f(bodyR, bodyG, bodyB);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(carW, 0);
        glVertex2f(carW, carH);
        glVertex2f(0, carH);
    glEnd();

    //top roof
    glColor3f(bodyR*0.9f, bodyG*0.9f, bodyB*0.9f);
    glBegin(GL_QUADS);
        glVertex2f(12, carH);
        glVertex2f(carW-12, carH);
        glVertex2f(carW-20, carH+12);
        glVertex2f(20, carH+12);
    glEnd();

    //  windows
    glColor3f(0.75f, 0.90f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(20, carH+2);
        glVertex2f(carW-20, carH+2);
        glVertex2f(carW-26, carH+10);
        glVertex2f(26, carH+10);
    glEnd();

    //  outline 
    glColor3f(0.05f,0.05f,0.05f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0, 0);
        glVertex2f(carW, 0);
        glVertex2f(carW, carH);
        glVertex2f(0, carH);
    glEnd();

    // heels
    float wheelR = 6;
    float wY = -2;

    glColor3f(0,0,0);
    midpointCircle(15, (int)wY, (int)wheelR, false, true);
    midpointCircle(carW-15, (int)wY, (int)wheelR, false, true);

    glColor3f(0.6f,0.6f,0.6f);
    midpointCircle(15, (int)wY, (int)(wheelR*0.5f), false, true);
    midpointCircle(carW-15, (int)wY, (int)(wheelR*0.5f), false, true);

    glPopMatrix();
}


void drawParkingArea(float x1, float x2, float y1, float y2)
{
    //Parking floor
    glColor3f(0.25f, 0.25f, 0.28f);
    glBegin(GL_QUADS);
        glVertex2f(x1,y1);
        glVertex2f(x2,y1);
        glVertex2f(x2,y2);
        glVertex2f(x1,y2);
    glEnd();

    glColor3f(0.30f, 0.30f, 0.33f);
    glBegin(GL_QUADS);
        glVertex2f(x1, y2-8);
        glVertex2f(x2, y2-8);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    glColor3f(0.95f,0.95f,0.95f);
    glLineWidth(2.0f);

    int slots = 5;
    float slotW = (x2-x1)/slots;

    glBegin(GL_LINES);
    for(int i=1;i<slots;i++){
        float sx = x1 + i*slotW;
        glVertex2f(sx, y1+5);
        glVertex2f(sx, y2-5);
    }
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(x1+4, y1+4); glVertex2f(x2-4, y1+4);
        glVertex2f(x1+4, y2-4); glVertex2f(x2-4, y2-4);
    glEnd();

    float carY = y1 + 8;

    drawCar2D(x1 + slotW*0.10f, carY, 0.9f, 0.85f,0.15f,0.15f);
    drawCar2D(x1 + slotW*1.10f, carY, 0.9f, 0.15f,0.35f,0.85f);
    drawCar2D(x1 + slotW*2.10f, carY, 0.9f, 0.90f,0.80f,0.20f);
    drawCar2D(x1 + slotW*3.10f, carY, 0.9f, 0.70f,0.72f,0.75f);
    drawCar2D(x1 + slotW*4.10f, carY, 0.9f, 0.10f,0.65f,0.55f);
}

//=========================================================================




// ===================== Brick Pattern Helper =====================

void drawBrickPattern(int x1, int x2, int y1, int y2, bool isFront) {
    if (y2 <= y1 || x2 <= x1) return;

    int brickHeight = 4;
    int brickWidth  = isFront ? 16 : 12;

    glColor3f(0.80f, 0.73f, 0.67f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    for (int y = y1; y <= y2; y += brickHeight) {
        glVertex2i(x1, y);
        glVertex2i(x2, y);
    }
    glEnd();

    bool offset = false;
    glBegin(GL_LINES);
    for (int y = y1; y < y2; y += brickHeight) {
        int startX = x1 + (offset ? brickWidth / 2 : 0);
        for (int x = startX; x < x2; x += brickWidth) {
            glVertex2i(x, y);
            int yTop = y + brickHeight;
            if (yTop > y2) yTop = y2;
            glVertex2i(x, yTop);
        }
        offset = !offset;
    }
    glEnd();
}


// ===================== One Floor of Building Strip =====================
void drawOneStripFloor(int x1, int x2, int baseY1, int baseY2, bool isFront, int floorIndex)
{
    float wallLight   = isFront ? 0.92f : 0.85f;
    float backBand    = isFront ? 0.65f : 0.55f;
    int   balOffsetX  = isFront ? 14   : 8;
    int   slabThickness  = isFront ? 4 : 3;
    int   railingHeight  = isFront ? 12 : 9;
    int   backBandHeight = 10;

    int y1 = baseY1;
    int y2 = baseY2;

    // Main wall
    glColor3f(wallLight, wallLight, wallLight);
    glBegin(GL_QUADS);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y2);
    glEnd();

    // Top back band
    glColor3f(backBand, backBand, backBand);
    glBegin(GL_QUADS);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y1 + backBandHeight);
        glVertex2i(x1, y1 + backBandHeight);
    glEnd();

    // Brick pattern
    int brickY1 = y1 + backBandHeight + 1;
    int brickY2 = y2 - 1;
    drawBrickPattern(x1 + 1, x2 - 1, brickY1, brickY2, isFront);

    //Wall outline
    glColor3f(0.3f, 0.3f, 0.3f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y2);
    glEnd();

    // Balcony slab
    int balX1 = x1 - balOffsetX;
    int balX2 = x2 + balOffsetX;
    int slabY1 = y1 - slabThickness;
    int slabY2 = y1;

    glColor3f(0.7f, 0.7f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2i(balX1, slabY1);
        glVertex2i(balX2, slabY1);
        glVertex2i(balX2, slabY2);
        glVertex2i(balX1, slabY2);
    glEnd();

    glColor3f(0.4f, 0.4f, 0.45f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2i(balX1, slabY1);
        glVertex2i(balX2, slabY1);
        glVertex2i(balX2, slabY2);
        glVertex2i(balX1, slabY2);
    glEnd();

    // Balcony glass railing
    int glassY1 = slabY2;
    int glassY2 = glassY1 + railingHeight;

    glColor3f(0.65f, 0.82f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2i(balX1, glassY1);
        glVertex2i(balX2, glassY1);
        glVertex2i(balX2, glassY2);
        glVertex2i(balX1, glassY2);
    glEnd();

    glColor3f(0.8f, 0.9f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2i(balX1, glassY2 - 4);
        glVertex2i(balX2, glassY2 - 4);
        glVertex2i(balX2, glassY2);
        glVertex2i(balX1, glassY2);
    glEnd();

    // Glass vertical panels 
    glColor3f(0.9f, 0.95f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        int panels = 3;
        for (int j = 1; j < panels; j++) {
            int px = balX1 + j * ((balX2 - balX1) / panels);
            glVertex2i(px, glassY1 + 2);
            glVertex2i(px, glassY2 - 2);
        }
    glEnd();

    // Railing top edge
    glColor3f(0.35f, 0.35f, 0.4f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2i(balX1, glassY2);
        glVertex2i(balX2, glassY2);
    glEnd();

    // Small plants 
    if (isFront && (floorIndex % 2 == 0)) {
        float plantBaseY = glassY1 + 2.0f;
        float plantX1 = balX1 + 12.0f;
        float plantX2 = balX2 - 12.0f;

        drawSmallPlant(plantX1, plantBaseY, 0.8f);
        drawSmallPlant(plantX2, plantBaseY, 0.8f);
    }
}


// ===================== Building Strip Using Translate =====================
void drawBuildingStrip(int x1, int x2, int baseY1, int baseY2, int floors, bool isFront)
{
    int floorHeight = baseY2 - baseY1;

    for (int i = 0; i < floors; i++)
    {
        glPushMatrix();
        glTranslatef(0.0f, i * floorHeight, 0.0f);

        drawOneStripFloor(x1, x2, baseY1, baseY2, isFront, i);

        glPopMatrix();
    }
}


// ===================== Right Wing (with name) =====================

void drawRightWing() {
    int baseY1 = BASE_Y1;
    int baseY2 = BASE_Y2;
    int floorsTotalRight      = 15;
    int floorsWithBalconRight = 14;
    int floorHeight = baseY2 - baseY1;

    int xA1 = 790;
    int xA2 = 910;
    int xCore1 = 910;
    int xCore2 = 930;
    int xB1 = 930;
    int xB2 = 1100;

    drawBuildingStrip(xA1,   xA2,   baseY1, baseY2, floorsWithBalconRight, false);
    drawMiddleCore   (xCore1, xCore2, baseY1, baseY2, floorsWithBalconRight);
    drawBuildingStrip(xB1,   xB2,   baseY1, baseY2, floorsWithBalconRight, true);

    drawTopGlassFloor(xB1, xB2, baseY1, baseY2, floorsTotalRight);

    int roofYBack  = baseY2 + (floorsWithBalconRight - 1) * floorHeight;
    int roofYFront = baseY2 + (floorsTotalRight      - 1) * floorHeight;

    drawRoofSegment(xA1, xA2, roofYBack);
    drawRoofSegment(xB1, xB2, roofYFront);

}


// ===================== Central Circular Wing (6 floors) =====================

void drawCentralWing() {
    int baseY1 = BASE_Y1;
    int baseY2 = BASE_Y2;
    int floorsCentral = 6;
    int floorHeight = baseY2 - baseY1;

    float cx     = 550.0f;
    float radius = 350.0f; 

    int segments = 48;

    for (int i = 0; i < floorsCentral; ++i) {
        float y1 = baseY1 + i * floorHeight;
        float y2 = baseY2 + i * floorHeight;

        glColor3f(0.88f, 0.93f, 0.98f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int k = 0; k <= segments; ++k) {
            float t = -M_PI / 2.0f + (M_PI * k) / segments; 
            float x = cx + radius * cosf(t);
            glVertex2f(x, y1);
            glVertex2f(x, y2);
        }
        glEnd();

        glColor3f(0.96f, 0.96f, 0.97f);
        float colAngles[] = { -1.4f, -0.9f, -0.45f, 0.0f, 0.45f, 0.9f, 1.4f };
        for (int c = 0; c < 7; ++c) {
            float t  = colAngles[c];
            float xC = cx + radius * cosf(t);
            float w  = 6.0f;
            glBegin(GL_QUADS);
                glVertex2f(xC - w/2, y1);
                glVertex2f(xC + w/2, y1);
                glVertex2f(xC + w/2, y2);
                glVertex2f(xC - w/2, y2);
            glEnd();
        }

        glColor3f(0.55f, 0.55f, 0.60f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for (int k = 0; k <= segments; ++k) {
            float t = -M_PI / 2.0f + (M_PI * k) / segments;
            float x = cx + radius * cosf(t);
            glVertex2f(x, y2);
        }
        glEnd();

        glColor3f(0.50f, 0.50f, 0.55f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
            glVertex2f(cx - radius * 0.95f, y1);
            glVertex2f(cx + radius * 0.95f, y1);
        glEnd();

        float balconyFactor = 0.30f;
        float frontX1 = cx - radius * balconyFactor + 110;
        float frontX2 = cx + radius;
        float slabY1  = y1 - 4.0f;
        float slabY2  = y1;

        glColor3f(0.72f, 0.72f, 0.76f);
        glBegin(GL_QUADS);
            glVertex2f(frontX1, slabY1);
            glVertex2f(frontX2, slabY1);
            glVertex2f(frontX2, slabY2);
            glVertex2f(frontX1, slabY2);
        glEnd();

        glColor3f(0.45f, 0.45f, 0.50f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(frontX1, slabY1);
            glVertex2f(frontX2, slabY1);
            glVertex2f(frontX2, slabY2);
            glVertex2f(frontX1, slabY2);
        glEnd();

        if (i > 0) {
            float railY1 = slabY2;
            float railY2 = railY1 + 10.0f;

            glColor3f(0.68f, 0.84f, 1.0f);
            glBegin(GL_QUADS);
                glVertex2f(frontX1, railY1);
                glVertex2f(frontX2, railY1);
                glVertex2f(frontX2, railY2);
                glVertex2f(frontX1, railY2);
            glEnd();

            glColor3f(0.85f, 0.93f, 1.0f);
            glBegin(GL_QUADS);
                glVertex2f(frontX1, railY2 - 3);
                glVertex2f(frontX2, railY2 - 3);
                glVertex2f(frontX2, railY2);
                glVertex2f(frontX1, railY2);
            glEnd();

            glColor3f(0.90f, 0.95f, 1.0f);
            glLineWidth(1.0f);
            glBegin(GL_LINES);
                int panels = 4;
                for (int j = 1; j < panels; ++j) {
                    float px = frontX1 + j * ((frontX2 - frontX1) / panels);
                    glVertex2f(px, railY1 + 2);
                    glVertex2f(px, railY2 - 2);
                }
            glEnd();

            glColor3f(0.40f, 0.40f, 0.45f);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
                glVertex2f(frontX1, railY2);
                glVertex2f(frontX2, railY2);
            glEnd();
        }
    }

    //CANOPY from 5th floor + rods from 2nd floor 

    int canopyFloorIndex = 3;
    float floor5_y1 = baseY1 + canopyFloorIndex * floorHeight;
    float floor5_y2 = baseY2 + canopyFloorIndex * floorHeight;

    float balconyFactor = 0.30f;
    float canFrontX1 = cx - radius * balconyFactor + 110;
    float canFrontX2 = cx + radius - 130;

    float canopyBackY  = floor5_y2;            
    float canopyFrontY = canopyBackY + 25.0f;  

    // canopy main slab
    glColor3f(0.6f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(canFrontX1, canopyBackY);
        glVertex2f(canFrontX2, canopyBackY);
        glVertex2f(canFrontX2, canopyFrontY);
        glVertex2f(canFrontX1, canopyFrontY);
    glEnd();

    // front edge
    glColor3f(0.50f, 0.50f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(canFrontX1, canopyFrontY);
        glVertex2f(canFrontX2, canopyFrontY);
    glEnd();

    // top highlight
    glColor3f(0.9f, 0.9f, 0.94f);
    glBegin(GL_LINES);
        glVertex2f(canFrontX1, canopyBackY + 1);
        glVertex2f(canFrontX2, canopyBackY + 1);
    glEnd();

    // Support rods: 2nd floor theke canopy front e
    int supportFloorIndex = 1; // 2nd floor
    float supportBaseY = baseY2 + supportFloorIndex * floorHeight; // top of 2nd
    float rodBottomY   = supportBaseY + 2.0f;
    float rodTopY      = canopyFrontY - 2.0f;

    float rodLeftX  = canFrontX1 + 20.0f;
    float rodRightX = canFrontX2 - 20.0f;

    glColor3f(0.35f, 0.35f, 0.40f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2f(rodLeftX,  rodBottomY);
        glVertex2f(rodLeftX,  rodTopY);

        glVertex2f(rodRightX, rodBottomY);
        glVertex2f(rodRightX, rodTopY);
    glEnd();

    glColor3f(0.7f, 0.7f, 0.75f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glVertex2f(rodLeftX,  rodBottomY);
        glVertex2f(rodLeftX,  rodBottomY + 8);

        glVertex2f(rodRightX, rodBottomY);
        glVertex2f(rodRightX, rodBottomY + 8);
    glEnd();

     // Front stairs only

    float entranceWidth = radius * 0.25f;        
    float entranceX1 = cx - entranceWidth+100;
    float entranceX2 = cx + entranceWidth+110;

    float entranceY1 = baseY1;  

    {
        int   steps      = 5;      
        float stepHeight = 4.0f;   
        float stepSpread = 6.0f;   

        for (int s = 0; s < steps; ++s) {
            float topY  = entranceY1 - s * stepHeight;
            float botY  = topY       - stepHeight;

            float leftX  = entranceX1 - (s + 1) * stepSpread;
            float rightX = entranceX2 + (s + 1) * stepSpread;

            // main step
            glColor3f(0.78f, 0.78f, 0.80f);
            glBegin(GL_QUADS);
                glVertex2f(leftX,  botY);
                glVertex2f(rightX, botY);
                glVertex2f(rightX, topY);
                glVertex2f(leftX,  topY);
            glEnd();

            // border
            glColor3f(0.55f, 0.55f, 0.58f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(leftX,  botY);
                glVertex2f(rightX, botY);
                glVertex2f(rightX, topY);
                glVertex2f(leftX,  topY);
            glEnd();
        }
    }
    //semi-circular terrace roof
    float topY       = baseY2 + (floorsCentral - 1) * floorHeight;
    float roofY1     = topY + 0.0f;
    float roofY2     = roofY1 + 18.0f;
    float roofRadius = radius * 1.05f;

    glColor3f(0.86f, 0.55f, 0.36f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int k = 0; k <= segments; ++k) {
        float t = -M_PI / 2.0f + (M_PI * k) / segments;
        float x = cx + roofRadius * cosf(t);
        glVertex2f(x, roofY1);
        glVertex2f(x, roofY2);
    }
    glEnd();

    glColor3f(0.55f, 0.32f, 0.22f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (int k = 0; k <= segments; ++k) {
        float t = -M_PI / 2.0f + (M_PI * k) / segments;
        float x = cx + roofRadius * cosf(t);
        glVertex2f(x, roofY2);
    }
    glEnd();

    //dome
    float domeRadius  = radius * 0.25f;
    float domeBaseY   = roofY2;
    float domeCenterX = cx + 170.0f;

    drawDomeMidpoint(domeCenterX, domeBaseY, domeRadius);

    int centralElevX = (int)(cx + radius * 0.7f);
    drawSmallElevatorSection(centralElevX, baseY1, baseY2, floorsCentral);
}


void animateBus()
{
    static float busX = -350.0f;         
    static float busY = 150.0f;          
    static float busSpeed = 0.5f;      

    busX += busSpeed;                 

    if (busX > WIDTH + 50.0f) {
        busX = -350.0f;
    }

    drawBus(busX, busY, 0.6f);          

}



//===========================DIU Colorr===========================

void getDIUTextColor(float t, float &r, float &g, float &b)
{
    if(t < 0) t = 0;
    if(t > 1) t = 1;

    float startFade = 0.65f; 
    float endFade   = 0.90f; 

    if(t <= startFade){
        r = g = b = 0.0f;
    }
    else if(t >= endFade){
        r = g = b = 1.0f;
    }
    else{
        float u = (t - startFade) / (endFade - startFade);
        u = u*u*(3.0f - 2.0f*u);

        r = g = b = u; 
    }
}



// ===================== Left Wing (main long building) =====================
void drawLeftWing(float t)
{
    int baseY1 = BASE_Y1;
    int baseY2 = BASE_Y2;

    int floorsTotal      = 14;
    int floorsWithBalcon = 13;

    int floorHeight  = baseY2 - baseY1;

    int x = 80;
    int stripW = 120;
    int coreW  = 20;
    int gapW   = 20;
    int lastCoreW = 30;

    drawBuildingStrip(x, x + stripW, baseY1, baseY2, floorsWithBalcon, false);
    x += stripW;

    drawMiddleCore(x, x + coreW, baseY1, baseY2, floorsWithBalcon);
    x += coreW;

    drawBuildingStrip(x, x + stripW, baseY1, baseY2, floorsWithBalcon, true);
    x += stripW;

    x += gapW;

    drawBuildingStrip(x, x + stripW, baseY1, baseY2, floorsWithBalcon, true);
    x += stripW;

    drawTopGlassFloor(220, 480, baseY1, baseY2, floorsTotal);
    drawElevatorSection(350, baseY1, baseY2, floorsTotal);

    drawMiddleCore(x, x + coreW, baseY1, baseY2, floorsWithBalcon);
    x += coreW;

    drawBuildingStrip(x, x + stripW, baseY1, baseY2, floorsWithBalcon, false);
    x += stripW;

    drawMiddleCore(x, x + coreW, baseY1, baseY2, floorsWithBalcon);
    x += coreW;

    drawBuildingStrip(x, x + stripW, baseY1, baseY2, floorsWithBalcon, false);
    x += stripW;

    drawMiddleCore(x, x + lastCoreW, baseY1, baseY2, floorsWithBalcon);
    x += lastCoreW;

    //roof segments for this wing 
    int leftRoofY    = baseY2 + (floorsWithBalcon - 1) * floorHeight;
    int centerRoofY  = baseY2 + (floorsTotal      - 1) * floorHeight;
    int rightRoofY   = leftRoofY;

    drawRoofSegment(80,  200, leftRoofY);
    drawRoofSegment(220, 480, centerRoofY);
    drawRoofSegment(500, 620, rightRoofY);
    drawRoofSegment(640, 760, rightRoofY);

    float tr,tg,tb;
    getDIUTextColor(t, tr,tg,tb);
    glColor3f(tr,tg,tb);
    drawTextBigStroke("DIU", 650, rightRoofY + 15, 0.5f);
}




// ===================== Display =====================
void display() {
    static float lastT = 0.0f;

    float sr, sg, sb;
    getSkyColor(lastT, sr, sg, sb);
    glClearColor(sr, sg, sb, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    lastT = drawSunMidpointAnimated();

    drawGroundAndRoad();

    animateBus();

    float t = drawSunMidpointAnimated();
    drawCloudAnimated(t);

    float mirrorCX = WIDTH * 0.5f;
    bool night = (t > 0.70f);
 

    drawTree(120,  BASE_Y1 - 100, 0.4f);
    drawTree(480,  BASE_Y1 - 100, 0.4f);
    drawTree(760,  BASE_Y1 - 100, 0.4f);
    drawTree(1040,  BASE_Y1 - 100, 0.4f);


    drawTree(WIDTH - 100, 150, 0.4f);
    drawTree(WIDTH - 60, 20, 0.7f);

    drawLeftWing(t);
    drawRightWing();
    drawCentralWing();

    //lamp
    drawLampPost(1190, 60, 0.7f, night);
    drawLampPost(50, 40, 0.7f, night);
    drawLampPostMirrorX(1150,230, 0.7f, night, mirrorCX);
    drawLampPostMirrorX(420,230, 0.7f, night, mirrorCX);

    drawTree(WIDTH-110,  BASE_Y1+40, 2.2f);
    drawTreePine(WIDTH-140,  BASE_Y1+40, 1.9f);
    drawParkingArea(1180, 1380, 250, 330);

    if(heliShow || heliState != HELI_HIDDEN){
    animateHelicopter();
    }


    drawTree(500,  BASE_Y1-5, 0.6f);

    drawTreePalm(75, BASE_Y1 - 30, 1.5f);
    drawTreePine(70,  BASE_Y1-20, 0.7f);

    drawTreePalm(WIDTH - 220, BASE_Y1 - 30, 1.5f);

    drawHumanRealisticSwing(630, BASE_Y1-30, 0.5f,   0.85f,0.20f,0.20f,   0.05f,0.05f,0.05f,0.0);
    drawHumanRealisticSwing(560, BASE_Y1-30, 0.5f,   0.10f,0.70f,0.25f,   0.20f,0.20f,0.55f,0.0);
    drawHumanWalkingV(0, WIDTH-120, 10, 120, 0.55f,0.10f,0.70f,0.25f,0.20f,0.20f,0.55f,0.35f, 0.8f, true);
    drawHumanWalkingV(1, WIDTH-160, 10, 120, 0.55f,  0.55f,0.25f,0.75f,   0.65f,0.65f,0.68f,  0.35f, 0.2f, false);


    glFlush();
}


void update(int value)
{
    glutPostRedisplay();         
    glutTimerFunc(16, update, 0);
}

// ===================== Init & main =====================

void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("OpenGL Test - Building Demo");

    init();
    glutDisplayFunc(display);
    glutIdleFunc([](){ glutPostRedisplay(); });
    glutTimerFunc(0, update, 0);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}