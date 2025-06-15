#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

struct Post
{
    std::string username;
    std::string caption;
    std::string imagePath; // optional
};

std::vector<Post> g_posts;

void debug(char *str)
{
    MessageBox(NULL, str, "Debug", MB_OK);
}

#define POST_HEIGHT 120
#define NUM_POSTS 20

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

LRESULT CALLBACK ScrollWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInstance;
HWND g_hScrollWnd;
int g_scrollPos = 0;

void LoadPostsFromFile()
{
    g_posts.clear();

    std::ifstream file("posts.txt");
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string username, caption, imagePath;

        std::getline(iss, username, ':');
        iss.ignore(1); // skip second colon
        std::getline(iss, caption, ':');
        iss.ignore(1); // skip second colon
        std::getline(iss, imagePath);

        if (!username.empty() && !caption.empty())
        {
            Post p;
            p.username = username;
            p.caption = caption;
            p.imagePath = "placeholder.jpg";
            g_posts.push_back(p);
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    g_hInstance = hInstance;

    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainWin";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    wc.lpfnWndProc = ScrollWndProc;
    wc.lpszClassName = "ScrollWin";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("MainWin", "Winstagram XP",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             400, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// Main window just holds the scrollable child
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create the scrollable child window
        g_hScrollWnd = CreateWindow("ScrollWin", NULL,
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                                    0, 40, 400, 560, // leave space at top for title and button
                                    hwnd, NULL, g_hInstance, NULL);

        // Title label
        CreateWindow("STATIC", "Winstagram XP",
                     WS_CHILD | WS_VISIBLE | SS_CENTER,
                     10, 10, 200, 20,
                     hwnd, NULL, g_hInstance, NULL);

        // Refresh button
        CreateWindow("BUTTON", "Refresh",
                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                     300, 10, 80, 25,
                     hwnd, (HMENU)1, g_hInstance, NULL);

        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) // Refresh button
        {
            g_scrollPos = 0;

            SCROLLINFO si = {sizeof(si), SIF_POS};
            si.nPos = g_scrollPos;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

            // Force repaint from top
            ScrollWindow(hwnd, 0, 0, NULL, NULL);
            UpdateWindow(hwnd);

            LoadPostsFromFile();                          // reload from file
            InvalidateRect(g_hScrollWnd, NULL, TRUE);     // repaint scroll window
            SendMessage(g_hScrollWnd, WM_USER + 1, 0, 0); // Custom message to refresh
        }
        return 0;

    case WM_SIZE:
        MoveWindow(g_hScrollWnd, 0, 40, LOWORD(lParam), HIWORD(lParam) - 40, TRUE);
        return 0;

    case WM_MOUSEWHEEL:
        // Forward to child scroll window
        SendMessage(g_hScrollWnd, msg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Handles drawing and scroll behavior
LRESULT CALLBACK ScrollWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SCROLLINFO si = {sizeof(SCROLLINFO), SIF_ALL, 0, 0, 0, 0, 0};
    int oldPos;

    switch (msg)
    {
    case WM_CREATE:
        si.nMin = 0;
        // si.nMax = NUM_POSTS * POST_HEIGHT;
        LoadPostsFromFile();
        si.nMax = g_posts.size() * POST_HEIGHT;
        si.nPage = 600;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        return 0;

    case WM_VSCROLL:
    {
        SCROLLINFO si = {sizeof(SCROLLINFO)};
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);
        int oldPos = si.nPos;

        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
            si.nPos -= 20;
            break;
        case SB_LINEDOWN:
            si.nPos += 20;
            break;
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = HIWORD(wParam);
            break;
        }

        // Clamp
        si.nPos = max(si.nMin, min(si.nPos, si.nMax - (int)si.nPage));
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        if (si.nPos != oldPos)
        {
            int delta = oldPos - si.nPos;
            g_scrollPos = si.nPos;
            ScrollWindow(hwnd, 0, delta, NULL, NULL);
            UpdateWindow(hwnd); // Force repaint
        }

        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);
        int y = -g_scrollPos;

        for (int i = 0; i < g_posts.size(); ++i)
        {
            int top = y + i * POST_HEIGHT + 10;
            int bottom = top + POST_HEIGHT - 10;

            RECT postRect = {10, top, 380, bottom};
            HBRUSH lightGrayBrush = CreateSolidBrush(RGB(230, 230, 230));
            FillRect(hdc, &postRect, lightGrayBrush);
            FrameRect(hdc, &postRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
            DeleteObject(lightGrayBrush);

            // Username
            RECT userRect = {20, top + 5, 360, top + 25};
            DrawText(hdc, g_posts[i].username.c_str(), -1, &userRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

            // Image placeholder
            RECT imgRect = {20, top + 30, 200, top + 80};
            FillRect(hdc, &imgRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
            FrameRect(hdc, &imgRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

            // Caption
            RECT capRect = {20, top + 85, 360, top + 105};
            DrawText(hdc, g_posts[i].caption.c_str(), -1, &capRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int scrollLines = -delta / WHEEL_DELTA * 20; // scroll 20px per notch

        SCROLLINFO si = {sizeof(si)};
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);

        int newPos = g_scrollPos + scrollLines;
        newPos = max(si.nMin, min(newPos, si.nMax - (int)si.nPage));

        if (newPos != g_scrollPos)
        {
            ScrollWindow(hwnd, 0, g_scrollPos - newPos, NULL, NULL);
            g_scrollPos = newPos;

            si.fMask = SIF_POS;
            si.nPos = g_scrollPos;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            UpdateWindow(hwnd);
        }
        return 0;
    }
    case WM_USER + 1:
        LoadPostsFromFile(); // or however you're updating
        g_scrollPos = 0;

        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = (int)(g_posts.size() * POST_HEIGHT);
        si.nPage = 600;
        si.nPos = 0;
        si.nTrackPos = 0;

        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        g_scrollPos = 0;
        InvalidateRect(hwnd, NULL, TRUE);

        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
