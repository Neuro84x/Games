#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>

#define _WIN32_WINNT 0x0A00

#define AddEnemy(a,b) ObjectInit(NewObject(),a,b,40,40,'e')

typedef struct SPoint {
	float x, y;
} TPoint;

typedef struct SObject {
	TPoint pos;
	TPoint size;
	TPoint speed;
	COLORREF brush;
	char oType;
	float range;
	float vecSpeed;
	BOOL isDel;
}TObject, * PObject;

RECT rct;
TObject player;
PObject enemies = NULL;
int enemCount = 0;
TPoint cam;
BOOL newGame = FALSE;

void SetCameraFocus(TObject obj) {
	cam.x = obj.pos.x - rct.right / 2;
	cam.y = obj.pos.y - rct.bottom / 2;
}


TPoint GetPoint(float x, float y) {

	TPoint pt;

	pt.x = x;
	pt.y = y;

	return pt;
}

void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType) {
	obj->pos = GetPoint(xPos, yPos);
	obj->size = GetPoint(width, height);
	obj->brush = RGB(186, 22, 227);
	obj->speed = GetPoint(0, 0);
	obj->oType = objType;
	obj->isDel = FALSE;

	if (objType == 'e') obj->brush = RGB(255, 0, 0);
}

PObject NewObject() {
	enemCount++;
	enemies = realloc(enemies, sizeof(*enemies) * enemCount);

	return enemies + enemCount - 1;
}

void ObjectSetDestPoint(TObject* obj, float xPos, float yPos, float vecSpeed) {

	TPoint xyLen = GetPoint(xPos - obj->pos.x, yPos - obj->pos.y);

	float dxy = (float)sqrt((pow(xyLen.x,2) + (pow(xyLen.y,2))));

	obj->speed.x = xyLen.x / dxy * vecSpeed;
	obj->speed.y = xyLen.y / dxy * vecSpeed;
	obj->vecSpeed = vecSpeed;
}

void ObjectShow(TObject obj, HDC dc) {
	SelectObject(dc, GetStockObject(DC_PEN));
	SetDCPenColor(dc, RGB(0, 0, 0));

	SelectObject(dc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(dc, obj.brush);

	BOOL(__stdcall * shape)(HDC, int, int, int, int);
	shape = obj.oType == 'e' ? Ellipse : Rectangle;
	shape(dc, (int)(obj.pos.x - cam.x), (int)(obj.pos.y - cam.y),
		(int)(obj.pos.x + obj.size.x - cam.x), (int)(obj.pos.y + obj.size.y - cam.y));
}

void ObjectMove(TObject* obj) {
	if (obj->oType == 'e') {
		if (rand() % 20 == 1) {
			static float enemySpeed = 7.5;
			ObjectSetDestPoint(obj, player.pos.x, player.pos.y, enemySpeed);
		}
		if (ObjectCollisison(*obj, player)) {
			newGame = TRUE;
		}
	}

	if (obj->oType == '1') {
		obj->range -= obj->vecSpeed;
		if (obj->range < 0) {
			obj->isDel = TRUE;
		}
		for (int i = 0; i < enemCount; ++i) {
			if ((enemies[i].oType == 'e') && (ObjectCollisison(*obj, enemies[i]))) {
				enemies[i].isDel = TRUE;
				obj->isDel = TRUE;
			}
		}
	}

	obj->pos.x += obj->speed.x;
	obj->pos.y += obj->speed.y;
}
void AddBullet(float xPos, float yPos, float x, float y) {
	PObject obj = NewObject();

	ObjectInit(obj, xPos, yPos, 10, 10, '1');
	ObjectSetDestPoint(obj, x, y, 4);
	obj->range = 300;
}

BOOL ObjectCollisison(TObject o1, TObject o2) {
	return  ((o1.pos.x + o1.size.x) > o2.pos.x)
			&& 
			(o1.pos.x < (o2.pos.x + o2.size.x)) 
			&&
			((o1.pos.y + o1.size.y) > o2.pos.y) 
			&& 
			(o1.pos.y < (o2.pos.y + o2.size.y));
}

void GenNewEnemy() {
	static int rad = 300;
	int pos1 = (rand() % 2 == 0 ? -rad : rad);
	int pos2 = (rand() % (rad * 2)) - rad;
	int k = rand() % 50;
	if (k == 1) {
		AddEnemy(player.pos.x + pos1, player.pos.y + pos2);
	}
	if (k == 2) {
		AddEnemy(player.pos.x + pos2, player.pos.y + pos1);
	}
}

void DelObjects() {
	int i = 0;
	while (i < enemCount) {
		if (enemies[i].isDel) {
			enemCount--;
			enemies[i] = enemies[enemCount];
			enemies = realloc(enemies, sizeof(*enemies) * enemCount);
		}
		else {
			i++;
		}
	}
}

void PlayerControl() {
	static int playerSpeed = 10;
	player.speed.x = 0;
	player.speed.y = 0;

	if (GetKeyState('W') < 0) {
		player.speed.y = (float)-playerSpeed;
	}
	if (GetKeyState('A') < 0) {
		player.speed.x = (float)-playerSpeed;
	}
	if (GetKeyState('S') < 0) {
		player.speed.y = (float)playerSpeed;
	}
	if (GetKeyState('D') < 0) {
		player.speed.x = (float)playerSpeed;
	}

	if ((player.speed.x != 0) && (player.speed.y != 0)) {
		player.speed = GetPoint(player.speed.x * (float)0.7, player.speed.y * (float)0.7);
	}
}

void WinShow(HDC dc) {
	HDC memDC = CreateCompatibleDC(dc);
	HBITMAP memBM = CreateCompatibleBitmap(dc, rct.right - rct.left, rct.bottom - rct.top);
	SelectObject(memDC, memBM);

	SelectObject(memDC, GetStockObject(DC_PEN));
	SetDCPenColor(memDC, RGB(255, 255, 255));
	SelectObject(memDC, GetStockObject(DC_BRUSH));
	SetDCBrushColor(memDC, RGB(0, 0, 0));

	static int rectSize = 200;
	int dx = (int)(cam.x) % rectSize;
	int dy = (int)(cam.y) % rectSize;

	for (int i = -1; i < (rct.right / rectSize) + 2; ++i) {
		for (int j = -1; j < (rct.bottom / rectSize) + 2; ++j) {
			Rectangle(memDC, -dx + (i * rectSize), -dy + (j * rectSize),
				-dx + ((i + 1) * rectSize), -dy + ((j + 1) * rectSize));
		}
	}


	ObjectShow(player, memDC);

	for (int i = 0; i < enemCount; ++i) {
		ObjectShow(enemies[i], memDC);
	}

	BitBlt(dc, 0, 0, rct.right - rct.left, rct.bottom - rct.top, memDC, 0, 0, SRCCOPY);
	DeleteDC(memDC);
	DeleteObject(memBM);
}

void WinInit() {
	newGame = FALSE;
	enemCount = 0;

	enemies = realloc(enemies, 0);


	ObjectInit(&player, 100, 100, 40, 40, 'p');
}

void WinMove() {
	if (newGame) {
		WinInit();
	}

	PlayerControl();
	ObjectMove(&player);
	SetCameraFocus(player);

	for (int i = 0; i < enemCount; ++i) {
		ObjectMove(enemies + i);
	}

	GenNewEnemy();
	DelObjects();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
	}
	else if (message == WM_KEYDOWN) {
		printf("code = %d\n", wparam);
	}
	else if (message == WM_CHAR) {
		printf("%c\n", wparam);
	}
	else if (message == WM_SIZE) {
		GetClientRect(hwnd, &rct);
	}
	else if (message == WM_MOUSEMOVE) {
		int xPos = LOWORD(lparam);
		int yPos = HIWORD(lparam);
		printf("mouse [%d,%d]\n", xPos, yPos);
	}
	else if (message == WM_LBUTTONDOWN) {
		printf("LMB down\n");
		int xPos = LOWORD(lparam);
		int yPos = HIWORD(lparam);
		AddBullet(player.pos.x + player.size.x / 2, player.pos.y + player.size.y / 2, xPos + cam.x, yPos + cam.y);
	}
	else if (message == WM_RBUTTONDOWN) {
		printf("RMB down\n");
	}
	else {
		return DefWindowProcA(hwnd, message, wparam, lparam);
	}
}

int main() {

	WNDCLASSA wcl;
	memset(&wcl, 0, sizeof(WNDCLASSA));
	wcl.lpszClassName = "my Window";
	wcl.lpfnWndProc = WndProc;
	wcl.hCursor = LoadCursorA(NULL, IDC_CROSS);

	RegisterClassA(&wcl);

	HWND hwnd;
	hwnd = CreateWindowExA(0, "my Window", "My Window", WS_OVERLAPPEDWINDOW, 10, 10, 640, 480, 0, 0, 0, 0);

	//HDC dc = GetDCEx(hwnd, DCX_INTERSECTRGN, DCX_CLIPCHILDREN);
	HDC dc = GetDC(hwnd);

	ShowWindow(hwnd, SW_SHOWNORMAL);

	WinInit();

	MSG msg;
	while (1) {
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			WinMove();
			WinShow(dc);
			Sleep(5);
		}
	}

	return 0;
}