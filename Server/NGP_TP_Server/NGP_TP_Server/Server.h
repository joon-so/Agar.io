#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <random>
using namespace std;

#define SERVERPORT      9000
#define MAX_BUFFER      1024
#define MAP_SIZE	    30.f	//맵 한칸당 크기
#define FEED_MAX_NUM    500     //최대먹이 갯수
#define ITEM_COUNT      20      //최대 아이템 개수
#define MAX_CHAT_SIZE 100       //최대 채팅 길이

CRITICAL_SECTION ac_move;
CRITICAL_SECTION ac_user;
CRITICAL_SECTION ac_feed;
CRITICAL_SECTION ac_item;
CRITICAL_SECTION ac_trap;


constexpr char CS_MOVE = 11;
constexpr char CS_CHAT = 12;

constexpr char SC_FIRST_POS = 13;
constexpr char SC_USER_MOVE = 14;
constexpr char SC_USER_SIZE = 15;
constexpr char SC_CHAT = 16;

constexpr char SC_LOGIN = 17;
constexpr char SC_LOGOUT = 18;

constexpr char SC_ALL_FEED = 19;
constexpr char SC_ALL_TRAP = 20;
constexpr char SC_ALL_ITEM = 21;


constexpr char SC_FEED_USER = 22;
constexpr char SC_TRAP_USER = 23;
constexpr char SC_ITEM_USER = 24;



uniform_int_distribution<> uiNUM(50, 255);
uniform_int_distribution<> enemy_position_NUM(-49 * MAP_SIZE, 49 * MAP_SIZE);
default_random_engine dre{ 2016182007 };

typedef struct sc_user_data_packet
{
    char type;
    int id;
    short x;
    short y;
    float size;
}sc_user_data_packet;

typedef struct sc_logout_packet
{
    char type;
    int id;

}sc_logout_packet;

typedef struct sc_user_size_packet
{
    char type;
    int id;
    float  size;

}sc_user_size_packet;

typedef struct cs_chat_packet
{
    char type;
    int id;
    char chat[MAX_CHAT_SIZE];
}cs_chat_packet;


class User {
    int id;
    short x;
    short y;
    float size = 16.f;

public:
    User() {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
        size = 16.f;
        id = 0;
    }
    User(int id) :id{ id } {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
    }

    //다른 원과의 거리 측정
    float MeasureDistance(User user1) {
        float distance = sqrt(pow(user1.x - x, 2) + pow(user1.y - y, 2));
        return distance;
    }

    //다른 유저와 충돌처리
    void CrushCheck(User user1);

    //x좌표 설정
    void SetXpos(short xpos) {
        x = xpos;
    }
    //y좌표 설정
    void SetYpos(short ypos) {
        y = ypos;
    }
    //size 설정
    void SetSize(float newsize) {
        size = newsize;
    }
    //x좌표 리턴
    short GetXpos() {
        return x;
    }
    //y좌표 리턴
    short GetYpos() {
        return y;
    }
    //size 리턴
    float GetSize() {
        return size;
    }
    //id 리턴
    int GetId() const {
        return id;
    }

    bool operator==(const User& rhs) const {
        return GetId() == rhs.GetId();
    };
};

vector<User> users;

void send_feedposi_usersize_data(SOCKET soc, int uid, float usize, int fi, short fx, short fy);

class Feed {
    short x;
    short y;
    float size = 5.f;

public:
    Feed() {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
    }

    //다른 원과의 거리 측정
    float MeasureDistance(User user) {
        float distance = sqrt(pow(user.GetXpos() - x, 2) + pow(user.GetYpos() - y, 2));
        return distance;
    }

    void CrushCheck(User user, int i) {
        // 충돌체크 후 다시 먹힐경우 다시 위치 조정
        if (MeasureDistance(user) < user.GetSize()) {
            //스레드 동기화해야함
            user.SetSize(user.GetSize() + (size * 0.35));
            x = enemy_position_NUM(dre);
            y = enemy_position_NUM(dre);
            for (User& u : users) {
                if (user.GetId() == u.GetId()) {
                    u.SetSize(user.GetSize());
                }
                send_feedposi_usersize_data((SOCKET)u.GetId(), user.GetId(), user.GetSize(), i, x, y);
            }

        }
    }

    //x좌표 설정
    short SetXpos(short xpos) {
        x = xpos;
    }
    //y좌표 설정
    short SetYpos(short ypos) {
        y = ypos;
    }
    //x좌표 리턴
    short GetXpos() {
        return x;
    }
    //y좌표 리턴
    short GetYpos() {
        return y;
    }
};

void send_item_type(SOCKET soc, int uid, bool type, int ii, short ix, short iy);

class Item {
    int x;
    int y;
    short type; //아이템 종류 구별

public:
    Item() {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
        type = rand() % 2;
    }

    //서버로부터 좌표와 타입을 받아옴
    Item(int x, int y, bool type) : x{ x }, y{ y }, type{ type } {}

    //충동체크
    void CrushCheck(User user, int i) {
        if (sqrt(pow(user.GetXpos() - x, 2) + pow(user.GetYpos() - y, 2)) < user.GetSize()) {
            x = enemy_position_NUM(dre);
            y = enemy_position_NUM(dre);
            for (User& u : users) {
                send_item_type((SOCKET)u.GetId(), user.GetId(), type, i, x, y);
            }
            cout << x << " " << y << endl;
        }
    }

    //x좌표 설정
    int SetXpos(int xpos) {
        x = xpos;
    }
    //y좌표 설정
    int SetYpos(int ypos) {
        y = ypos;
    }
    //x좌표 리턴
    int GetXpos() {
        return x;
    }
    //y좌표 리턴
    int GetYpos() {
        return y;
    }
};

void send_trapposi_usersize_data(SOCKET soc, int uid, float usize, int ti, short tx, short ty);

class Trap {
    short x;
    short y;

public:
    //함정 재배치
    Trap() {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
    }

    //서버로부터 좌표와 타입을 받아옴
    Trap(short x, short y) : x{ x }, y{ y } {}

    //충돌처리
    void CrushCheck(User user, int i) {
        if (sqrt(pow(user.GetXpos() - x, 2) + pow(user.GetYpos() - y, 2)) < user.GetSize()) {
            x = enemy_position_NUM(dre);
            y = enemy_position_NUM(dre);

            user.SetSize(user.GetSize() * 0.8f);
            if (user.GetSize() < 16.f)
                user.SetSize(16.f);

            //패킷 보내기
            for (User& u : users) {
                if (user.GetId() == u.GetId()) {
                    u.SetSize(user.GetSize());
                }
                send_trapposi_usersize_data((SOCKET)u.GetId(), user.GetId(), user.GetSize(), i, x, y);
            }
            cout << x << " " << y << endl;
        }
    }

    //x좌표 설정
    void SetXpos(short xpos) {
        x = xpos;
    }
    //y좌표 설정
    void SetYpos(short ypos) {
        y = ypos;
    }
    //x좌표 리턴
    short GetXpos() {
        return x;
    }
    //y좌표 리턴
    short GetYpos() {
        return y;
    }
};

Feed feed[FEED_MAX_NUM];
Item item[ITEM_COUNT];
Trap trap[ITEM_COUNT];

typedef struct sc_all_feed_packet
{
    char type;
    Feed feeds[FEED_MAX_NUM];
}sc_all_feed_packet;

typedef struct sc_all_trap_packet
{
    char type;
    Trap traps[ITEM_COUNT];
}sc_all_trap_packet;

typedef struct sc_all_item_packet
{
    char type;
    Item items[ITEM_COUNT];
}sc_all_item_packet;

typedef struct sc_feed_data_packet
{
    char type;
    int user_id;
    float user_size;
    Feed feeds[FEED_MAX_NUM];

}sc_feed_data_packet;

typedef struct sc_trap_data_packet
{
    char type;
    int user_id;
    float user_size;
    Trap traps[ITEM_COUNT];
}sc_trap_data_packet;

typedef struct sc_item_type_packet
{
    char type;
    int user_id;
    short item_type;
    Item items[ITEM_COUNT];
}sc_item_type_packet;

void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
void send_first_pos(SOCKET soc, User user);
void send_Login_packet(SOCKET soc, User user);
void send_user_move_packet(SOCKET soc, int id, int x, int y, float usize);
void send_all_feed_data(SOCKET soc);
void send_all_trap_data(SOCKET soc);
void send_all_item_data(SOCKET soc);
void send_user_logout_packet(SOCKET soc, int client);
void send_user_size_packet(SOCKET soc, int uid, float usize);
void send_chat_packet(SOCKET soc, cs_chat_packet* cp);