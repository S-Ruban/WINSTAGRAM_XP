#include <windows.h>
#include <vector>
#include <algorithm> // for std::random_shuffle
#include <ctime>     // for srand
#include <iostream>
#include <stdio.h>

#define ID_REFRESH 1001
int g_scrollOffset = 0;
const int SCROLL_STEP = 20;
const int TOP_UI_HEIGHT = 40;

// Define the Post structure
struct Post
{
    char username[1024];
    char caption[1024];
    char imagePath[1024]; // Optional for now
};

void debug(char *str)
{
    MessageBox(NULL, str, "Debug", MB_OK);
}

// Global post list
std::vector<Post> g_posts;

// Track created controls to remove later
std::vector<HWND> g_postControls;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateScrollRange(HWND hwnd);

HWND hwndLogoLabel = NULL;
HWND hwndRefreshButton = NULL;

HINSTANCE g_hInstance;

int yOffset = 50; // Start below the top bar
int postSpacing = 150;

void UpdateScrollRange(HWND hwnd)
{
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE;

    int totalHeight = g_posts.size() * postSpacing; // Total scrollable area
    int visibleHeight = 600 - TOP_UI_HEIGHT;

    si.nMin = 0;
    si.nMax = totalHeight > visibleHeight ? totalHeight - visibleHeight : 0;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void InitPosts()
{
    g_posts.clear(); // In case it’s called more than once

    Post p1 = {"user_one", "This is the first fake post!", ""};
    Post p2 = {"second_user", "Another caption for post two!", ""};
    Post p3 = {"vintage_lover", "XP hits different!", ""};
    Post p4 = {"retrotech", "Throwback Thursday #WinXP", ""};

    g_posts.push_back(p1);
    g_posts.push_back(p2);
    g_posts.push_back(p3);
    g_posts.push_back(p4);
}

void PrintCurrentDirectory()
{
    char buffer[MAX_PATH];
    DWORD len = GetCurrentDirectory(MAX_PATH, buffer);
    if (len > 0)
    {
        MessageBox(NULL, buffer, "Current Directory", MB_OK);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const char CLASS_NAME[] = "WinstagramXP";
    g_hInstance = hInstance;
    srand((unsigned int)time(0));

    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Winstagram XP",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    memset(&msg, 0, sizeof(msg));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void RenderPosts(HWND hwnd)
{
    int yOffset = TOP_UI_HEIGHT - g_scrollOffset;
    int postSpacing = 150;

    for (size_t i = 0; i < g_posts.size(); ++i)
    {
        const Post &post = g_posts[i];

        HWND hUser = CreateWindow("STATIC", post.username,
                                  WS_VISIBLE | WS_CHILD,
                                  10, yOffset, 200, 20, hwnd, NULL, g_hInstance, NULL);

        HWND hImage = CreateWindow("STATIC", "",
                                   WS_VISIBLE | WS_CHILD | SS_GRAYRECT,
                                   10, yOffset + 20, 200, 100, hwnd, NULL, g_hInstance, NULL);

        HWND hCaption = CreateWindow("STATIC", post.caption,
                                     WS_VISIBLE | WS_CHILD | SS_LEFT,
                                     10, yOffset + 125, 360, 20, hwnd, NULL, g_hInstance, NULL);

        // Save handles for later cleanup
        g_postControls.push_back(hUser);
        g_postControls.push_back(hImage);
        g_postControls.push_back(hCaption);

        yOffset += postSpacing;
    }

    // Calculate total height and set scroll range
    int totalContentHeight = yOffset + g_scrollOffset;
    int visibleHeight = 600 - TOP_UI_HEIGHT;
    int maxScroll = max(0, totalContentHeight - visibleHeight);
    SetScrollRange(hwnd, SB_VERT, 0, maxScroll, TRUE);
}

void ClearPostControls()
{
    for (size_t i = 0; i < g_postControls.size(); i++)
        DestroyWindow(g_postControls[i]);
    g_postControls.clear();
}

void LoadPosts()
{
    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    strcat(cwd, "\\posts.txt");

    FILE *file = fopen(cwd, "r");
    if (!file)
        return;

    g_posts.clear();

    char line[4096];
    while (fgets(line, sizeof(line), file))
    {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = 0;

        Post post;
        char *token = strtok(line, "::");
        if (token)
            strcpy(post.username, token);

        token = strtok(NULL, "::");
        if (token)
            strcpy(post.caption, token);

        token = strtok(NULL, "::");
        if (token)
            strcpy(post.imagePath, token);
        else
            strcpy(post.imagePath, "placeholder.jpg"); // fallback

        g_posts.push_back(post);
    }
    fclose(file);
}

void ShufflePosts()
{
    std::random_shuffle(g_posts.begin(), g_posts.end());
}

void AddUIControls(HWND hwnd)
{
    InitPosts();

    hwndLogoLabel = CreateWindow("STATIC", "WinstagramXP",
                                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                                 10, 10, 300, 20, hwnd, NULL, g_hInstance, NULL);

    hwndRefreshButton = CreateWindow("BUTTON", "Refresh",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     320, 10, 60, 20, hwnd, (HMENU)ID_REFRESH, g_hInstance, NULL);

    SetScrollRange(hwnd, SB_VERT, 0, 100, FALSE);
    SetScrollPos(hwnd, SB_VERT, 0, TRUE);

    RenderPosts(hwnd);
    UpdateScrollRange(hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    int maxScroll;
    SCROLLINFO si;
    switch (msg)
    {
    case WM_CREATE:
        AddUIControls(hwnd);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_REFRESH)
        {
            ClearPostControls();
            LoadPosts();
            ShufflePosts();
            RenderPosts(hwnd);
            UpdateScrollRange(hwnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_VSCROLL:
    {
        int nScrollCode = LOWORD(wParam);
        int nPos = HIWORD(wParam);

        switch (nScrollCode)
        {
        case SB_LINEUP:
            g_scrollOffset = max(g_scrollOffset - SCROLL_STEP, 0);
            break;
        case SB_LINEDOWN:
            memset(&si, 0, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE;
            GetScrollInfo(hwnd, SB_VERT, &si);
            maxScroll = si.nMax;

            g_scrollOffset = min(g_scrollOffset + SCROLL_STEP, maxScroll);
            break;
        case SB_THUMBTRACK:
            g_scrollOffset = nPos;
            break;
        }

        SetScrollPos(hwnd, SB_VERT, g_scrollOffset, TRUE);
        ClearPostControls(); // Delete old ones
        RenderPosts(hwnd);   // Re-render with offset
        UpdateScrollRange(hwnd);
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
