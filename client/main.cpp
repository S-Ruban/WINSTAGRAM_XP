#include <windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const char CLASS_NAME[] = "WinstagramXP";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Winstagram XP",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void AddUIControls(HWND hwnd)
{
    // Header: "Instagram" + Refresh button
    CreateWindow("STATIC", "Instagram",
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 10, 10, 300, 20,
                 hwnd, nullptr, nullptr, nullptr);

    CreateWindow("BUTTON", "Refresh",
                 WS_VISIBLE | WS_CHILD,
                 310, 10, 70, 20,
                 hwnd, (HMENU)1, nullptr, nullptr);

    // Dummy Post 1
    CreateWindow("STATIC", "User1",
                 WS_VISIBLE | WS_CHILD,
                 10, 50, 100, 20,
                 hwnd, nullptr, nullptr, nullptr);

    CreateWindow("STATIC", "[Image]",
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 10, 70, 360, 200,
                 hwnd, nullptr, nullptr, nullptr);

    CreateWindow("STATIC", "Caption: Hello world!",
                 WS_VISIBLE | WS_CHILD,
                 10, 280, 360, 20,
                 hwnd, nullptr, nullptr, nullptr);

    // Dummy Post 2
    CreateWindow("STATIC", "User2",
                 WS_VISIBLE | WS_CHILD,
                 10, 310, 100, 20,
                 hwnd, nullptr, nullptr, nullptr);

    CreateWindow("STATIC", "[Image]",
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 10, 330, 360, 200,
                 hwnd, nullptr, nullptr, nullptr);

    CreateWindow("STATIC", "Caption: My cat",
                 WS_VISIBLE | WS_CHILD,
                 10, 540, 360, 20,
                 hwnd, nullptr, nullptr, nullptr);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        AddUIControls(hwnd);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1)
        {
            MessageBox(hwnd, "Refresh clicked!", "Info", MB_OK);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
