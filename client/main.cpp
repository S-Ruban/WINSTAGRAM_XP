#include <windows.h>
#include <vector>
#include <algorithm> // for std::random_shuffle
#include <ctime>     // for srand
#include <iostream>
#include <stdio.h>

#define ID_REFRESH 1001

// Define the Post structure
struct Post
{
    const char *username;
    const char *caption;
    const char *imagePath; // Optional for now
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

HWND hwndLogoLabel;
HWND hwndRefreshButton;
HINSTANCE g_hInstance;

int yOffset = 50; // Start below the top bar
int postSpacing = 150;

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
    int yOffset = 50; // Space below header
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
}

void ClearPostControls()
{
    for (size_t i = 0; i < g_postControls.size(); i++)
        DestroyWindow(g_postControls[i]);
    g_postControls.clear();
}

void LoadAndShufflePosts()
{
    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    strcat(cwd, "\\posts.txt");

    FILE *file = fopen(cwd, "r");
    if (!file)
        return;

    g_posts.clear();

    char line[512];
    while (fgets(line, sizeof(line), file))
    {
        Post p;
        char *sep = strchr(line, '|');
        if (sep)
        {
            *sep = '\0'; // split at the '|'
            char *username = line;
            char *caption = sep + 1;

            // strip trailing newline
            char *newline = strchr(caption, '\n');
            if (newline)
                *newline = '\0';

            p.username = line;
            p.caption = line;
        }
        g_posts.push_back(p);
    }

    fclose(file);
}

void AddUIControls(HWND hwnd)
{
    InitPosts();

    CreateWindow("STATIC", "WinstagramXP",
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 10, 10, 300, 20, hwnd, NULL, g_hInstance, NULL);

    CreateWindow("BUTTON", "Refresh",
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 320, 10, 60, 20, hwnd, (HMENU)ID_REFRESH, g_hInstance, NULL);

    RenderPosts(hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        AddUIControls(hwnd);
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_REFRESH)
        {
            ClearPostControls();
            LoadAndShufflePosts();
            RenderPosts(hwnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
