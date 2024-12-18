#define _CRT_SECURE_NO_WARNINGS
#include "iostream"
#include "vector"
#include "ctime"
#include "cmath"
#include "string"
#include "Windows.h"
#include "TlHelp32.h"

using namespace std;


constexpr uintptr_t offxhead = 0x4;
constexpr uintptr_t offyhead = 0x8;
constexpr uintptr_t offzhead = 0xC;//height
constexpr uintptr_t offxleg = 0x28;
constexpr uintptr_t offyleg = 0x2C;
constexpr uintptr_t offzleg = 0x30;//height
constexpr uintptr_t offhp = 0xEC;//HP
constexpr uintptr_t offentity = 0x191FCC;
constexpr uintptr_t offbots = 0x109430;
constexpr uintptr_t offmat = 0x17DFD0;
constexpr uintptr_t offname = 0x205;
constexpr uintptr_t offfps = 0x17C1F8;
constexpr uintptr_t offfov = 0x18A7CC;
constexpr uintptr_t offcamx = 0x34;
constexpr uintptr_t offcamy = 0x38;
constexpr uintptr_t offmyclass = 0x17E0A8;

#define TEXT_POS - 25.0f
#define TEXT_POSLEFT 25.0f
#define NUM_POSLEFT 15.0f
#define HEALTH_LEFT 5.0f
#define HEALTH_RIGHT 5.0f
struct vec3
{
	float x, y, z;
};
struct vec4
{
	float x, y, z, w;
};
struct vec2
{
	float x, y;
};

vec3 headpos, legpos, mylegpos, myheadpos;
vec2 screenheadpos, screenlegpos;

namespace global
{
	DWORD id;
	HWND window;
	const wchar_t* gamename = L"AssaultCube";
	HANDLE openpr;
	uintptr_t baseadd;
	const wchar_t* modulename = L"ac_client.exe";
	float matrix[16];
	HDC assaultdc;
	HDC overlaydc;
	HWND overlaywindow;
	uintptr_t botsadr = 0x591FD4;
	DWORD bots = 0;
	uintptr_t entitylist;
	int gameheight = 0;
	int gamewidth = 0;

	RECT rectr;
	DWORD readedbot;

	HPEN textpen;
	HPEN circlepen;
	HPEN scopepen;
	HPEN linepen;
	HPEN boxpen;
	HPEN distancepen;
	HPEN hpboxpen;

	uintptr_t entityadd = 0x191FCC;
	COLORREF color = RGB(255, 0, 0);
	COLORREF color2 = RGB(0, 255, 0);
	COLORREF color3 = RGB(0, 0, 255);
	COLORREF color4 = RGB(255, 255, 0);
	COLORREF color5 = RGB(255, 0, 255);
	COLORREF color6 = RGB(0, 255, 255);
	COLORREF color7 = RGB(255, 128, 0);

	HGDIOBJ oldpen;
	HBRUSH oldbrush;
	uintptr_t cameraxadd = 0x92575C;
	uintptr_t camerayadd = 0x925760;
	char botname[258];
	float fps;
	int boxheight;
	int boxwidth;
	float distance;
	char distancetext[100];
	DWORD myclassadd;
	DWORD health;
	uintptr_t fovadd;
	float fov;
	float depth;
	float camx;
	float camy;
}
namespace bin
{
	uintptr_t temp1;
	uintptr_t temp2;
	uintptr_t temp3;
	uintptr_t temp4;
	uintptr_t temp5;
	uintptr_t temp6;


	int lfcx;
	int lfcy;


}


#define fDEBUG

using global::openpr;
using global::baseadd;
using global::entitylist;
using global::rectr;
using global::readedbot;


bool startall();




bool findwindow()
{

	global::window = FindWindowW(0, global::gamename);

	if (!global::window)
	{
		cout << "запустите игру\n";
		Sleep(1100);
		findwindow();
	}
	else
	{
		system("cls");
		cout << "INSERT to activate" << endl;
		cout << "END to exit" << endl;
		return true;
	}

	return true;
}
bool findid()
{
	if (!GetWindowThreadProcessId(global::window, &global::id))
	{
		cerr << "ошибка получения айди процесса\n";
		return false;
	}
#ifdef DEBUG
	cout << "айди процесс получен " << dec << global::id << endl;
#endif // DEBUG

	return true;

}
bool openproc()
{
	global::openpr = OpenProcess(PROCESS_ALL_ACCESS, FALSE, global::id);
	if (!global::openpr)
	{
		cerr << "ошибка открытия процесса\n";
		return false;
	}
#ifdef DEBUG
	cout << "процесс открыт\n";
#endif // DEBUG
	return TRUE;
}
DWORD getbaseadd()
{
	const wchar_t* modulename = global::modulename;
	HANDLE hshot;
	DWORD sizemodule;
	hshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, global::id);


	if (hshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(entry);
		if (Module32First(hshot, &entry))
		{
			do
			{
				if (_wcsicmp((entry.szModule), modulename) == 0)
				{
#ifdef DEBUG
					cout << "модуль найден\n";
					cout << "адрес базового адреса: " << hex << (uintptr_t)entry.modBaseAddr << endl;
#endif // DEBUG
					global::baseadd = (uintptr_t)entry.modBaseAddr;
					break;
				}

			} while (Module32Next(hshot, &entry));
		}
	}



	CloseHandle(hshot);
	return global::baseadd;
}
bool getres()
{

	
	while (IsIconic(global::window))
	{
		Sleep(160);
	}

	RECT rectwindow;
	GetClientRect(global::window, &rectwindow);
	global::gameheight = rectwindow.bottom - rectwindow.top;
	global::gamewidth = rectwindow.right - rectwindow.left;




	global::assaultdc = GetDC(global::window);
	return true;
}
inline bool w2s(const vec3& pos, vec2* screen, float viewMatrix[16], int gamewidth, int gameheight)
{

	float clipX = pos.x * viewMatrix[0] + pos.y * viewMatrix[4] + pos.z * viewMatrix[8] + viewMatrix[12];
	float clipY = pos.x * viewMatrix[1] + pos.y * viewMatrix[5] + pos.z * viewMatrix[9] + viewMatrix[13];
	float clipW = pos.x * viewMatrix[3] + pos.y * viewMatrix[7] + pos.z * viewMatrix[11] + viewMatrix[15];
	if (clipW < 0.00001f)
	{
		return false;
	}
	global::depth = clipW;

	// Нормализация
	float ndcX = clipX / clipW;
	float ndcY = clipY / clipW;
	// Преобразование в экранные координаты
	screen->x = (gamewidth / 2.0f) + (gamewidth / 2.0f) * ndcX;
	screen->y = (gameheight / 2.0f) - (gameheight / 2.0f) * ndcY;
	return true;
}



inline bool textrender(vec2& screenposition, int gamewidth, int gameheight, char botname[], HDC gamedc)
{
	//global::textpen = CreatePen(PS_SOLID, 2, global::color);//ручка для рендера текста 

	//texpen не надо
	SetTextColor(gamedc, global::color);
	SetBkMode(gamedc, TRANSPARENT);
	TextOutA(gamedc, screenposition.x - TEXT_POSLEFT, screenposition.y + TEXT_POS, botname, strlen(botname));

	return true;
}
inline bool circlerender(vec2& screenposition, int gamewidth, int gameheight, HDC gamedc, float fov)
{
	global::circlepen = CreatePen(PS_SOLID, 2, global::color2);//ручка для рендера круга вокруг головы врагов

	float scaleradius = 1.6f * (fov / global::depth);
	if (scaleradius > 57)
	{
		scaleradius = 57;
	}

	global::oldbrush = (HBRUSH)SelectObject(gamedc, GetStockObject(NULL_BRUSH));
	global::oldpen = SelectObject(gamedc, global::circlepen);

	Ellipse(gamedc, screenposition.x - scaleradius, screenposition.y - scaleradius,
		screenposition.x + scaleradius, screenposition.y + scaleradius);//на бошке противника 

	SelectObject(gamedc, global::oldbrush);
	SelectObject(gamedc, global::oldpen);
	return true;
}
inline bool scoperender(int gamewidth, int gameheight, HDC gamedc)
{
	global::scopepen = CreatePen(PS_SOLID, 2, global::color3);//ручка для рендера прицела в цетре экрана

	//в центре экрана, прицел

	global::oldpen = SelectObject(gamedc, global::scopepen);
	global::oldbrush = (HBRUSH)SelectObject(gamedc, global::scopepen);

	SelectObject(gamedc, global::scopepen);
	Ellipse(gamedc, (gamewidth / 2) - 2, (gameheight / 2) - 2, (gamewidth / 2) + 2, (gameheight / 2) + 2);

	SelectObject(gamedc, global::oldpen);
	SelectObject(gamedc, global::oldbrush);

	return true;
}
inline bool linerender(vec2& screenposition,vec2& screenlegpositiin, int gamewidth, int gameheight, HDC gamedc)
{
	global::linepen = CreatePen(PS_SOLID, 1, global::color4);//ручка для рендера линий
	global::oldpen = SelectObject(gamedc, global::linepen);
	MoveToEx(gamedc, gamewidth / 2, gameheight, 0);
	LineTo(gamedc, screenlegpositiin.x, screenlegpositiin.y);

	SelectObject(gamedc, global::oldpen);

	return true;
}
inline bool boxrender(vec2& screenheadpos, vec2& screenlegpos, int boxwidth, int boxheight, int gamewidth, int gameheight, HDC gamendc)
{
	global::boxpen = CreatePen(PS_SOLID, 2, global::color5);//ручка для рендера боксов вокруг врагов
	HBRUSH nullbrush = (HBRUSH)SelectObject(gamendc, GetStockObject(NULL_BRUSH));
	SelectObject(gamendc, global::boxpen);
	global::oldbrush = nullbrush;//сохраняем старую кисть
	global::oldpen = global::boxpen;//сохраняем старую ручку
	//ошибка в размере боксов
	Rectangle(gamendc, (screenheadpos.x - boxwidth / 2), screenheadpos.y, (screenlegpos.x + boxwidth / 2), screenlegpos.y);
	//уменьшается только правая сторона, что-то не так там

	SelectObject(gamendc, global::oldpen);
	SelectObject(gamendc, global::oldbrush);
	return true;
}
inline bool distancerender(vec3& mylegpos, vec3& legposent, int gamewidth, int gameheight, vec2& textscreenpos, HDC gamedc)
{ 
	//textpen не надо
	global::distance = sqrt(
		(legposent.x - mylegpos.x) * (legposent.x - mylegpos.x) +
		(legposent.y - mylegpos.y) * (legposent.y - mylegpos.y) +
		(legposent.z - mylegpos.z) * (legposent.z - mylegpos.z));
	//возводим в степень дабы убрать - число при минусовых координатах, после степеня используем корень, таким
	//образом шлем минус нахуй
	string str = to_string(global::distance);

	strcpy(global::distancetext, str.c_str());

	SetTextColor(gamedc, global::color6);
	SetBkMode(gamedc, TRANSPARENT);
	TextOutA(gamedc, textscreenpos.x - NUM_POSLEFT, textscreenpos.y, global::distancetext, strlen(global::distancetext) - 5);


	return true;
}
inline bool hprender(vec2& legposent, vec2& headposent, int gamewidth, int gameheight, HDC gamendc, DWORD enthealth)
{
	if (global::hpboxpen)
	{
		DeleteObject(global::hpboxpen);
	}
	//то бишь отрисовка была не верной из-за того что я не обхватывал все значения
	//2) я должен понимать что createobj и deleteobj это как выделение памяти и очистка ее
	if (enthealth == 100)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(0, 255, 0));
	}
	else if (enthealth >= 80)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(150, 183, 140)); 
	}
	else if (enthealth >= 60)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(132, 211, 80)); 
	}
	else if (enthealth >= 40)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(242, 255, 29)); 
	}
	else if (enthealth >= 20)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(255, 174, 10)); 
	}
	else if (enthealth > 0)
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(255, 0, 0)); 
	}
	else
	{
		global::hpboxpen = CreatePen(PS_SOLID, 8, RGB(0, 0, 0));
	}

	// Расчет высоты бокса
	float genboxsize = (legposent.y - headposent.y); // Полная высота бокса
	float boxsize = genboxsize * (enthealth / 100.0f); 

	float height = boxsize;


	global::oldpen = SelectObject(gamendc, global::hpboxpen);


	float left = ((HEALTH_LEFT * global::fov) / global::depth);
	float right = ((HEALTH_RIGHT * global::fov) / global::depth);


	MoveToEx(gamendc, legposent.x + right, legposent.y, 0);
	LineTo(gamendc, legposent.x + right, legposent.y - height);

	// Восстановление старой ручки
	SelectObject(gamendc, global::oldpen);

	return true;
}

inline bool readvalues(vec3& entheadpos, vec3& entlegpos, vec3& mylegpos, vec3& myheadpos, vec2& toscreenheadpos, vec2& toscreenlegpos, int gamewidth, int gameheight, HDC gamendc)
{
	ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offfov), &global::fov, 4, 0);//fov

	ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offbots), &bin::temp1, 4, 0);
	ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offmyclass), &global::myclassadd, 4, 0);//temp5

	while (!GetAsyncKeyState(VK_END))
	{
		ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offentity), &global::entityadd, 4, 0);
		ReadProcessMemory(global::openpr, (PDWORD)(bin::temp1), &global::bots, 4, 0);
		ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offmat), &global::matrix, sizeof(global::matrix), 0);
		ReadProcessMemory(global::openpr, (LPVOID)(global::baseadd + offfps), &global::fps, 4, 0);

		ReadProcessMemory(global::openpr, (LPVOID)(global::myclassadd + offcamx), &global::camx, 4, 0);
		ReadProcessMemory(global::openpr, (LPVOID)(global::myclassadd + offcamy), &global::camy, 4, 0);

		for (int i(0); i < global::bots; i++)
		{
			if (GetAsyncKeyState(VK_ADD))
			{
				startall();
			}
			ReadProcessMemory(global::openpr, (LPVOID)((LPDWORD)global::entityadd + i), &bin::temp2, 4, 0);

			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offxhead), &entheadpos.x, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offyhead), &entheadpos.y, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offzhead), &entheadpos.z, 4, 0);



			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offxleg), &entlegpos.x, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offyleg), &entlegpos.y, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offzleg), &entlegpos.z, 4, 0);

			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offhp), &global::health, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(bin::temp2 + offname), &global::botname, sizeof(global::botname), 0);

			ReadProcessMemory(global::openpr, (LPVOID)(global::myclassadd + offxleg), &mylegpos.x, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(global::myclassadd + offyleg), &mylegpos.y, 4, 0);
			ReadProcessMemory(global::openpr, (LPVOID)(global::myclassadd + offzleg), &mylegpos.z, 4, 0);



			if (global::health > 0 && global::health <= 100)//жив ли игрок
			{
				if (w2s(entheadpos, &toscreenheadpos, global::matrix, gamewidth, gameheight))
				{
					if (w2s(entlegpos, &toscreenlegpos, global::matrix, gamewidth, gameheight))
					{
						global::boxheight = (toscreenlegpos.y - toscreenheadpos.y);
						global::boxwidth = global::boxheight / 2.4;
						if (GetAsyncKeyState(VK_ADD))
						{
							startall();
						}
						if (global::fps < 60.0f)
						{
							hprender(toscreenlegpos, toscreenheadpos, gamewidth, gameheight, gamendc, global::health);
							scoperender(gamewidth, gameheight, gamendc);
							circlerender(toscreenheadpos, gamewidth, gameheight, gamendc, global::fov);
							textrender(toscreenheadpos, gamewidth, gameheight, global::botname, gamendc);
							linerender(toscreenheadpos,screenlegpos, gamewidth, gameheight, gamendc);
							boxrender(toscreenheadpos, toscreenlegpos, global::boxwidth, global::boxheight, gamewidth, gameheight, gamendc);
							distancerender(mylegpos, entlegpos, gamewidth, gameheight, toscreenlegpos, gamendc);

							DeleteObject(global::scopepen);
							DeleteObject(global::textpen);
							DeleteObject(global::circlepen);
							DeleteObject(global::scopepen);
							DeleteObject(global::linepen);
							DeleteObject(global::boxpen);
							DeleteObject(global::hpboxpen);
						}
						else
						{
							scoperender(gamewidth, gameheight, gamendc);
							circlerender(toscreenheadpos, gamewidth, gameheight, gamendc, global::fov);
							textrender(toscreenheadpos, gamewidth, gameheight, global::botname, gamendc);
							linerender(toscreenheadpos,screenlegpos, gamewidth, gameheight, gamendc);
							boxrender(toscreenheadpos, toscreenlegpos, global::boxwidth, global::boxheight, gamewidth, gameheight, gamendc);
							distancerender(mylegpos, entlegpos, gamewidth, gameheight, screenlegpos, gamendc);

							DeleteObject(global::scopepen);
							DeleteObject(global::textpen);
							DeleteObject(global::circlepen);
							DeleteObject(global::scopepen);
							DeleteObject(global::linepen);
							DeleteObject(global::boxpen);
							//second render because of big fps, DGI is shit!
							scoperender(gamewidth, gameheight, gamendc);
							circlerender(toscreenheadpos, gamewidth, gameheight, gamendc, global::fov);
							textrender(toscreenheadpos, gamewidth, gameheight, global::botname, gamendc);
							linerender(toscreenheadpos, screenlegpos, gamewidth, gameheight, gamendc);
							boxrender(toscreenheadpos, toscreenlegpos, global::boxwidth, global::boxheight, gamewidth, gameheight, gamendc);
							distancerender(mylegpos, entlegpos, gamewidth, gameheight, screenlegpos, gamendc);

							DeleteObject(global::scopepen);
							DeleteObject(global::textpen);
							DeleteObject(global::circlepen);
							DeleteObject(global::scopepen);
							DeleteObject(global::linepen);
							DeleteObject(global::boxpen);
						}
					}
				}
			}
		}
	}


	return true;
}

bool startall()
{
	findwindow();
	findid();
	openproc();
	getbaseadd();
	getres();

	readvalues(headpos, legpos, mylegpos, myheadpos, screenheadpos, screenlegpos, global::gamewidth,
		global::gameheight, global::assaultdc);
	return true;
}

void main()
{
	setlocale(LC_ALL, "ru");
	startall();
}



//I strongly advise you to use 30 fps, because gdi will not have time to render.
//#discord borncock - questions here
