#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

int g_totalContentHeight = 0;

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
            p.imagePath = imagePath;
            g_posts.push_back(p);
        }
    }
}

int CalculateFeedHeight()
{
    int total = 10;

    for (size_t i = 0; i < g_posts.size(); ++i)
    {
        HBITMAP hBitmap =
            (HBITMAP)LoadImage(
                NULL,
                g_posts[i].imagePath.c_str(),
                IMAGE_BITMAP,
                0,
                0,
                LR_LOADFROMFILE);

        int scaledHeight = 200; // fallback

        if (hBitmap)
        {
            BITMAP bmp;
            GetObject(hBitmap, sizeof(BITMAP), &bmp);
            int maxWidth = 250;
            float scale = (float)maxWidth / bmp.bmWidth;
            scaledHeight = (int)(bmp.bmHeight * scale); // Scale the height to match the width
            DeleteObject(hBitmap);
        }

        total += 25;                // username
        total += scaledHeight + 10; // image
        total += 30;                // caption
        total += 20;                // padding
    }

    return total;
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
                                    // WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
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
    {
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
    }

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

    case WM_KEYDOWN: // This case handles keystroke events
    {
        SCROLLINFO si;
        switch (wParam)
        {
        case VK_HOME:
            g_scrollPos = 0;
            break;

        case VK_END:
        {
            memset(&si, 0, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            GetScrollInfo(g_hScrollWnd, SB_VERT, &si);
            g_scrollPos = si.nMax - si.nPage;
            if (g_scrollPos < 0)
                g_scrollPos = 0;

            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        si.fMask = SIF_POS;
        si.nPos = g_scrollPos;
        SetScrollInfo(g_hScrollWnd, SB_VERT, &si, TRUE);
        InvalidateRect(g_hScrollWnd, NULL, TRUE);
        return 0;
    }
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
        LoadPostsFromFile();
        si.nMax = CalculateFeedHeight();
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

        int currentY = -g_scrollPos + 10;

        for (int i = 0; i < g_posts.size(); ++i)
        {
            int postTop = currentY;

            // Rendering username
            RECT userRect = {20, currentY + 5, 360, currentY + 25};
            DrawText(
                hdc,
                g_posts[i].username.c_str(),
                -1,
                &userRect,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER);

            currentY += 30;

            // Rendering image
            int scaledWidth = 250;
            int scaledHeight = 100;

            RECT imgRect;

            HBITMAP hBitmap =
                (HBITMAP)LoadImage(
                    NULL,
                    g_posts[i].imagePath.c_str(),
                    IMAGE_BITMAP,
                    0,
                    0,
                    LR_LOADFROMFILE);

            if (hBitmap)
            {
                BITMAP bmp;
                GetObject(hBitmap, sizeof(BITMAP), &bmp);

                float scale = (float)scaledWidth / (float)bmp.bmWidth;
                scaledHeight = (int)(bmp.bmHeight * scale);

                imgRect.left = 20;
                imgRect.top = currentY;
                imgRect.right = 20 + scaledWidth;         // calculate right of image border using scaled width of image
                imgRect.bottom = currentY + scaledHeight; // calculate bottom of image border using scaled height of image

                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP oldBmp =
                    (HBITMAP)SelectObject(hdcMem, hBitmap);

                StretchBlt(
                    hdc,
                    imgRect.left,
                    imgRect.top,
                    scaledWidth,
                    scaledHeight,
                    hdcMem,
                    0,
                    0,
                    bmp.bmWidth,
                    bmp.bmHeight,
                    SRCCOPY);

                SelectObject(hdcMem, oldBmp);
                DeleteDC(hdcMem);
                DeleteObject(hBitmap);
            }
            else
            {
                imgRect.left = 20;
                imgRect.top = currentY;
                imgRect.right = 20 + scaledWidth;
                imgRect.bottom = currentY + scaledHeight;

                FillRect(
                    hdc,
                    &imgRect,
                    (HBRUSH)GetStockObject(LTGRAY_BRUSH));

                FrameRect(
                    hdc,
                    &imgRect,
                    (HBRUSH)GetStockObject(GRAY_BRUSH));
            }
            currentY += scaledHeight + 10;

            // Rendering caption
            RECT capRect = {20, currentY, 360, currentY + 20};

            DrawText(
                hdc,
                g_posts[i].caption.c_str(),
                -1,
                &capRect,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER);

            currentY += 30;

            // Rendering border
            RECT postRect = {10, postTop, 380, currentY};
            HBRUSH lightGrayBrush = CreateSolidBrush(RGB(230, 230, 230));

            FrameRect(
                hdc,
                &postRect,
                (HBRUSH)GetStockObject(GRAY_BRUSH));

            DeleteObject(lightGrayBrush);
            currentY += 15;
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

    case WM_MBUTTONDOWN:
        MessageBox(hwnd, "Middle button!", "Test", MB_OK); // Let this be here for now
        return 0;

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

    case WM_KEYDOWN:
    {
        SCROLLINFO si;
        memset(&si, 0, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;

        GetScrollInfo(hwnd, SB_VERT, &si);

        int newPos = g_scrollPos;

        switch (wParam)
        {
        case VK_UP:
            newPos -= 20 * 5; // faster scrolling with arrow keys
            break;

        case VK_DOWN:
            newPos += 20 * 5;
            break;

        case VK_HOME:
            newPos = 0;
            break;

        case VK_END:
            newPos = si.nMax - (int)si.nPage;
            break;

        default:
            return 0;
        }

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

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
