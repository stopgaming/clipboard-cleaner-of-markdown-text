#include <windows.h>
#include <shellapi.h>
#include <string>
#include <algorithm>

#define ID_TRAY_ICON 1
#define WM_TRAYICON (WM_USER + 1)

HWND hWnd;
HINSTANCE GlobhInstance;

// Функция для удаления * и # из строки
std::wstring RemoveStarsAndHashes(const std::wstring& input) {
    std::wstring result;
    std::copy_if(input.begin(), input.end(), std::back_inserter(result),
        [](wchar_t c) { return c != L'*' && c != L'#'; });
    return result;
}

// Функция для обработки буфера обмена
void ProcessClipboard() {
    if (!OpenClipboard(nullptr)) {
        return;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData != nullptr) {
        wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
        if (pszText != nullptr) {
            std::wstring text(pszText);
            std::wstring cleaned = RemoveStarsAndHashes(text);
            
            // Подготовка нового буфера
            HGLOBAL hNewData = GlobalAlloc(GMEM_MOVEABLE, (cleaned.size() + 1) * sizeof(wchar_t));
            if (hNewData != nullptr) {
                wchar_t* pNewText = static_cast<wchar_t*>(GlobalLock(hNewData));
                if (pNewText != nullptr) {
                    wcscpy_s(pNewText, cleaned.size() + 1, cleaned.c_str());
                    GlobalUnlock(hNewData);
                    
                    EmptyClipboard();
                    SetClipboardData(CF_UNICODETEXT, hNewData);
                }
                GlobalUnlock(hNewData);
            }
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
}

// Обработчик сообщений
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            NOTIFYICONDATA nid = {0};
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = ID_TRAY_ICON;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_TRAYICON;
            //nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
nid.hIcon = LoadIcon(GlobhInstance, MAKEINTRESOURCE(102));
            wcscpy_s(nid.szTip, L"Clipboard Cleaner");
            
            Shell_NotifyIcon(NIM_ADD, &nid);
//SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)nid.hIcon);
//SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)nid.hIcon);

            break;
        }
        
        case WM_TRAYICON: {
            if (lParam == WM_LBUTTONDOWN) {
                ProcessClipboard();
            }
            if (lParam == WM_RBUTTONDOWN) {
                SendMessage(hWnd, WM_DESTROY, 0, 0);
            }
            break;
        }
        
        case WM_DESTROY: {
            NOTIFYICONDATA nid = {0};
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = ID_TRAY_ICON;
            
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
GlobhInstance = hInstance;
    wc.lpszClassName = L"ClipboardCleanerClass";
    
    if (!RegisterClass(&wc)) {
        return 1;
    }
    
    // Создание невидимого окна
    hWnd = CreateWindow(
        wc.lpszClassName,
        L"Clipboard Cleaner",
        0, 0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    
    if (!hWnd) {
        return 1;
    }
    
    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int) msg.wParam;
}
