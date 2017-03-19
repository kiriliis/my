#if defined(UNICODE)
#undef UNICODE // юникод не используем
#endif

#include <Windowsx.h>
#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <stdio.h>

// отключить некоторые предупреждения
#pragma warning(disable : 4996)
#pragma warning(disable : 4018)

// размеры окна по умолчанию
#define MAIN_WIDTH		480
#define MAIN_HEIGHT		320
#define TOP				10
#define LEFT			10
#define PADDING			10
#define LABEL_WIDTH		50
#define LABEL_HEIGHT	20
#define TEXT_WIDTH		200
#define TEXT_HEIGHT		20

#define STR_IN			"вход в чат"
#define BUT_IN_ICO		"res//in.ico"
#define BUT_IN			1001
#define STR_OUT		"выход из чата"
#define BUT_OUT_ICO	"res//out.ico"
#define BUT_OUT		1002
#define STR_EXIT		"завершить работу"
#define BUT_EXIT_ICO	"res//exit.ico"
#define BUT_EXIT		1003
#define STR_CLEAR		"очистить сообщения"
#define BUT_CLEAR_ICO	"res//clear.ico"
#define BUT_CLEAR		1004
#define STR_MSG			"отправить сообщение"
#define BUT_MSG_ICO		"res//msg.ico"
#define BUT_MSG			1012

#define BUT_NIKE		1009
#define BUT_AVATARE		1010


#define LIST_USERS		1005
#define LIST_MSGS		1006

#define TXT_NIKE		1007
#define TXT_AVATARE		1008



#define TXT_MSG			1011

#define BUT_WIDTH		40
#define BUT_HEIGHT		40

#define LIST_WIDTH	MAIN_WIDTH / 2 
#define LIST_HEIGHT	MAIN_HEIGHT - 4*TOP
#define LIST_USERS_ITEM_HEIGHT	40

#define CLASS_NAME		"TextChat"
#define STR_EMPTY		""
#define STR_TITLE		"Текстовый чат"
#define STR_NIKE		"ник"
#define STR_AVATARE		"аватар"
#define STR_DOTS		"..."

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
HWND hWndBut_In;
HWND hWndBut_Out;
HWND hWndBut_Exit;
HWND hWndBut_Clear;
HWND hWnd_Users;
HWND hWnd_Messages;
HWND hWnd_Nike;
HWND hWnd_Avatare;
HWND hWnd_Msg;
HWND hWnd_ButMsg;

char	szClassName[]	= {CLASS_NAME};
char	szWndName[]		= {STR_TITLE};
pAPP_SETTINGS pcs	= NULL; // настройки программы
char nike_buf[1024]; // ник пользователя

// определения функций
BOOL InitMainWindow(HINSTANCE hInstance);
HWND CreateButIcon(HWND, INT, INT, INT, INT, PCHAR, INT, LPTSTR, HWND *);
HWND CreateListBox(HWND, INT, INT, INT, INT, PCHAR, INT);
HWND CreateLabel(HWND, INT, INT, INT, INT, PCHAR);
HWND CreateText(HWND, INT, INT, INT, INT, PCHAR, INT);
HWND CreateMultiRowText(HWND, INT, INT, INT, INT, PCHAR, INT);
HWND CreateBut(HWND, INT, INT, INT, INT, PCHAR, INT);
void DrawItem(HWND hWndParent, LPDRAWITEMSTRUCT lpDrawItem);
void SelectAvatareFile();

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

	ZeroMemory(nike_buf, 1024);
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

	SetWindowPos(hWnd_Msg, HWND_TOP,
						LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH+BUT_WIDTH/2+PADDING, 
						TOP, 
						pcs->main_w - LIST_WIDTH - LEFT*3 - PADDING - TEXT_WIDTH - BUT_WIDTH / 2 * 3, 
						TEXT_HEIGHT*2, 
						SWP_DRAWFRAME);

	SetWindowPos(hWnd_ButMsg, HWND_TOP,
						LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH+BUT_WIDTH/2+PADDING+
						(pcs->main_w - LIST_WIDTH - LEFT*3 - PADDING - TEXT_WIDTH - BUT_WIDTH / 2 * 3), 
						TOP, 
						BUT_WIDTH, 
						BUT_HEIGHT, 
						SWP_DRAWFRAME);
	


}



void DrawBitmaps(HDC hDC, RECT rect)
{

    HDC memdc, memdc1;
    memdc=CreateCompatibleDC(hDC);
    memdc1=CreateCompatibleDC(hDC);

    HBITMAP bitmap,bitmap1, oldbitmap, oldbitmap1;

//    bitmap=LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_REDX));
//    bitmap1=LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_REDXMASK));

    oldbitmap=(HBITMAP)SelectObject(memdc,bitmap);
    oldbitmap1=(HBITMAP)SelectObject(memdc1,bitmap1);

    //draw the bitmap
    BitBlt(hDC,rect.left+2,rect.top+2,16,16,memdc,0,0,SRCINVERT);
    BitBlt(hDC,rect.left+2,rect.top+2,16,16,memdc1,0,0,SRCAND);
    BitBlt(hDC,rect.left+2,rect.top+2,16,16,memdc,0,0,SRCINVERT);

    //cleanup
    SelectObject(memdc,oldbitmap);
    SelectObject(memdc1,oldbitmap1);
}


BOOL DrawUserItem(LPDRAWITEMSTRUCT Item){
	if (Item->itemID == -1){
		return FALSE;
	}

	HDC dc = Item->hDC;
    
	SetBkColor(Item->hDC, 0xFFFFFF);
    FillRect(Item->hDC, &Item->rcItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetTextColor(Item->hDC, 0x000000);
    //if (Item->itemState == ODS_SELECTED)
        
    int len = SendMessage(Item->hwndItem , LB_GETTEXTLEN, Item->itemID, 0);
    if (len > 0)
    {
        LPTSTR lpBuff = new TCHAR[len+1];
        len = SendMessage(Item->hwndItem , LB_GETTEXT, Item->itemID, (LPARAM)lpBuff);
        if (len > 0)
            TextOut(Item->hDC, Item->rcItem.left, Item->rcItem.top, lpBuff, len);
        delete[] lpBuff;
    }
    
	//draw the bitmap
    //DrawBitmaps(dc, Item->rcItem);

    if (Item->itemState & ODS_FOCUS)
    {
        DrawFocusRect(Item->hDC, &Item->rcItem);
    }

    return TRUE;
}

// установка имени пользователя
void SetNike(){
	char buf[2048];	

	SendMessage(hWnd_Nike, WM_GETTEXT, (WPARAM)1023, (LPARAM)nike_buf);

	sprintf(buf, "%s (%s)", STR_TITLE, nike_buf);

	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)buf);
}

// отправка сообщения
void SendTextMessage(){
	SYSTEMTIME st;
	char buffer[2048]; // буфер для сообщения
	int pos;

	ZeroMemory(buffer, 2048);

	GetLocalTime(&st);
	sprintf(buffer, "[%02d.%02d.%04d_%02d:%02d:%02d] (%s)", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, nike_buf);
	pos = lstrlen(buffer);
	buffer[pos] = ' ';
	// получить текст сообщения в буфер
	SendMessage(hWnd_Msg, WM_GETTEXT, (WPARAM)2047-pos-2, (LPARAM)&buffer[pos+1]);
	// время прихода сообщения
	// добавть текст в список сообщений
	// это пока так работает, в дальнейшем сообщение будет отправляться в сеть
	// и в список попадать из сети
	SendMessage(hWnd_Messages, LB_INSERTSTRING, 0, (LPARAM)buffer);
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	
	LPDRAWITEMSTRUCT Item;
	switch (iMsg)
	{
		case WM_CREATE : // создаются элементы графического интерфейса

			hWndBut_In = CreateButIcon(hwnd,
									LEFT, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_IN, BUT_IN, (LPTSTR)BUT_IN_ICO, NULL);
			hWndBut_Out = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_OUT, BUT_OUT, (LPTSTR)BUT_OUT_ICO, NULL);
			hWndBut_Exit = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH*2, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_EXIT, BUT_EXIT, (LPTSTR)BUT_EXIT_ICO, NULL);
			hWndBut_Clear = CreateButIcon(hwnd,
									LEFT+BUT_WIDTH*3, TOP,
									BUT_WIDTH,BUT_HEIGHT,
									(PCHAR)STR_CLEAR, BUT_CLEAR, (LPTSTR)BUT_CLEAR_ICO, NULL);
			hWnd_Users = CreateListBox(hwnd, LEFT, TOP*2 + BUT_HEIGHT, LIST_WIDTH, LIST_HEIGHT, STR_EMPTY, LIST_USERS);
			hWnd_Messages = CreateListBox(hwnd, LEFT+LIST_WIDTH+PADDING, TOP*2 + BUT_HEIGHT, LIST_WIDTH, LIST_HEIGHT, STR_EMPTY, LIST_MSGS);

			CreateLabel(hwnd, LEFT+BUT_WIDTH*4+PADDING, TOP, LABEL_WIDTH, LABEL_HEIGHT, STR_NIKE);
			hWnd_Nike = CreateText(hwnd, LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING, TOP, TEXT_WIDTH, TEXT_HEIGHT, STR_EMPTY, TXT_NIKE);
			CreateBut(hwnd, LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH, TOP, BUT_WIDTH/2,BUT_HEIGHT/2, (PCHAR)STR_DOTS, BUT_NIKE);

			CreateLabel(hwnd, LEFT+BUT_WIDTH*4+PADDING, TOP+LABEL_HEIGHT, LABEL_WIDTH, LABEL_HEIGHT, STR_AVATARE);
			hWnd_Avatare = CreateText(hwnd, LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING, TOP+TEXT_HEIGHT, TEXT_WIDTH, TEXT_HEIGHT, STR_EMPTY, TXT_AVATARE);
			CreateBut(hwnd, LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH, TOP+BUT_HEIGHT/2, BUT_WIDTH/2,BUT_HEIGHT/2, (PCHAR)STR_DOTS, BUT_AVATARE);

			hWnd_Msg = CreateMultiRowText(hwnd, LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH+BUT_WIDTH/2+PADDING, TOP, TEXT_WIDTH, TEXT_HEIGHT*2, STR_EMPTY, TXT_MSG);

			hWnd_ButMsg = CreateButIcon(hwnd, 
				LEFT+BUT_WIDTH*4+PADDING+LABEL_WIDTH+PADDING+TEXT_WIDTH+BUT_WIDTH/2+PADDING+TEXT_WIDTH, 
				TOP, 
				BUT_WIDTH, 
				BUT_HEIGHT, 
				(PCHAR)STR_MSG,
				BUT_MSG, BUT_MSG_ICO, NULL );
			
			break;
		case WM_SIZE :// при изменении размера окна
			ResizeUI(hwnd);
			break;
		case WM_DESTROY :
            PostQuitMessage (0) ;
            break;
		case WM_COMMAND :// обработчики команд
			switch( LOWORD(wParam)) // если нажата какая-то кнопка
            {
			case BUT_MSG: // кнопка отправить сообщение
				SendTextMessage();
				break;
			case BUT_NIKE:
				SetNike();
				break;
			case BUT_AVATARE:
				SelectAvatareFile();
				break;
			case BUT_CLEAR: // очистить список сообщений
				SendMessage(hWnd_Messages, LB_RESETCONTENT, 0, 0);
				break;
			case BUT_EXIT:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			default:
				break;
			}
			break;
		case WM_DRAWITEM: // прорисовка строки списка
			Item = (LPDRAWITEMSTRUCT)lParam;
            if (Item->CtlID == LIST_MSGS) // есои список сообщений
            {
                DrawUserItem(Item);
            }
			break;
		case WM_MEASUREITEM: 
        {
            MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT*) lParam;
            if (mis->CtlID == LIST_USERS) // для списка пользователей более широкая строка
            {
                mis->itemHeight = LIST_USERS_ITEM_HEIGHT;
                return TRUE;
            }
            break;
        }
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
	wc.hIcon = NULL;
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
		LBS_NOTIFY | WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS |  WS_VISIBLE| WS_VSCROLL,
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

// кнопка с картинкой
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

// кнопка
HWND CreateBut(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name, INT cmd)
{
    HWND ww;

    ww = CreateWindowEx(WS_EX_CONTROLPARENT, (LPCSTR)"BUTTON", (LPCSTR)name,
    WS_VISIBLE | WS_CHILD | WS_TABSTOP, x, y, w, h, hwnd2, (HMENU) cmd,

    (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE),
            NULL);

	SendMessage(ww, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

    return ww;
}

// метка
HWND CreateLabel(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name )
{
    HWND ww;

    ww = CreateWindow( (LPCSTR)"STATIC", (LPCSTR)name, WS_VISIBLE | WS_CHILD | SS_RIGHT, x, y, w,  h, hwnd2, (HMENU) 99,
    (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE), NULL);

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

// текстовое поле
HWND CreateText(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name, INT cmd )
{
    HWND ww;

    ww = CreateWindow( (LPCSTR)"EDIT", (LPCSTR)name, WS_VISIBLE | WS_CHILD | SS_SUNKEN | SS_LEFT | WS_BORDER,
		x, y, w,  h, hwnd2, (HMENU) cmd, (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE), NULL);

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

// многострочное текстовое поле
HWND CreateMultiRowText(HWND hwnd2, INT x, INT y, INT w, INT h, PCHAR name, INT cmd )
{
    HWND ww;

    ww = CreateWindow( (LPCSTR)"EDIT", (LPCSTR)name, WS_VISIBLE | WS_CHILD | SS_SUNKEN | SS_LEFT | 
				WS_BORDER | ES_MULTILINE, x, y, w,  h, hwnd2, (HMENU) cmd, (HINSTANCE) GetWindowLong(hwnd2, GWL_HINSTANCE), NULL);

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
//---------------------------------------------------------------------------------------

void setIcon(char *path){
	HANDLE hIcon = LoadImage(0, path, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hIcon) {
		//Change both icons to the same icon handle.
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		//This will ensure that the application icon gets changed too.
		//SendMessage(GetWindow(hwnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		//SendMessage(GetWindow(hwnd, GW_OWNER), WM_SETICON, ICON_BIG, hIcon);
	}
}

void SelectAvatareFile(){

	char *filename;//[MAX_PATH];             // буфер под имя файла
	char *cur_dir;

	filename = (char *)LocalAlloc(LPTR, MAX_PATH - 1);
	if(filename == NULL){
		return;
	}

	cur_dir = (char *)LocalAlloc(LPTR, MAX_PATH - 1);
	if(cur_dir == NULL){

		LocalFree(filename);
		return;
	}
	ZeroMemory(filename, MAX_PATH - 1);
	ZeroMemory(cur_dir, MAX_PATH - 1);
	
	OPENFILENAME of;
	ZeroMemory(&of,sizeof(OPENFILENAME));
	of.lStructSize			= sizeof(OPENFILENAME);
	of.hwndOwner			= NULL;
	of.lpstrFilter			= (LPCSTR)"Icon files(*.ico)\0*.ico";// фильтр файлов (тип)
	of.lpstrCustomFilter	= NULL;           
	of.nMaxCustFilter		= 0;              
	of.nFilterIndex			= 1;              //количество заданных фильтров
	of.lpstrFile			= (LPSTR)filename;       //адрес буфера под имя файла
	of.lpstrFile[0]			= '\0';
	of.nMaxFile				= MAX_PATH - 1;   //размер буфера под имя файла
	of.lpstrFileTitle		= NULL;           //буфер под рекомендуемый заголовок: нам не надо
	of.nMaxFileTitle		= 0;              //нам не надо
	of.lpstrInitialDir		= NULL;			//стартовый каталог: текущий
	of.Flags				= OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;//разные флаги

	if (GetOpenFileName(&of)) {
		SendMessage( hWnd_Avatare, WM_SETTEXT, 0, (LPARAM)( filename ) );
		setIcon(filename);
	}

	LocalFree(filename);
	LocalFree(cur_dir);
}



