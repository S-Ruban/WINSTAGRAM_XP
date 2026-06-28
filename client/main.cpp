#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

typedef unsigned long ULONG_PTR;

#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <windowsx.h>
#include <objbase.h>
#include <gdiplus.h>
#pragma message("Using Platform SDK headers")
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

ULONG_PTR g_gdiplusToken;

int g_totalContentHeight = 0;
std::string IMAGE_DIRECTORY = "images\\";

struct Post
{
    std::string username;
    std::string caption;
    std::string imagePath; // optional

    HBITMAP hBitmap;

    int imageWidth;
    int imageHeight;
    int scaledHeight;

    int postHeight;    // cache post's height
    int captionHeight; // cache caption height
};

void ReplaceNewlines(std::string &text)
{
    size_t pos = 0;

    while ((pos = text.find("\\n", pos)) != std::string::npos)
    {
        text.replace(pos, 2, "\n");
        pos++;
    }
}

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

HBITMAP LoadImageFile(const std::string &filename)
{
    WCHAR wfilename[MAX_PATH];

    MultiByteToWideChar(
        CP_ACP,
        0,
        filename.c_str(),
        -1,
        wfilename,
        MAX_PATH); // GDI+ handles strings the old way

    Bitmap bitmap(wfilename);

    Status status = bitmap.GetLastStatus();

    if (status != Ok)
    {
        char buf[256];

        wsprintf(
            buf,
            "Failed!\nFile=[%s]\nStatus=%d",
            filename.c_str(),
            status);

        MessageBox(NULL, buf, "PNG Debug", MB_OK);

        return NULL;
    }

    HBITMAP hBitmap = NULL;

    bitmap.GetHBITMAP(
        Color(0, 0, 0),
        &hBitmap);

    BITMAP bmp;

    GetObject(
        hBitmap,
        sizeof(BITMAP),
        &bmp);

    char buf[256];

    wsprintf(
        buf,
        "BPP=%d",
        bmp.bmBitsPixel);

    return hBitmap;
}

int CalculatePostHeight(const Post &p)
{
    return 30 + // username
           p.scaledHeight + 10 +
           p.captionHeight + 10 +
           15;
}

void LoadPostsFromFile()
{

    for (size_t i = 0; i < g_posts.size(); ++i)
    {
        if (g_posts[i].hBitmap)
            DeleteObject(g_posts[i].hBitmap);
    }
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
            ReplaceNewlines(p.caption);
            p.imagePath = IMAGE_DIRECTORY + imagePath;
            p.hBitmap = LoadImageFile(p.imagePath);

            p.imageWidth = 0;
            p.imageHeight = 0;
            p.scaledHeight = 100;

            if (p.hBitmap)
            {
                BITMAP bmp;

                GetObject(
                    p.hBitmap,
                    sizeof(BITMAP),
                    &bmp);

                p.imageWidth = bmp.bmWidth;
                p.imageHeight = bmp.bmHeight;

                int maxWidth = 250;

                float scale =
                    (float)maxWidth /
                    (float)bmp.bmWidth;

                p.scaledHeight =
                    (int)(bmp.bmHeight * scale);

                HDC hdc = GetDC(NULL); // give a generic display screen, since the caption size is calculated
                                       // before the post is actually rendered for the first time

                RECT rc = {0, 0, 340, 0};

                DrawText(
                    hdc, p.caption.c_str(), -1, &rc,       // CALCRECT is to estimate the rectangle size to hold the caption
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT); // does not actually draw the rectangle

                ReleaseDC(NULL, hdc); // "I don't want to play with you anymore"

                p.captionHeight = rc.bottom - rc.top;

                p.postHeight = CalculatePostHeight(p);
            }

            g_posts.push_back(p);
        }
    }
}

int CalculateFeedHeight()
{
    int total = 10;

    for (size_t i = 0; i < g_posts.size(); ++i)
        total += CalculatePostHeight(g_posts[i]);

    return total;
}

void UpdateScrollbar(HWND hwnd)
{
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));

    RECT rc;
    GetClientRect(hwnd, &rc);

    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    si.nMin = 0;
    si.nMax = CalculateFeedHeight();
    si.nPage = rc.bottom - rc.top;
    si.nPos = g_scrollPos;

    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void RefreshFeed(HWND hwnd)
{
    LoadPostsFromFile();

    g_scrollPos = 0;

    UpdateScrollbar(hwnd);

    SCROLLINFO si = {sizeof(si), SIF_POS};
    si.nPos = g_scrollPos;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    InvalidateRect(hwnd, NULL, TRUE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

    GdiplusStartupInput gdiplusStartupInput;

    GdiplusStartup(
        &g_gdiplusToken,
        &gdiplusStartupInput,
        NULL);

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
                             420, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(g_gdiplusToken);

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
        if (LOWORD(wParam) == 1) // Refresh button is clicked
        {
            RefreshFeed(g_hScrollWnd);
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
    {
        LoadPostsFromFile();
        g_scrollPos = 0;
        UpdateScrollbar(hwnd);
        return 0;
    }

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
        int renderedPosts = 0;

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);
        int visibleTop = 0;
        int visibleBottom = client.bottom;

        int currentY = -g_scrollPos + 10;

        for (int i = 0; i < g_posts.size(); ++i)
        {
            int postTop = currentY;
            int postHeight = g_posts[i].postHeight;
            int postBottom = postTop + postHeight;

            // really simple viewport culling - skip rendering anything above the viewport, and stop rendering by the end of the viewport
            if (postBottom < visibleTop)
            {
                currentY += postHeight;
                continue; // skip rendering current post if it lies above the scrollbar
            }
            if (postTop > visibleBottom)
            {
                break; // stop rendering further posts if they lie below the scrollbar
            }

            renderedPosts++;
            // Rendering username
            RECT userRect = {20, currentY + 5, 360, currentY + 25};
            DrawText(
                hdc, g_posts[i].username.c_str(), -1, &userRect,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER);

            currentY += 30;

            // Rendering image
            int scaledWidth = 250;
            int scaledHeight = g_posts[i].scaledHeight;

            RECT imgRect;

            HBITMAP hBitmap = g_posts[i].hBitmap;

            if (hBitmap)
            {
                BITMAP bmp;
                GetObject(hBitmap, sizeof(BITMAP), &bmp);

                imgRect.left = 20;
                imgRect.top = currentY;
                imgRect.right = 20 + scaledWidth;         // calculate right of image border using scaled width of image
                imgRect.bottom = currentY + scaledHeight; // calculate bottom of image border using scaled height of image

                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

                RECT frameRect = {imgRect.left - 4, imgRect.top - 4, imgRect.right + 4, imgRect.bottom + 4};

                DrawEdge(hdc, &frameRect, EDGE_SUNKEN, BF_RECT);
                SetStretchBltMode(hdc, COLORONCOLOR); // needed to stretch png/jpg
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
            }
            else
            {
                imgRect.left = 20;
                imgRect.top = currentY;
                imgRect.right = 20 + scaledWidth;
                imgRect.bottom = currentY + scaledHeight;

                FillRect(hdc, &imgRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
                DrawEdge(hdc, &imgRect, EDGE_SUNKEN, BF_RECT);
            }
            currentY += scaledHeight + 10;

            // Rendering caption
            RECT capRect = {20, currentY, 360, currentY + g_posts[i].captionHeight};

            DrawText(hdc, g_posts[i].caption.c_str(), -1, &capRect, DT_LEFT | DT_WORDBREAK);

            currentY += g_posts[i].captionHeight;
            currentY += 10;

            RECT postRect = {10, postTop, 380, currentY};

            DrawEdge(hdc, &postRect, EDGE_ETCHED, BF_RECT);

            currentY += 15;
        }

        char buf[64];
        wsprintf(buf, "Rendered %d posts", renderedPosts);
        // SetWindowText(GetParent(hwnd), buf);

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

    case WM_SIZE:
    {
        UpdateScrollbar(hwnd);
        return 0;
    }

    case WM_USER + 1: // kinda useless but keeping it for now
    {
        RefreshFeed(hwnd);
        return 0;
    }

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
            newPos = CalculateFeedHeight();
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
