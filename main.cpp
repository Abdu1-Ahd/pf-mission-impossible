// Mission: Impossible - PF Project
// No OOP - global variables only
#include<iostream>
#include<fstream>
#include<cstring>
#include<cstdlib>
#include<ctime>
#include<windows.h>
#include<conio.h>
using namespace std;
#define WW 78
#define WH 27
#define NR 6
#define EW 3
#define EH 3
#define RW 3
#define RH 2
#define SW 5
#define SH 3
// sprite rows
const char* ES[]={" o ","/|\\","/ \\"};  // Ethan 3x3 - head centered

const char* RS[]={"[|]","[V]"};
const char* SS[]={" [ ] ","[|X|]"," [ ] "};
// map grid
char grid[WH][WW];
// ethan
int ex,ey,lives,score,level;
// laser
int lx,ly,ldx,ldy; bool lon;
int aimX,aimY;
// robots
int rx[NR],ry[NR]; bool ra[NR];
int rlx[NR],rly[NR],rldx[NR],rldy[NR]; bool rlon[NR];
int robCnt,robTimer;
// soloman
int sx,sy,sStep,sDelay;
// door
int doorX,doorY;
// obstacles
struct Obs{int x,y,w;} obs[10]; int numObs;
// timing
time_t tik;
// console helpers
void setCol(int c){SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),c);}
void gxy(int x,int y){COORD p={(short)x,(short)y};SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),p);}
void hideCur(){CONSOLE_CURSOR_INFO ci={1,FALSE};SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci);}
// draw sprite
void drawSpr(int x,int y,const char**sp,int h,int c){for(int r=0;r<h;r++){gxy(x,y+r);setCol(c);cout<<sp[r];}setCol(7);}
void eraseSpr(int x,int y,int w,int h){string b(w,' ');for(int r=0;r<h;r++){gxy(x,y+r);cout<<b;}}
// draw door only when spawned (doorX>=0)
void drawDoor(){
    if(doorX<0) return;
    gxy(doorX,doorY);setCol(10);cout<<"[D]";setCol(7);
}
// draw obstacles
void drawObs(){for(int i=0;i<numObs;i++){gxy(obs[i].x,obs[i].y);setCol(8);cout<<'[';for(int j=0;j<obs[i].w;j++)cout<<'-';cout<<']';}setCol(7);}
// collision helpers
bool outBound(int x,int y,int w,int h){return x<1||x+w>WW-1||y<1||y+h>WH-1;}
bool obsCol(int x,int y,int w,int h){for(int r=y;r<y+h;r++)for(int c=x;c<x+w;c++)if(c>=0&&c<WW&&r>=0&&r<WH&&grid[r][c]=='#')return true;return false;}
bool boxCol(int ax,int ay,int aw,int ah,int bx,int by,int bw,int bh){return ax<bx+bw&&ax+aw>bx&&ay<by+bh&&ay+ah>by;}
bool laserInBox(int bx,int by,int bw,int bh){return lx>=bx&&lx<bx+bw&&ly>=by&&ly<by+bh;}
// all robots dead check
bool allDead(){for(int i=0;i<robCnt;i++) if(ra[i]) return false; return true;}
// robots overlap check
bool robOverlap(int idx,int nx,int ny){
    for(int j=0;j<robCnt;j++){
        if(j==idx||!ra[j]) continue;
        if(boxCol(nx,ny,RW,RH,rx[j],ry[j],RW,RH)) return true;
    }
    return false;
}

// place obstacles
void placeObs(){
    numObs=0;
    int cnt[]={2,3,3,4,4},wid[]={3,3,4,4,5};
    int c=cnt[level-1],w=wid[level-1];
    for(int i=0;i<c;i++){
        int ox,oy,t=0;
        do{ox=2+rand()%(WW-6-w);oy=2+rand()%(WH-5);t++;}while(grid[oy][ox]!=' '&&t<400);
        obs[numObs++]={ox,oy,w};
        for(int j=0;j<w+2;j++) grid[oy][ox+j]='#';
    }
}
// init level
void initLevel(){
    for(int r=0;r<WH;r++) for(int c=0;c<WW;c++) grid[r][c]=' ';
    placeObs();
    // door hidden at start (spawned when all robots dead)
    doorX=-10; doorY=-10;
    // ethan lower half
    do{ex=1+rand()%(WW-4);ey=WH/2+rand()%(WH/2-4);}while(obsCol(ex,ey,EW,EH));
    // robots upper half - no overlap
    robCnt=level+1; robTimer=0;
    for(int i=0;i<robCnt;i++){
        int t=0;
        do{rx[i]=1+rand()%(WW-4);ry[i]=1+rand()%(WH/2-3);t++;}
        while((obsCol(rx[i],ry[i],RW,RH)||robOverlap(i,rx[i],ry[i]))&&t<500);
        ra[i]=true; rlon[i]=false;
    }
    // soloman
    do{sx=1+rand()%(WW-6);sy=1+rand()%(WH/2-4);}while(obsCol(sx,sy,SW,SH));
    sDelay=max(6,14-level*2); sStep=0;
    lives=3; lon=false; aimX=0; aimY=-1; tik=time(0);
}
// respawn positions only (keep lives/score)
void respawnPos(){
    do{ex=1+rand()%(WW-4);ey=WH/2+rand()%(WH/2-4);}while(obsCol(ex,ey,EW,EH));
    for(int i=0;i<robCnt;i++){
        if(!ra[i]) continue;
        int t=0;
        do{rx[i]=1+rand()%(WW-4);ry[i]=1+rand()%(WH/2-3);t++;}
        while((obsCol(rx[i],ry[i],RW,RH)||robOverlap(i,rx[i],ry[i]))&&t<500);
        rlon[i]=false;
    }
    do{sx=1+rand()%(WW-6);sy=1+rand()%(WH/2-4);}while(obsCol(sx,sy,SW,SH));
    lon=false; aimX=0; aimY=-1;
}
// full redraw
void drawAll(){
    system("cls"); hideCur();
    // border
    setCol(14);
    for(int x=0;x<WW;x++){gxy(x,0);cout<<'-';gxy(x,WH-1);cout<<'-';}
    for(int y=0;y<WH;y++){gxy(0,y);cout<<'|';gxy(WW-1,y);cout<<'|';}
    setCol(7);
    drawObs(); drawDoor();
    for(int i=0;i<robCnt;i++) if(ra[i]) drawSpr(rx[i],ry[i],RS,RH,12);
    drawSpr(sx,sy,SS,SH,13);
    drawSpr(ex,ey,ES,EH,11);
    if(lon){gxy(lx,ly);setCol(14);cout<<'*';setCol(7);}
    gxy(0,WH);setCol(11);
    cout<<" Lvl:"<<level<<" Lives:"<<lives<<" Score:"<<score<<"  [Arrows/WASD=Move | Space=Shoot]     ";
    setCol(7);
}
// partial redraw
void partDraw(){
    for(int i=0;i<robCnt;i++) if(ra[i]) drawSpr(rx[i],ry[i],RS,RH,12);
    drawSpr(sx,sy,SS,SH,13);
    drawSpr(ex,ey,ES,EH,11);
    if(lon){gxy(lx,ly);setCol(14);cout<<'*';setCol(7);}
    drawDoor();
    gxy(0,WH);setCol(11);
    cout<<" Lvl:"<<level<<" Lives:"<<lives<<" Score:"<<score<<"        ";
    setCol(7);
}
// tracking missile: kill nearest robot
void trackMissile(){
    for(int i=0;i<robCnt;i++) if(ra[i]){eraseSpr(rx[i],ry[i],RW,RH);ra[i]=false;score+=2;Beep(1200,150);return;}
}
// try move ethan, return true=level done
bool tryMove(int ddx,int ddy){
    int nx=ex+ddx,ny=ey+ddy;
    // door: only if all robots dead
    if(allDead()&&boxCol(nx,ny,EW,EH,doorX,doorY,3,1)){score+=5;return true;}
    if(outBound(nx,ny,EW,EH)||obsCol(nx,ny,EW,EH)){lives--;Beep(200,150);return false;}
    // check robot body collision after move
    for(int i=0;i<robCnt;i++){
        if(ra[i]&&boxCol(nx,ny,EW,EH,rx[i],ry[i],RW,RH)){lives--;Beep(300,200);return false;}
    }
    eraseSpr(ex,ey,EW,EH);
    ex=nx;ey=ny;
    aimX=ddx;aimY=ddy;
    return false;
}
// update lasers, return true=ethan killed
bool updateLasers(){
    if(lon){
        gxy(lx,ly);cout<<' ';
        lx+=ldx;ly+=ldy;
        if(lx<0||lx>=WW||ly<0||ly>=WH||obsCol(lx,ly,1,1)||outBound(lx,ly,1,1)){lon=false;}
        else{
            for(int i=0;i<robCnt;i++){
                if(!ra[i]) continue;
                if(laserInBox(rx[i],ry[i],RW,RH)){
                    eraseSpr(rx[i],ry[i],RW,RH);ra[i]=false;lon=false;
                    score+=2;Beep(700,100);
                    if(score%10==0) trackMissile();
                    break;
                }
                // cancel robot laser
                if(rlon[i]&&lx==rlx[i]&&ly==rly[i]){rlon[i]=false;lon=false;break;}
            }
        }
    }
    // robot lasers
    for(int i=0;i<robCnt;i++){
        if(!ra[i]||!rlon[i]) continue;
        gxy(rlx[i],rly[i]);cout<<' ';
        rlx[i]+=rldx[i];rly[i]+=rldy[i];
        if(rlx[i]<0||rlx[i]>=WW||rly[i]<0||rly[i]>=WH||obsCol(rlx[i],rly[i],1,1)){rlon[i]=false;}
        else if(laserInBox(ex,ey,EW,EH)){rlon[i]=false;lives--;Beep(300,250);if(lives<=0)return true;}
    }
    return false;
}
// move robots - no overlap allowed
void moveRobots(){
    int spds[]={8,7,6,5,5};
    robTimer++;
    if(robTimer<spds[level-1]) return;
    robTimer=0;
    for(int i=0;i<robCnt;i++){
        if(!ra[i]) continue;
        int ddx=(ex>rx[i])?1:(ex<rx[i]?-1:0);
        int ddy=(ey>ry[i])?1:(ey<ry[i]?-1:0);
        int nx=rx[i]+ddx,ny=ry[i]+ddy;
        // wall kills robot
        if(outBound(nx,ny,RW,RH)){eraseSpr(rx[i],ry[i],RW,RH);ra[i]=false;score+=2;continue;}
        // obstacle or soloman: bounce random
        if(obsCol(nx,ny,RW,RH)||boxCol(nx,ny,RW,RH,sx,sy,SW,SH)||robOverlap(i,nx,ny)){
            eraseSpr(rx[i],ry[i],RW,RH);
            int t=0;
            do{rx[i]=1+rand()%(WW-4);ry[i]=1+rand()%(WH/2-3);t++;}
            while((obsCol(rx[i],ry[i],RW,RH)||robOverlap(i,rx[i],ry[i]))&&t<400);
        } else {
            // check ethan contact
            if(boxCol(nx,ny,RW,RH,ex,ey,EW,EH)){lives--;Beep(400,200);}
            eraseSpr(rx[i],ry[i],RW,RH);rx[i]=nx;ry[i]=ny;
        }
        // fire laser
        if(!rlon[i]&&rand()%40==0){
            rldx[i]=(ex>rx[i])?1:(ex<rx[i]?-1:0);
            rldy[i]=(ey>ry[i])?1:(ey<ry[i]?-1:0);
            if(rldx[i]||rldy[i]){rlx[i]=rx[i]+rldx[i];rly[i]=ry[i]+rldy[i];rlon[i]=true;}
        }
    }
}
// move soloman: teleports on wall/obstacle hit
void moveSoloman(){
    sStep++;
    if(sStep<sDelay) return;
    sStep=0;
    eraseSpr(sx,sy,SW,SH);
    int ddx=(ex>sx)?1:(ex<sx?-1:0);
    int ddy=(ey>sy)?1:(ey<sy?-1:0);
    // add occasional random wander
    if(rand()%4==0){ddx=rand()%3-1;ddy=rand()%3-1;}
    int nx=sx+ddx, ny=sy+ddy;
    // teleport if wall or obstacle
    if(outBound(nx,ny,SW,SH)||obsCol(nx,ny,SW,SH)){
        do{sx=1+rand()%(WW-6);sy=1+rand()%(WH-6);}
        while(obsCol(sx,sy,SW,SH)||outBound(sx,sy,SW,SH));
    } else {
        sx=nx; sy=ny;
    }
    if(boxCol(ex,ey,EW,EH,sx,sy,SW,SH)){lives--;Beep(200,350);}
}
// spawn door on wall when all robots cleared
void checkSpawnDoor(){
    if(doorX>=0 || !allDead()) return;
    int side=rand()%3;
    if(side==0){
        // top border wall (y=0), ensure [D] fits inside border
        doorY=0;
        doorX=2+rand()%(WW-6);  // col 2..WW-4 so [D] stays within top wall
    } else if(side==1){
        // left border wall (x=0)
        doorX=0;
        doorY=2+rand()%(WH/2);  // upper half
    } else {
        // right border wall: [D] is 3 chars, so start at WW-1-2=WW-3
        doorX=WW-4;
        doorY=2+rand()%(WH/2);
    }
    drawDoor();
}
// high scores
struct HS{char name[30];int pts;};
int loadHS(HS a[]){ifstream f("scores.txt");int n=0;while(n<10&&f>>a[n].name>>a[n].pts)n++;return n;}
void saveHS(int pts,const char*name){
    HS a[11];int n=loadHS(a);
    strcpy(a[n].name,name);a[n].pts=pts;n++;
    for(int i=0;i<n-1;i++) for(int j=i+1;j<n;j++) if(a[j].pts>a[i].pts){HS t=a[i];a[i]=a[j];a[j]=t;}
    if(n>10)n=10;
    ofstream f("scores.txt");for(int i=0;i<n;i++)f<<a[i].name<<" "<<a[i].pts<<"\n";
}
void showScores(){
    system("cls");setCol(14);
    cout<<"\n\n                 +-- TOP 10 HIGH SCORES --+\n\n";
    HS a[10];int n=loadHS(a);
    if(!n){setCol(8);cout<<"                 No scores yet.\n";}
    for(int i=0;i<n;i++){setCol(i==0?14:7);cout<<"                 "<<(i+1)<<". "<<a[i].name<<" - "<<a[i].pts<<"\n";}
    setCol(7);cout<<"\n                 Press any key...";_getch();
}
void showHelp(){
    system("cls");setCol(11);
    cout<<"\n\n                +--- HOW TO PLAY ---+\n\n";
    setCol(7);
    cout<<"                Arrows / WASD  = Move\n";
    cout<<"                Space          = Shoot\n";
    cout<<"                ESC            = Quit\n\n";
    setCol(11);cout<<"                E"; setCol(7);cout<<" Ethan (you)   ";
    setCol(12);cout<<"R"; setCol(7);cout<<" = Robot (+2pts)\n";
    setCol(13);cout<<"                S"; setCol(7);cout<<" Soloman (invincible)\n";
    setCol(10);cout<<"                D"; setCol(7);cout<<" = Exit door (+5pts)\n\n";
    cout<<"                Every 10 pts = tracking missile!\n";
    cout<<"                Wall/obstacle = lose life\n\n";
    cout<<"                Press any key...";_getch();
}
// centered arrow-key menu
int showMenu(){
    const char* items[]={
        "[ START GAME ]",
        "[ HIGH SCORES]",
        "[ HOW TO PLAY]",
        "[    QUIT    ]"
    };
    int sel=0;
    while(true){
        system("cls");hideCur();
        // Box: 52 wide, centered in 80
        int W=52, L=(80-W)/2;
        // top bar
        setCol(9); gxy(L,2);
        cout<<'+';
        for(int i=0;i<W-2;i++) cout<<'=';
        cout<<'+';
        // title row
        setCol(14); gxy(L,3); cout<<'|';
        setCol(11); gxy(L+1,3);
        cout<<"  M I S S I O N  :  I M P O S S I B L E  ";
        setCol(14); gxy(L+W-1,3); cout<<'|';
        // subtitle row
        setCol(14); gxy(L,4); cout<<'|';
        setCol(8);  gxy(L+1,4);
        cout<<"      A thrilling 5-level console game      ";
        setCol(14); gxy(L+W-1,4); cout<<'|';
        // divider
        setCol(9); gxy(L,5);
        cout<<'+';
        for(int i=0;i<W-2;i++) cout<<'-';
        cout<<'+';
        // sprite legend header
        setCol(8); gxy(L+1,7);
        cout<<" -- Characters --";
        // Ethan
        gxy(L+3,8);  setCol(11); cout<<ES[0];
        gxy(L+3,9);  setCol(11); cout<<ES[1];
        gxy(L+3,10); setCol(11); cout<<ES[2];
        gxy(L+7,9);  setCol(8);  cout<<"Ethan Hunt";
        // Robot
        gxy(L+20,8);  setCol(12); cout<<RS[0];
        gxy(L+20,9);  setCol(12); cout<<RS[1];
        gxy(L+24,9);  setCol(8);  cout<<"Robot";
        // Soloman
        gxy(L+33,8);  setCol(13); cout<<SS[0];
        gxy(L+33,9);  setCol(13); cout<<SS[1];
        gxy(L+33,10); setCol(13); cout<<SS[2];
        gxy(L+39,9);  setCol(8);  cout<<"Soloman";
        // obstacle + door on same row
        gxy(L+3,12);  setCol(8);  cout<<"[----] Obstacle";
        gxy(L+20,12); setCol(10); cout<<"[D] Door (kill all to unlock)";
        // divider
        setCol(9); gxy(L,14);
        cout<<'+';
        for(int i=0;i<W-2;i++) cout<<'-';
        cout<<'+';
        // menu items centered
        for(int i=0;i<4;i++){
            gxy(L+17,15+i);
            if(i==sel){setCol(14);cout<<">> "<<items[i]<<" <<";}
            else       {setCol(7); cout<<"   "<<items[i];}
        }
        // bottom bar
        setCol(9); gxy(L,20);
        cout<<'+';
        for(int i=0;i<W-2;i++) cout<<'=';
        cout<<'+';
        // hint
        gxy(L+7,21); setCol(8);
        cout<<"W/S or Arrows = Move | Enter = Confirm";
        setCol(7);
        int k=_getch();
        if(k==0||k==224){k=_getch();if(k==72)sel=(sel-1+4)%4;if(k==80)sel=(sel+1)%4;}
        else if(k=='w'||k=='W') sel=(sel-1+4)%4;
        else if(k=='s'||k=='S') sel=(sel+1)%4;
        else if(k==13) return sel;
        else if(k==27) return 3;
    }
}
// game
void playGame(){
    srand((unsigned)time(0));
    score=0; level=1;
    while(level<=5){
        initLevel(); drawAll();
        bool levelDone=false, gameOver=false;

        while(!levelDone && !gameOver){
            int prevLives = lives;

            // --- input ---
            if(_kbhit()){
                int k=_getch();
                if(k==0||k==224){
                    k=_getch(); bool r=false;
                    if(k==72){aimX=0; aimY=-1; r=tryMove(0,-1);}
                    if(k==80){aimX=0; aimY= 1; r=tryMove(0, 1);}
                    if(k==75){aimX=-1;aimY= 0; r=tryMove(-1,0);}
                    if(k==77){aimX= 1;aimY= 0; r=tryMove( 1,0);}
                    if(r){levelDone=true; break;}
                }
                else if(k=='w'||k=='W'){aimX=0; aimY=-1; if(tryMove(0,-1)){levelDone=true;break;}}
                else if(k=='s'||k=='S'){aimX=0; aimY= 1; if(tryMove(0, 1)){levelDone=true;break;}}
                else if(k=='a'||k=='A'){aimX=-1;aimY= 0; if(tryMove(-1,0)){levelDone=true;break;}}
                else if(k=='d'||k=='D'){aimX= 1;aimY= 0; if(tryMove( 1,0)){levelDone=true;break;}}
                else if(k==' '&&!lon){lx=ex+aimX;ly=ey+aimY;ldx=aimX;ldy=aimY;if(ldx||ldy)lon=true;Beep(500,40);}
                else if(k==27){gameOver=true; level=99; break;}
            }

            // --- time score ---
            if(time(0)-tik>=10){score++; tik=time(0);}

            // --- update entities ---
            updateLasers();
            moveRobots();
            moveSoloman();
            // spawn door silently when all robots die
            checkSpawnDoor();

            // --- check life lost ---
            if(lives < prevLives){
                // life was lost this frame
                if(lives<=0){
                    gameOver=true; break;
                }
                // respawn: reset positions, keep lives & score
                Beep(200,400);
                Sleep(400);
                respawnPos();
                drawAll();
                continue; // skip partDraw, restart loop
            }

            partDraw();
            Sleep(60);
        }

        if(gameOver && level!=99) break;
        if(level<=5) level++;
    }
    system("cls");
    setCol(level>5?10:12);
    if(level>5) cout<<"\n\n         ** YOU COMPLETED ALL 5 LEVELS! **\n";
    else        cout<<"\n\n         ** GAME OVER **\n";
    setCol(14);cout<<"         Final Score: "<<score<<"\n\n";
    setCol(11);cout<<"         Your name: ";setCol(7);
    char name[30];cin>>name;
    saveHS(score,name);
    cout<<"         Saved!\n";Sleep(1200);
}
int main(){
    hideCur();
    while(true){
        int c=showMenu();
        if(c==0) playGame();
        else if(c==1) showScores();
        else if(c==2) showHelp();
        else break;
    }
    system("cls");setCol(11);cout<<"\n  Thanks for playing!\n";setCol(7);
    return 0;
}
