#include <iostream>
#include <curses.h>
#include <panel.h>
#include <vector>
#include <stack>
#include <string>
#include <utility>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <string>
#include <fstream>
using namespace std;

///////////////MAP 선택////////////////////
// map1.txt 난이도 하
// map2.txt 난이도 중
// map3.txt 난이도 상

#define SELECTED_MAP "map1.txt" 

//////////////////////////////////////////


#define KEY_UP 259
#define KEY_LEFT 260
#define KEY_RIGHT 261
#define KEY_DOWN 258


// Snake 관련
typedef pair<int, int> p;
queue<p> q;
int snackMap[101][101], n, m;
int y, x, yyy[4] = { 0,-1,0,1 }, xxx[4] = { -1,0,1,0 };
int curDir = 0, snackSize = 5;

// 게임 관련
bool is_gameover = false;
int tik = 0;

// growth 관련
int growth_num = 0;

// poison 관련
int poison_num = 0;

// fast 관련
int fast_num = 0;

// slow 관련
int slow_num = 0;

// gate 관련
int gate_num = 0;
int in_gate = 0;
int collapse_tik;
p gate_id[101][101];

// speed 관련
int speed_maximum = 700;
int frequency = 400;
int speed_minimum = 100;
int speed_var = 50;

// score 관련
int sum_speed = 0, move_cnt = 0;
int score[5];
char score_txt[5][100] = {
    "Length      ",
    "+           ",
    "-           ",
    "Gate        ",
    "Distance    ",
};
// 0: 스네이크 길이 * 전체 이동한 칸 수/(sum(frequency)/1000) / 4
// 1: 획득한 +
// 2: 획득한 -
// 3: Gate 사용 횟수
// 4: speed

int mission[5] = { 10,5,2,5,800 };
int mission_score[5] = { 30,30,50,80,150 };
bool mission_clear[5];


void input_map(string S)
{
    ifstream fin;
    fin.open(S);
    for (int i = 1; i <= 50; i++)
    {
        char C[100] = { 0 };
        fin.getline(C, 51);
        for (int j = 1; j <= 50; j++)
        {
            snackMap[i][j] = C[j - 1] - '0';
        }
    }
}



void init() {
    initscr(); // Curses 모드 시작
    start_color(); // Color 사용 선언
    curs_set(0); // 터미널 커서 사라짐
    noecho(); // 입력한 값 안보이게함

    init_pair(0, 0, 0); // 빈 공간색 검은색
    init_pair(1, 7, 7); // 일반벽 흰색(밝은 회색)
    init_pair(2, 8, 8); // 절대 벽 밝은검은색(회색)
    init_pair(3, 4, 4);   // 스네이크 머리 빨간색
    init_pair(4, 14, 14); // 스네이크 꼬리 노란색
    init_pair(5, 7, 9); // 성장아이템 밝은 파란색 
    init_pair(6, 7, 12); // 독아이템 밝은 빨강
    init_pair(7, 7, 1); // 빨라지는 아이템 파랑
    init_pair(8, 7, 13); // 느려지는 아이템 빨강
    init_pair(15, 7, 5); // 게이트 마젠타

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    srand(time(NULL)); // 랜덤 시드 초기화
    n = 50; // 맵 세로 길이
    m = 50; // 맵 가로 길이
    y = n / 2;
    x = m / 2;

    for (int i = 0; i <= n + 1; i++)
        snackMap[i][0] = snackMap[i][m + 1] = 2;
    for (int i = 0; i <= m + 1; i++)
        snackMap[0][i] = snackMap[n + 1][i] = 2;
    input_map(SELECTED_MAP);
    snackMap[n][1] = snackMap[n][m] = snackMap[1][1] = snackMap[1][m] = 2;


    for (int i = 1; i < snackSize; i++) {
        q.push(make_pair(y, x));
        snackMap[y][x] = 4;
        y += yyy[curDir]; x += xxx[curDir];
    }
    snackMap[y][x] = 3;
}
void drawMap() {
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            switch (snackMap[i][j]) {
            case 0: attron(COLOR_PAIR(0)); printw(" "); attroff(COLOR_PAIR(0)); break; // 빈 공간
            case 1: attron(COLOR_PAIR(1)); printw(" "); attroff(COLOR_PAIR(1)); break; // 일반 벽
            case 2: attron(COLOR_PAIR(2)); printw(" "); attroff(COLOR_PAIR(2)); break; // 게이트 생성 불가 벽
            case 3: attron(COLOR_PAIR(3)); printw(" "); attroff(COLOR_PAIR(3)); break;
            case 4: attron(COLOR_PAIR(4)); printw(" "); attroff(COLOR_PAIR(4)); break;
            case 5: attron(COLOR_PAIR(5)); printw("+"); attroff(COLOR_PAIR(5)); break; // 성장 아이템
            case 6: attron(COLOR_PAIR(6)); printw("-"); attroff(COLOR_PAIR(6)); break; // 독 아이템
            case 7: attron(COLOR_PAIR(7)); printw("F"); attroff(COLOR_PAIR(7)); break; // 빨라지는 
            case 8: attron(COLOR_PAIR(8)); printw("S"); attroff(COLOR_PAIR(8)); break; // 느려지는
            case 15: attron(COLOR_PAIR(15)); printw(" "); attroff(COLOR_PAIR(15)); break; // 게이트
            }
        }
        printw("\n");
    }
}

void drawScore() {
    move(7, m + 10);
    printw("Score Board");
    score[0] = snackSize;
    sum_speed += (400 - frequency) / 50;
    score[4] = sum_speed / 20;
    for (int i = 0; i < 5; i++) {
        move(9 + i, m + 10);
        printw("%s:", score_txt[i]);
        printw("%07d", score[i]);
    }
    move(14, m + 10);
    int s = 0;
    for (int i = 0; i < 5; i++) {
        if (mission_clear[i]) s += mission_score[i]; // 미션 점수
        s += score[i];
    }
    printw("Total Score :%07d", s);
    move(16, m + 10);
    printw("Current Speed %d", 1000 - frequency);
    if (gate_num) {
        move(17, m + 10);
        printw("gate Collapse time: %d", collapse_tik);
    }
}
void drawMission() {
    move(20, m + 10);
    printw("Mission Board");
    for (int i = 0; i < 5; i++) {
        move(21 + i, m + 10);
        printw("%s: ", score_txt[i]);
        printw("%d\t", mission[i]);
        if (!mission_clear[i] && score[i] >= mission[i])
            mission_clear[i] = true;
        if (mission_clear[i])
            printw("(V)");
        else
            printw("( )");
    }
}
void render() { //맵 랜더기 
    clear();
    drawMap();
    drawMission();
    drawScore();
    refresh();
}

void game_over() { // 게임오버 관련 처리기
    is_gameover = true;
    input_map("gameover.txt");
    clear();
    drawMap();
    drawScore();
    drawMission();
    refresh();
}


void move() { // 이동랜더기
    snackMap[y][x] = 4;
    q.push(make_pair(y, x)); // 꼬리 남기기
    y += yyy[curDir]; x += xxx[curDir]; // 한칸 이동
    if (1 <= snackMap[y][x] && snackMap[y][x] <= 4) game_over();
    else if (snackMap[y][x] == 15) { // 게이트다!
        score[3]++; // 점수 계산
        int my = gate_id[y][x].first, mx = gate_id[y][x].second; // my,mx는 연결된 게이트의 좌표
        int tmp[4] = { 0, 1, -1, 2 }; //직진 시계 역시계 후진 
        for (int i = 0; i < 4; i++) {
            int yy = my + yyy[(curDir + tmp[i] + 4) % 4], xx = mx + xxx[(curDir + tmp[i] + 4) % 4];
            if (snackMap[yy][xx] == 0) {
                y = yy; x = xx;
                snackMap[y][x] = 3;
                curDir = (curDir + tmp[i] + 4) % 4;
                snackMap[q.front().first][q.front().second] = 0;
                q.pop();
                in_gate = snackSize;
                break;
            }
        }
    }
    else { // 아이템, 빈공간이다!
       // 0, 5, 6
        switch (snackMap[y][x]) {
        case 6:
            snackMap[q.front().first][q.front().second] = 0;
            q.pop();
            poison_num--;
            snackSize--;
            score[2]++; // 점수 계산
        case 0:
            snackMap[q.front().first][q.front().second] = 0;
            q.pop();
            break;
        case 5:
            growth_num--;
            snackSize++;
            score[1]++; // 점수 계산
            break;
        case 7:
            frequency -= speed_var;
            frequency = max(speed_minimum, frequency);
            fast_num--;
            break;
        case 8:
            frequency += speed_var;
            frequency = min(speed_maximum, frequency);
            slow_num--;
            break;
        }
        snackMap[y][x] = 3;
    }
    if (in_gate) in_gate--;
}

p random_point(int t) { //좌표생성기
    vector<p> v;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            if (snackMap[i][j] == t)
                v.push_back(make_pair(i, j));
    int tmp = rand() % v.size();
    return v[tmp];
}

void growth_spawn() {  // 성장
    if (growth_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 5;
    growth_num++;
}

void poison_spawn() { // 독 아이템 랜덤 스폰
    if (tik % 10) return;
    p point;
    if (poison_num < 3) {
        point = random_point(0);
        snackMap[point.first][point.second] = 6;
        poison_num++;
    }
}

void fast_spawn() { // 빨라진다 아이템 스폰
    if (tik % 15 || fast_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 7;
    fast_num++;
}
void slow_spawn() { // 느려진다 아이템 스폰
    if (tik % 10 || slow_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 8;
    slow_num++;
}

void gate_spawn() { // 게이트 스폰
    if (tik % 20 || gate_num) return;
    p point1, point2;
    point1 = random_point(1);
    snackMap[point1.first][point1.second] = 15;
    point2 = random_point(1);
    snackMap[point2.first][point2.second] = 15;
    gate_id[point1.first][point1.second] = make_pair(point2.first, point2.second);
    gate_id[point2.first][point2.second] = make_pair(point1.first, point1.second);
    gate_num++;
}

void gate_collapse() {
    collapse_tik = ((800 - frequency) / 5) - (tik % ((800 - frequency) / 5));
    if (tik % ((800 - frequency) / 5) || in_gate || gate_num == 0) return;
    p point1 = random_point(15);
    p point2 = gate_id[point1.first][point1.second];
    snackMap[point1.first][point1.second] = 1;
    snackMap[point2.first][point2.second] = 1;
    gate_num--;
}

int main()
{
    init();
    clock_t start;
    //cout << "Hello World!\n";
    //attron(COLOR_PAIR(1)); // Attribute 적용
    //printw("Hello world"); // Hello world 출력
    //attroff(COLOR_PAIR(1)); // Attribute 해제
    while (!is_gameover) {
        tik = (tik + 1) % 3600;
        
        render();
        int keyPressed = -1, tmp;
        int next = curDir;
        start = clock();
        while (clock() - start < frequency) {
            tmp = getch();
            if (tmp != ERR)
                keyPressed = tmp;
        }
        switch (keyPressed) {
        case KEY_LEFT: next = 0;  break;
        case KEY_UP: next = 1; break;
        case KEY_RIGHT: next = 2; break;
        case KEY_DOWN: next = 3; break;
        }
        
      
        
        if ((next + 2) % 4 != curDir) {
            curDir = next;
            move();
            growth_spawn();
            poison_spawn();
            fast_spawn();
            slow_spawn();
            gate_collapse();
            gate_spawn();
        }
        else {
            game_over();
        }
        if (snackSize <= 3) game_over();
        
    }
    endwin(); // Curses 모드 종료
    return 0;
}
