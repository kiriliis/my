#if defined(UNICODE)
#undef UNICODE // юникод не используем
#endif

#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <stdio.h>

// отключить некоторые предупреждения
#pragma warning(disable : 4996)
#pragma warning(disable : 4018)

// размеры окна по умолчанию
#define MAIN_WIDTH	480
#define MAIN_HEIGHT	320
#define TOP	10
#define LEFT 10
#define PADDING 10

#define STR_NEW "новое сообщение"
#define BUT_NEW_ICO "res//new.ico"
#define BUT_NEW	1001
#define STR_EDIT "редактировать сообщение"
#define BUT_EDIT_ICO "res//edit.ico"
#define BUT_EDIT 1002
#define STR_DELETE "удалить сообщение"
#define BUT_DELETE_ICO "res//delete.ico"
#define BUT_DELETE 1003
#define STR_CLEAR "очистить сообщения"
#define BUT_CLEAR_ICO "res//clear.ico"
#define BUT_CLEAR 1004

#define LIST_USERS	1005
#define LIST_MSGS	1006

#define BUT_WIDTH	40
#define BUT_HEIGHT	40

#define LIST_WIDTH	MAIN_WIDTH / 2 
#define LIST_HEIGHT	MAIN_HEIGHT - 4*TOP

#define CLASS_NAME		"TextChat"
#define STR_EMPTY		""
#define STR_TITLE		"Текстовый чат"
// файл настроек
#define CONF_NAME		"res\\chat_app.ini"

// настройки программы
typedef struct tagAppSettings{
	int main_x;
	int main_y;
	int main_w;
	int main_h;
}APP_SETTINGS, *pAPP_SETTINGS, **ppAPP_SETTINGS;


// глобальные переменные
HINSTANCE hInst;
HWND hwnd;
HWND hWndBut_New;
HWND hWndBut_Edit;
HWND hWndBut_Delete;
HWND hWndBut_Clear;
HWND hWnd_Users;
HWND hWnd_Messages;

char	szClassName[]	= {CLASS_NAME};
char	szWndName[]		= {STR_TITLE};
pAPP_SETTINGS pcs	= NULL; // настройки программы

// определения функций
BOOL InitMainWindow(HINSTANCE hInstance);
HWND CreateButIcon(HWND, INT, INT, INT, INT, PCHAR, INT, LPTSTR, HWND *);
HWND CreateListBox(HWND, INT, INT, INT, INT, PCHAR, INT);

//---------------------------  код для работы с файлом настроек программы ------------------------
#define MAIN_WINDOW "главное окно"
#define MAIN_WINDOW_X "левый верхний угол x"
#define MAIN_WINDOW_Y "левый верхний угол y"
#define MAIN_WINDOW_WIDTH	"ширина"
#define MAIN_WINDOW_HEIGHT "высота"

HANDLE OpenConfigFile(LPTSTR env_buf, DWORD dwSize){
	HANDLE fh = INVALID_HANDLE_VALUE;
	
	CopyMemory(env_buf, CONF_NAME, lstrlen(CONF_NAME));
	env_buf[lstrlen(CONF_NAME)] = '\x0';
	// пробуем открыть файл настроек в каталоге res		
	fh = CreateFile((LPCSTR)env_buf, GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);	

	return fh;
}

// читает настройки программы
int ReadSettings(pAPP_SETTINGS ps){
	HANDLE f_conf;
	LPTSTR env_buf;
	DWORD dwSize;
	int result = 0;
	DWORD dwBufLen = MAX_PATH;

	dwSize = MAX_PATH;

	env_buf = (LPTSTR)LocalAlloc(LPTR, dwSize + 1);	
	if(env_buf == NULL){
		return -1;
	}

	f_conf = OpenConfigFile(env_buf, dwSize);
	if(f_conf == INVALID_HANDLE_VALUE){
		return -1;
	}

	CloseHandle(f_conf);

	ps->main_x = GetPrivateProfileInt((LPCSTR)MAIN_WINDOW, (LPCSTR)MAIN_WINDOW_X, 0, env_buf);

	ps->main_y = GetPrivateProfileInt((LPCSTR)MAIN_WINDOW, (LPCSTR)MAIN_WINDOW_Y, 0, env_buf);

	ps->main_w = GetPrivateProfileInt((LPCSTR)MAIN_WINDOW, (LPCSTR)MAIN_WINDOW_WIDTH, 0, env_buf);

	ps->main_h = GetPrivateProfileInt((LPCSTR)MAIN_WINDOW, (LPCSTR)MAIN_WINDOW_HEIGHT, 0, env_buf);

	return 0;
}
// записывает настройки программы
int WriteSettings(pAPP_SETTINGS ps){
	HANDLE f_conf;
	LPTSTR env_buf;
	DWORD dw, dwSize;
	int result = 0;
	char int_buf[16];

	dwSize = MAX_PATH;

	env_buf = (LPTSTR)LocalAlloc(LPTR, dwSize + 1);	
	if(env_buf == NULL){
		//MessageBox(NULL, "text", "title", MB_ICONERROR);
		return -1;
	}

	f_conf = OpenConfigFile(env_buf, dwSize);
	if(f_conf == INVALID_HANDLE_VALUE){
		return -1;
	}
	CloseHandle(f_conf);

	sprintf(int_buf, "%d", ps->main_x);
	dw = WritePrivateProfileString(MAIN_WINDOW, MAIN_WINDOW_X, (LPCSTR)(int_buf), env_buf);

	sprintf(int_buf, "%d", ps->main_y);
	dw = WritePrivateProfileString(MAIN_WINDOW, MAIN_WINDOW_Y, (LPCSTR)(int_buf), env_buf);

	sprintf(int_buf, "%d", ps->main_w);
	dw = WritePrivateProfileString(MAIN_WINDOW, MAIN_WINDOW_WIDTH, (LPCSTR)(int_buf), env_buf);

	sprintf(int_buf, "%d", ps->main_h);
	dw = WritePrivateProfileString(MAIN_WINDOW, MAIN_WINDOW_HEIGHT, (LPCSTR)(int_buf), env_buf);

	LocalFree(env_buf);
	return 0;
}

//-----------------------------------------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hPrevInstance,char * szCmdLine, int iCmdShow)
{
	MSG         msg ;

	if(! InitMainWindow(hInstance) ) goto exit;

	for(;;)
    {
        while(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
        {
            if(msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT) break;

        WaitMessage();
    }
	// запись настроек программы
	WriteSettings(pcs);
exit:
	if(pcs != NULL )LocalFree(pcs);
    return ((int) msg.wParam);
}

// при изменении размера окна
void ResizeUI(HWND hwnd){
	RECT rc;

	if(! GetWindowRect(hwnd, &rc)){
		return;
	}

	pcs->main_x =  rc.left;
	pcs->main_y =  rc.top;
	pcs->main_w =  rc.right - rc.left;
	pcs->main_h =  rc.bottom - rc.top;

	SetWindowPos(hWnd_Users, HWND_TOP, 
						LEFT, TOP*2 + BUT_HEIGHT, 
						LIST_WIDTH, 
						pcs->main_h - TOP - BUT_HEIGHT*2, 
						SWP_DRAWFRAME);
	SetWindowPos(hWnd_Messages, HWND_TOP, 
						LEFT+LIST_WIDTH+PADDING, TOP*2 + BUT_HEIGHT, 
						pcs->main_w - LIST_WIDTH - LEFT*3 - PADDING, 
						pcs->main_h - TOP - BUT_HEIGHT*2, 
						SWP_DRAWFRAME);


}


LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
		case WM_CREATE :

			hWndBut_New = CreateButIcon(hwnd,
									LEFT, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_NEW, BUT_NEW, (LPTSTR)BUT_NEW_ICO, NULL);
			hWndBut_Edit = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_EDIT, BUT_EDIT, (LPTSTR)BUT_EDIT_ICO, NULL);
			hWndBut_Delete = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH*2, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_DELETE, BUT_DELETE, (LPTSTR)BUT_DELETE_ICO, NULL);
			hWndBut_Clear = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH*3, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_CLEAR, BUT_CLEAR, (LPTSTR)BUT_CLEAR_ICO, NULL);
			hWnd_Users = CreateListBox(hwnd, LEFT, TOP*2 + BUT_HEIGHT, LIST_WIDTH, LIST_HEIGHT, STR_EMPTY, LIST_USERS);
			hWnd_Messages = CreateListBox(hwnd, LEFT+LIST_WIDTH+PADDING, TOP*2 + BUT_HEIGHT, LIST_WIDTH, LIST_HEIGHT, STR_EMPTY, LIST_MSGS);
			
			break;
		case WM_SIZE:
			ResizeUI(hwnd);
			break;
		case WM_DESTROY :

                PostQuitMessage (0) ;
                break;

		default:

			break;
	}
									
  return DefWindowProc (hwnd, iMsg, wParam, lParam) ;

}

// инициализация главного окна приложения
BOOL InitMainWindow(HINSTANCE hInstance){
	WNDCLASS    wc;
	static LPCTSTR lpszAppName = (LPCSTR)"MAINICO";
	// выделяется память для настроек программы
	pcs = (pAPP_SETTINGS)LocalAlloc(LPTR, sizeof(APP_SETTINGS));
	// если не удалось выделить, то выход
	if(pcs == NULL){
		return FALSE;
	}
	// читаются настройки программы
	ReadSettings(pcs);

	// инициализация структуры для создания окна
	hInst = hInstance;
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;//(HICON)LoadImage(0, (LPCSTR)MAIN_ICO, IMAGE_ICON,0,0,LR_LOADFROMFILE);//LoadIcon(hInstance, lpszAppName);	 
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPCSTR)szClassName;

	RegisterClass(&wc);
	DEVMODE dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

	// определяютс яразмеры экрана
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	// установка размеров окна по умолчанию
	if(pcs->main_w == 0){
		pcs->main_w = MAIN_WIDTH;
	}
	if(pcs->main_h == 0){
		pcs->main_h = MAIN_HEIGHT;
	}

	if(pcs->main_x == 0){
		pcs->main_x = (dm.dmPelsWidth - pcs->main_w)/2;
	}

	if(pcs->main_y == 0){
		pcs->main_y = (dm.dmPelsHeight - pcs->main_h)/2;
	}
	
	hwnd = CreateWindow ((LPCSTR)szClassName, (LPCSTR)szWndName, 
				WS_EX_TOOLWINDOW | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX,
				pcs->main_x, pcs->main_y, pcs->main_w, pcs->main_h, 
				NULL, NULL, hInstance, NULL) ;  

	ShowWindow (hwnd, SW_SHOW);

	return TRUE;
}

//---------------------------------------------------------------------------------------
// элементы интерфейса пользователя

// список
HWND CreateListBox(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name, INT cmd )
{
    HWND ww;

    ww = CreateWindow( (LPCSTR)"LISTBOX", (LPCSTR)name, 
		WS_CHILD | WS_VISIBLE| WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY 
		| LBS_OWNERDRAWFIXED | LBS_HASSTRINGS, 
    x, y, w,  h, 
	hwnd2, (HMENU) cmd, (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE), NULL);

	if (ww != NULL) 
	{
		HDC hdc = GetDC(ww);
 
		HFONT fontTitle = CreateFont( (GetDeviceCaps(hdc, LOGPIXELSY) * 10) / 72,
								0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, (LPCSTR)"MS Sans Serif");

		SendMessage( ww, WM_SETFONT, (WPARAM)fontTitle, TRUE );
		ReleaseDC(ww, hdc );
		
		return ww; 
	}

	return NULL;
}

// кнопка
HWND CreateButIcon(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name, INT cmd, LPTSTR file, HWND *ptth)
{
    HWND ww;
	HANDLE himg;
	TOOLINFO ti;
	HINSTANCE hinst = (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE);


    ww = CreateWindowEx(
						WS_EX_CONTROLPARENT, (LPCSTR)"BUTTON", (LPCSTR)name,
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON, 
						x, y, w, h, hwnd2, (HMENU) cmd,
						hinst,
						NULL);

	SendMessage(ww, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	himg = LoadImage(0, file, IMAGE_ICON,0,0,LR_LOADFROMFILE);
	if(himg != NULL){
		SendMessage( ww, BM_SETIMAGE, IMAGE_ICON, (long)himg );
	}

	HWND hTooltip = CreateWindow (
									TOOLTIPS_CLASS, 
									(LPCSTR)NULL, 
									WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP, 
									CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
									NULL, (HMENU)NULL, hinst, NULL
								);
 
    memset (&ti, 0, sizeof TOOLINFO);
    ti.cbSize = sizeof TOOLINFO;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.uId = (UINT)ww;
    ti.lpszText = (LPSTR)name;
    ti.hinst = hinst;
    SendMessage (hTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);  

	if( ptth != NULL ){
		*ptth = hTooltip;
	}

    return ww;
}

//---------------------------------------------------------------------------------------
