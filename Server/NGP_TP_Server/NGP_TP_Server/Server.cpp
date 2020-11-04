#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include<random>
using namespace std;

#define SERVERPORT 9000
#define MAX_BUFFER 1024
#define MAP_SIZE	100.f	//�� ��ĭ�� ũ��

constexpr char SC_POS = 0;
constexpr char CS_MOVE = 1;

uniform_int_distribution<> uiNUM(50, 255);
uniform_int_distribution<> enemy_position_NUM(-50 * MAP_SIZE, 50 * MAP_SIZE);
default_random_engine dre{ 2016182007 };

typedef struct position_packet
{
    char type;
    int x;
    int y;
}position_packet;

typedef struct sc_login_packet
{
    char type;
    int id;
    bool login;
};


class User {
    int id;
    int x;
    int y;
    float size;

public:
    User() {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
        size = 20.f;
        id = 0;
    }
    User(int id) :id{ id } {
        x = enemy_position_NUM(dre);
        y = enemy_position_NUM(dre);
        size = 20.f;
    }


    //�ٸ� ������ �Ÿ� ����
    float MeasureDistance(User user1) {
        float distance = sqrt(pow(user1.x - x, 2) + pow(user1.y - y, 2));
        return distance;
    }

    //�ٸ� ������ �浹ó��
    void CrushCheck(User user1) {
        // �浹 üũ �� ������ �� Ŭ ���
        if (user1.GetSize() > size) {
            if (MeasureDistance(user1) < user1.GetSize()) {
                float newsize = user1.GetSize() + size * 0.3f;
                user1.SetSize(newsize);
                //���� ����
            }
        }

        // �浹üũ �� ���� �� ū���
        else if (user1.GetSize() < size) {
            if (MeasureDistance(user1) < size) {
                float newsize = size + user1.GetSize() * 0.3f;
                size = newsize;
                //��밡 ����
            }
        }
    }

    //x��ǥ ����
    void SetXpos(int xpos) {
        x = xpos;
    }
    //y��ǥ ����
    void SetYpos(int ypos) {
        y = ypos;
    }
    //size ����
    void SetSize(float newsize) {
        size = newsize;
    }
    //x��ǥ ����
    int GetXpos() {
        return x;
    }
    //y��ǥ ����
    int GetYpos() {
        return y;
    }
    //size ����
    float GetSize() {
        return size;
    }
    //id ����
    int GetId() const {
        return id;
    }

    bool operator==(const User& rhs) const {
        return GetId() == rhs.GetId();
    };
};

vector<User> users;

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    //���� ������ ���� ��ȯ
    return (len - left);
}


void send_first_pos(SOCKET soc, User user)
{
    position_packet mp;
    char buf[MAX_BUFFER];
    int num_sent;

    mp.type = SC_POS;
    mp.x = user.GetXpos();
    mp.y = user.GetYpos();

    int retval = send(soc, (char*)&mp, sizeof(position_packet), 0);

    cout << "Sent : " << mp.x << " " << mp.y << " " << "(" << retval << " bytes)\n";
}

//void send_Login_packet(User u)
//{
//
//}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    char buf[MAX_BUFFER];
    int len;
    //����ü  Ȯ�� ptr
    char* ptr;

    while (true) {
        //������ ����
        retval = recv(client_sock, (char*)&buf, MAX_BUFFER, 0);
        if (client_sock == INVALID_SOCKET) err_quit("recv()");
        if (retval <= 0) break;

        //���ŵ� ��Ŷ ó��
        switch (buf[0]){
        case CS_MOVE: {
            position_packet* mp = reinterpret_cast<position_packet*>(buf);
            int x = mp->x;
            int y = mp->y;

            cout << client_sock << " x = " << x << " y = " << y << endl;
            break;
        }
        default:
            break;
           
        }
    
    }

    cout << "������ ����" << endl;
    //������ ����!! ������ �Լ��� ����
    return 0;
}

int main()
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartUp Fail\n");
        return 1;
    }

    cout << "Sever Program Open" << endl;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    HANDLE hThread;
    cout << "Ŭ���̾�Ʈ ���" << endl;
    while (1) {
        // accept()
        memset(&clientaddr, 0, addrlen);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ����
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);

        cout << client_sock << "����" << endl;
        //���� ���Ϳ� ���� �߰�
        User user(client_sock);
        users.push_back(user);

        //������ ��ǥ�� ������ ������
        send_first_pos(client_sock, user);
        //��ε� ĳ��Ʈ

        cout << "���� ������ User: ";
        for (auto u : users)
            cout << u.GetId() << "->";

        for (User u : users)
        {
            //send_Login_packet(u);
        }

        if (hThread == NULL) { closesocket(client_sock); }
        else {
            CloseHandle(hThread);
        }
    }
    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}