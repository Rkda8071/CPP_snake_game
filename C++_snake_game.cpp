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

///////////////MAP ����////////////////////
// map1.txt ���̵� ��
// map2.txt ���̵� ��
// map3.txt ���̵� ��

#define SELECTED_MAP "map1.txt" 

//////////////////////////////////////////


#define KEY_UP 259
#define KEY_LEFT 260
#define KEY_RIGHT 261
#define KEY_DOWN 258


// Snake ����
typedef pair<int, int> p;
queue<p> q;
int snackMap[101][101], n, m;
int y, x, yyy[4] = { 0,-1,0,1 }, xxx[4] = { -1,0,1,0 };
int curDir = 0, snackSize = 5;

// ���� ����
bool is_gameover = false;
int tik = 0;

// growth ����
int growth_num = 0;

// poison ����
int poison_num = 0;

// fast ����
int fast_num = 0;

// slow ����
int slow_num = 0;

// gate ����
int gate_num = 0;
int in_gate = 0;
int collapse_tik;
p gate_id[101][101];

// speed ����
int speed_maximum = 700;
int frequency = 400;
int speed_minimum = 100;
int speed_var = 50;

// score ����
int sum_speed = 0, move_cnt = 0;
int score[5];
char score_txt[5][100] = {
    "Length      ",
    "+           ",
    "-           ",
    "Gate        ",
    "Distance    ",
};
// 0: ������ũ ���� * ��ü �̵��� ĭ ��/(sum(frequency)/1000) / 4
// 1: ȹ���� +
// 2: ȹ���� -
// 3: Gate ��� Ƚ��
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
    initscr(); // Curses ��� ����
    start_color(); // Color ��� ����
    curs_set(0); // �͹̳� Ŀ�� �����
    noecho(); // �Է��� �� �Ⱥ��̰���

    init_pair(0, 0, 0); // �� ������ ������
    init_pair(1, 7, 7); // �Ϲݺ� ���(���� ȸ��)
    init_pair(2, 8, 8); // ���� �� ����������(ȸ��)
    init_pair(3, 4, 4);   // ������ũ �Ӹ� ������
    init_pair(4, 14, 14); // ������ũ ���� �����
    init_pair(5, 7, 9); // ��������� ���� �Ķ��� 
    init_pair(6, 7, 12); // �������� ���� ����
    init_pair(7, 7, 1); // �������� ������ �Ķ�
    init_pair(8, 7, 13); // �������� ������ ����
    init_pair(15, 7, 5); // ����Ʈ ����Ÿ

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    srand(time(NULL)); // ���� �õ� �ʱ�ȭ
    n = 50; // �� ���� ����
    m = 50; // �� ���� ����
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
            case 0: attron(COLOR_PAIR(0)); printw(" "); attroff(COLOR_PAIR(0)); break; // �� ����
            case 1: attron(COLOR_PAIR(1)); printw(" "); attroff(COLOR_PAIR(1)); break; // �Ϲ� ��
            case 2: attron(COLOR_PAIR(2)); printw(" "); attroff(COLOR_PAIR(2)); break; // ����Ʈ ���� �Ұ� ��
            case 3: attron(COLOR_PAIR(3)); printw(" "); attroff(COLOR_PAIR(3)); break;
            case 4: attron(COLOR_PAIR(4)); printw(" "); attroff(COLOR_PAIR(4)); break;
            case 5: attron(COLOR_PAIR(5)); printw("+"); attroff(COLOR_PAIR(5)); break; // ���� ������
            case 6: attron(COLOR_PAIR(6)); printw("-"); attroff(COLOR_PAIR(6)); break; // �� ������
            case 7: attron(COLOR_PAIR(7)); printw("F"); attroff(COLOR_PAIR(7)); break; // �������� 
            case 8: attron(COLOR_PAIR(8)); printw("S"); attroff(COLOR_PAIR(8)); break; // ��������
            case 15: attron(COLOR_PAIR(15)); printw(" "); attroff(COLOR_PAIR(15)); break; // ����Ʈ
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
        if (mission_clear[i]) s += mission_score[i]; // �̼� ����
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
void render() { //�� ������ 
    clear();
    drawMap();
    drawMission();
    drawScore();
    refresh();
}

void game_over() { // ���ӿ��� ���� ó����
    is_gameover = true;
    input_map("gameover.txt");
    clear();
    drawMap();
    drawScore();
    drawMission();
    refresh();
}


void move() { // �̵�������
    snackMap[y][x] = 4;
    q.push(make_pair(y, x)); // ���� �����
    y += yyy[curDir]; x += xxx[curDir]; // ��ĭ �̵�
    if (1 <= snackMap[y][x] && snackMap[y][x] <= 4) game_over();
    else if (snackMap[y][x] == 15) { // ����Ʈ��!
        score[3]++; // ���� ���
        int my = gate_id[y][x].first, mx = gate_id[y][x].second; // my,mx�� ����� ����Ʈ�� ��ǥ
        int tmp[4] = { 0, 1, -1, 2 }; //���� �ð� ���ð� ���� 
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
    else { // ������, ������̴�!
       // 0, 5, 6
        switch (snackMap[y][x]) {
        case 6:
            snackMap[q.front().first][q.front().second] = 0;
            q.pop();
            poison_num--;
            snackSize--;
            score[2]++; // ���� ���
        case 0:
            snackMap[q.front().first][q.front().second] = 0;
            q.pop();
            break;
        case 5:
            growth_num--;
            snackSize++;
            score[1]++; // ���� ���
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

p random_point(int t) { //��ǥ������
    vector<p> v;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            if (snackMap[i][j] == t)
                v.push_back(make_pair(i, j));
    int tmp = rand() % v.size();
    return v[tmp];
}

void growth_spawn() {  // ����
    if (growth_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 5;
    growth_num++;
}

void poison_spawn() { // �� ������ ���� ����
    if (tik % 10) return;
    p point;
    if (poison_num < 3) {
        point = random_point(0);
        snackMap[point.first][point.second] = 6;
        poison_num++;
    }
}

void fast_spawn() { // �������� ������ ����
    if (tik % 15 || fast_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 7;
    fast_num++;
}
void slow_spawn() { // �������� ������ ����
    if (tik % 10 || slow_num) return;
    p point;
    point = random_point(0);
    snackMap[point.first][point.second] = 8;
    slow_num++;
}

void gate_spawn() { // ����Ʈ ����
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
    //attron(COLOR_PAIR(1)); // Attribute ����
    //printw("Hello world"); // Hello world ���
    //attroff(COLOR_PAIR(1)); // Attribute ����
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
    endwin(); // Curses ��� ����
    return 0;
}
