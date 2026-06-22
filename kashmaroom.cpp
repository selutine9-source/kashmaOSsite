#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>

#define W 1200
#define H 800
#define cmax(a,b) ((a)>(b)?(a):(b))

struct ChatMsg { wchar_t who[64]; wchar_t txt[256]; DWORD t; };

float g_px = 600, g_py = 400;
int g_dir = 0, g_frame = 0, g_ftimer = 0;
std::vector<ChatMsg> g_chat;
int g_cx = 0, g_cy = 0;
bool g_typing = false;
wchar_t g_in[256] = L"";

HFONT hf14, hf18, hf16;

void AddChat(const wchar_t* w, const wchar_t* t) {
    ChatMsg m; wcscpy_s(m.who, w); wcscpy_s(m.txt, t); m.t = GetTickCount();
    g_chat.push_back(m); if (g_chat.size() > 50) g_chat.erase(g_chat.begin());
}

void Grass(HDC hdc, int cx, int cy) {
    for (int y = -60; y < H + 60; y += 60)
        for (int x = -60; x < W + 60; x += 60) {
            int sx = x - (cx % 60), sy = y - (cy % 60);
            int s = ((x + cx) / 60 * 7 + (y + cy) / 60 * 13) % 3;
            COLORREF c = s == 0 ? RGB(60,140,60) : s == 1 ? RGB(50,130,55) : RGB(70,150,65);
            HBRUSH br = CreateSolidBrush(c);
            RECT r = { sx, sy, sx + 60, sy + 60 }; FillRect(hdc, &r, br); DeleteObject(br);
        }
    for (int i = 0; i < 30; i++) {
        int fx = ((i * 137 + 50) % 2000) - cx;
        int fy = ((i * 97 + 30) % 1500) - cy;
        if (fx > -20 && fx < W + 20 && fy > -20 && fy < H + 20) {
            HBRUSH br = CreateSolidBrush(i % 3 == 0 ? RGB(255,200,50) : i % 3 == 1 ? RGB(255,100,150) : RGB(200,100,255));
            RECT r = { fx - 3, fy - 3, fx + 3, fy + 3 }; FillRect(hdc, &r, br); DeleteObject(br);
        }
    }
}

void Tree(HDC hdc, int x, int y, int cx, int cy) {
    int sx = x - cx, sy = y - cy;
    HBRUSH t = CreateSolidBrush(RGB(100,70,40));
    RECT tr = { sx-8, sy, sx+8, sy+40 }; FillRect(hdc, &tr, t); DeleteObject(t);
    HBRUSH l = CreateSolidBrush(RGB(30,120,30));
    RECT lr = { sx-30, sy-30, sx+30, sy+10 }; FillRect(hdc, &lr, l); DeleteObject(l);
}

void Char(HDC hdc, int x, int y, int dir, int fr, COLORREF col, const wchar_t* name) {
    HBRUSH b = CreateSolidBrush(col);
    RECT br = { x-12, y-20, x+12, y+10 }; FillRect(hdc, &br, b); DeleteObject(b);
    HBRUSH h = CreateSolidBrush(RGB(255,220,180));
    RECT hr = { x-10, y-36, x+10, y-16 }; FillRect(hdc, &hr, h); DeleteObject(h);
    HBRUSH e = CreateSolidBrush(RGB(30,30,30));
    int eo = dir==2 ? -4 : dir==3 ? 4 : 0;
    RECT e1 = { x-6+eo, y-30, x-2+eo, y-26 }, e2 = { x+2+eo, y-30, x+6+eo, y-26 };
    FillRect(hdc, &e1, e); FillRect(hdc, &e2, e); DeleteObject(e);
    int lo = fr%2 ? 3 : 0;
    HBRUSH lg = CreateSolidBrush(RGB(50,50,100));
    RECT l1 = { x-10, y+10, x-3, y+18+lo }, l2 = { x+3, y+10, x+10, y+18-lo };
    FillRect(hdc, &l1, lg); FillRect(hdc, &l2, lg); DeleteObject(lg);
    SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, RGB(255,255,255));
    SelectObject(hdc, hf14);
    RECT nr = { x-40, y-50, x+40, y-38 };
    DrawText(hdc, name, -1, &nr, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        hf14 = CreateFont(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        hf16 = CreateFont(16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Consolas");
        hf18 = CreateFont(18,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        AddChat(L"\u0421\u0438\u0441\u0442\u0435\u043c\u0430", L"\u0414\u043e\u0431\u0440\u043e \u043f\u043e\u0436\u0430\u043b\u043e\u0432\u0430\u0442\u044c \u0432 \u041a\u0430\u0448\u043c\u0430\u0440\u0443\u043c!");
        SetTimer(hwnd, 1, 16, NULL);
        break;
    case WM_TIMER:
        g_cx = (int)g_px - W/2; g_cy = (int)g_py - H/2;
        if (g_cx<0) g_cx=0; if (g_cy<0) g_cy=0;
        if (g_cx>2000-W) g_cx=2000-W; if (g_cy>1500-H) g_cy=1500-H;
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd,&ps);
        HDC m = CreateCompatibleDC(hdc); HBITMAP mb = CreateCompatibleBitmap(hdc,W,H);
        SelectObject(m, mb);
        HBRUSH bg = CreateSolidBrush(RGB(40,80,40));
        RECT r = {0,0,W,H}; FillRect(m,&r,bg); DeleteObject(bg);
        Grass(m, g_cx, g_cy);
        Tree(m, 500,300,g_cx,g_cy); Tree(m, 700,150,g_cx,g_cy);
        Tree(m, 150,400,g_cx,g_cy); Tree(m, 1000,350,g_cx,g_cy);
        Tree(m, 400,600,g_cx,g_cy);
        Char(m, (int)g_px-g_cx, (int)g_py-g_cy, g_dir, g_frame, RGB(100,50,180), L"\u041a\u0430\u0448\u043c\u0430");
        SelectObject(m, hf18); SetBkMode(m, TRANSPARENT);
        wchar_t c[32]; swprintf(c,32,L"SM: 100");
        SetTextColor(m,RGB(255,220,50));
        RECT cr = {W-160,10,W-10,35}; DrawText(m,c,-1,&cr,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        SetTextColor(m,RGB(200,200,255));
        RECT nr = {10,10,300,40}; DrawText(m,L"\u041a\u0430\u0448\u043c\u0430",-1,&nr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        int cx2=10, cy2=H-180, cw=400, ch=170;
        HBRUSH cb = CreateSolidBrush(RGB(10,10,25));
        RECT cbr = {cx2,cy2,cx2+cw,cy2+ch}; FillRect(m,&cbr,cb); DeleteObject(cb);
        HPEN cp = CreatePen(PS_SOLID,1,RGB(80,50,150));
        SelectObject(m,cp); SelectObject(m,GetStockObject(NULL_BRUSH));
        Rectangle(m,cx2,cy2,cx2+cw,cy2+ch); DeleteObject(cp);
        SelectObject(m, hf14); SetBkMode(m,TRANSPARENT);
        int si = (int)g_chat.size()-8; if (si<0) si=0;
        int yo = cy2+8;
        for (int i=si; i<(int)g_chat.size(); i++) {
            DWORD age = GetTickCount()-g_chat[i].t;
            int a = age<15000 ? 255 : cmax(0, (int)(255*(1.0f-(age-15000)/5000.0f)));
            if (a<=0) continue;
            wchar_t buf[320]; swprintf(buf,320,L"%s: %s", g_chat[i].who, g_chat[i].txt);
            SetTextColor(m,RGB(a,a,a));
            RECT tr={cx2+8,yo,cx2+cw-8,yo+16};
            DrawText(m,buf,-1,&tr,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            yo+=18;
        }
        if (g_typing) {
            HBRUSH ib = CreateSolidBrush(RGB(20,20,40));
            RECT ir={cx2,cy2+ch+2,cx2+cw,cy2+ch+24}; FillRect(m,&ir,ib); DeleteObject(ib);
            wchar_t ibuf[270]; swprintf(ibuf,270,L"> %s_", g_in);
            SetTextColor(m,RGB(200,200,255));
            RECT itr={cx2+8,cy2+ch+2,cx2+cw-8,cy2+ch+24};
            DrawText(m,ibuf,-1,&itr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        }
        BitBlt(hdc,0,0,W,H,m,0,0,SRCCOPY);
        DeleteObject(mb); DeleteDC(m);
        EndPaint(hwnd,&ps);
        break;
    }
    case WM_KEYDOWN: {
        if (g_typing) {
            if (wp==VK_RETURN) { if (wcslen(g_in)>0) { AddChat(L"\u041a\u0430\u0448\u043c\u0430", g_in); g_in[0]=0; } g_typing=false; }
            else if (wp==VK_ESCAPE) { g_typing=false; g_in[0]=0; }
            else if (wp==VK_BACK) { int l=(int)wcslen(g_in); if(l>0) g_in[l-1]=0; }
            InvalidateRect(hwnd,NULL,FALSE); break;
        }
        float sp=4; bool mv=false;
        if (wp=='W'||wp==VK_UP) { g_py-=sp; g_dir=1; mv=true; }
        if (wp=='S'||wp==VK_DOWN) { g_py+=sp; g_dir=0; mv=true; }
        if (wp=='A'||wp==VK_LEFT) { g_px-=sp; g_dir=2; mv=true; }
        if (wp=='D'||wp==VK_RIGHT) { g_px+=sp; g_dir=3; mv=true; }
        if (g_px<20)g_px=20; if(g_py<20)g_py=20; if(g_px>1980)g_px=1980; if(g_py>1480)g_py=1480;
        if (mv) { g_ftimer++; if(g_ftimer>=4){g_frame++;g_ftimer=0;} }
        if (wp=='T'||wp=='t') { g_typing=true; g_in[0]=0; }
        InvalidateRect(hwnd,NULL,FALSE);
        break;
    }
    case WM_CHAR: {
        if (g_typing && wp!=VK_RETURN && wp!=VK_ESCAPE && wp!=VK_BACK) {
            int len=(int)wcslen(g_in);
            if (len<200) { g_in[len]=(wchar_t)wp; g_in[len+1]=0; }
            InvalidateRect(hwnd,NULL,FALSE);
        }
        break;
    }
    case WM_DESTROY:
        KillTimer(hwnd,1); PostQuitMessage(0); break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}

int WINAPI wWinMain(HINSTANCE hI, HINSTANCE, LPWSTR, int n) {
    WNDCLASS wc={}; wc.lpfnWndProc=WndProc; wc.hInstance=hI;
    wc.lpszClassName=L"Kashmaroom"; wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);
    HWND hw = CreateWindowEx(0,L"Kashmaroom",L"Kashmaroom — KF Production",
        WS_OVERLAPPEDWINDOW&~WS_THICKFRAME&~WS_MAXIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,W,H,NULL,NULL,hI,NULL);
    ShowWindow(hw,n); UpdateWindow(hw);
    MSG m; while(GetMessage(&m,NULL,0,0)){TranslateMessage(&m);DispatchMessage(&m);}
    return 0;
}
