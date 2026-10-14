#include <windows.h>
#include <string>
#include <cmath>
#include <stdexcept>
using namespace std;
 
// ==========================================
// 1. МОДЕЛЬ (Model) - Бизнес-логика
// ==========================================
class WorldPopulationModel {
private:
    int day = 0, month = 0, year = 0;
    long long population = 0;
    long long diffDays = 0;
 
    // Опорная дата: 01.01.2024
    static const int refDay = 1, refMonth = 1, refYear = 2024;
    static constexpr long long refPopulation = 8019876189LL;
    // Средний суточный прирост населения Земли (чел./сутки)
    static constexpr long long dailyGrowth = 227000LL;
 
    // Юлианский день (для вычисления разницы между датами)
    static long long toJulianDay(int d, int m, int y) {
        int a = (14 - m) / 12;
        int yy = y + 4800 - a;
        int mm = m + 12 * a - 3;
        return d + (153 * mm + 2) / 5 + 365LL * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
    }
 
public:
    HWND viewWindow = nullptr;
 
    void updateData(int d, int m, int y) {
        static const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        int maxDay = daysInMonth[m - 1] + ((m == 2 && leap) ? 1 : 0);
 
        if (y < 1900 || y > 2100 || m < 1 || m > 12 || d < 1 || d > maxDay) {
            throw invalid_argument("Введены некорректные данные!");
        }
 
        day = d; month = m; year = y;
        diffDays = toJulianDay(d, m, y) - toJulianDay(refDay, refMonth, refYear);
        population = refPopulation + diffDays * dailyGrowth;
        if (population < 0) population = 0;
 
        if (viewWindow) {
            PostMessageW(viewWindow, WM_USER + 1, 0, 0);
        }
    }
 
    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
    long long getPopulation() const { return population; }
};
 
WorldPopulationModel g_model;
 
// ==========================================
// 2. КОНТРОЛЛЕР (Controller)
// ==========================================
class WorldPopulationController {
public:
    bool trySaveData(const wstring& dStr, const wstring& mStr, const wstring& yStr,
                      wstring& outError) {
        try {
            int d = stoi(dStr);
            int m = stoi(mStr);
            int y = stoi(yStr);
            g_model.updateData(d, m, y);
            return true;
        }
        catch (...) {
            outError = L"Ошибка ввода! Проверьте корректность даты.";
            return false;
        }
    }
};
 
WorldPopulationController g_controller;
 
HWND hDay, hMonth, hYear;
HWND hLblDate, hLblPopulation;
 
// Форматирование числа с разделением по разрядам (пробелами)
wstring formatNumber(long long n) {
    wstring s = to_wstring(n);
    wstring res;
    int cnt = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        res = s[i] + res;
        if (++cnt % 3 == 0 && i != 0) res = L" " + res;
    }
    return res;
}
 
// ==========================================
// 3. ПРЕДСТАВЛЕНИЕ (View) - Интерфейс
// ==========================================
LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        CreateWindowW(L"Static", L"Дата (ДД ММ ГГГГ):", WS_VISIBLE | WS_CHILD,
            10, 10, 150, 20, hwnd, NULL, NULL, NULL);
        hDay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
            170, 10, 30, 22, hwnd, NULL, NULL, NULL);
        hMonth = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
            205, 10, 30, 22, hwnd, NULL, NULL, NULL);
        hYear = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
            240, 10, 55, 22, hwnd, NULL, NULL, NULL);
 
        if (g_model.getYear() != 0) {
            SetWindowTextW(hDay, to_wstring(g_model.getDay()).c_str());
            SetWindowTextW(hMonth, to_wstring(g_model.getMonth()).c_str());
            SetWindowTextW(hYear, to_wstring(g_model.getYear()).c_str());
        }
 
        CreateWindowW(L"Button", L"Сохранить", WS_VISIBLE | WS_CHILD,
            105, 140, 130, 32, hwnd, (HMENU)1, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wp) == 1) {
            wchar_t d[10], m[10], y[10];
            GetWindowTextW(hDay, d, 10);
            GetWindowTextW(hMonth, m, 10);
            GetWindowTextW(hYear, y, 10);
 
            wstring error;
            if (g_controller.trySaveData(d, m, y, error)) {
                DestroyWindow(hwnd);
            } else {
                MessageBoxW(hwnd, error.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }
        break;
    }
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}
 
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        g_model.viewWindow = hwnd;
        CreateWindowW(L"Button", L"Ввести данные", WS_VISIBLE | WS_CHILD,
            20, 20, 150, 32, hwnd, (HMENU)1, NULL, NULL);
        hLblDate = CreateWindowW(L"Static", L"Дата: -", WS_VISIBLE | WS_CHILD,
            20, 75, 440, 20, hwnd, NULL, NULL, NULL);
        hLblPopulation = CreateWindowW(L"Static", L"Население Земли: Не рассчитано",
            WS_VISIBLE | WS_CHILD, 20, 107, 440, 20, hwnd, NULL, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wp) == 1) {
            CreateWindowExW(0, L"InputWindowClass", L"Input Data",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 340, 210, hwnd, NULL, NULL, NULL);
        }
        break;
    }
    case WM_USER + 1: {
        wchar_t buf[16];
        swprintf(buf, 16, L"%02d.%02d.%04d", g_model.getDay(), g_model.getMonth(), g_model.getYear());
        wstring dateText = wstring(L"Дата: ") + buf;
        wstring popText = L"Население Земли: " + formatNumber(g_model.getPopulation()) + L" чел.";
 
        SetWindowTextW(hLblDate, dateText.c_str());
        SetWindowTextW(hLblPopulation, popText.c_str());
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}
 
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSW wcMain = {0};
    wcMain.lpfnWndProc = MainWndProc;
    wcMain.hInstance = hInst;
    wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcMain.lpszClassName = L"MainWindowClass";
    RegisterClassW(&wcMain);
 
    WNDCLASSW wcInput = {0};
    wcInput.lpfnWndProc = InputWndProc;
    wcInput.hInstance = hInst;
    wcInput.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcInput.lpszClassName = L"InputWindowClass";
    RegisterClassW(&wcInput);
 
    HWND hwnd = CreateWindowExW(
        0, L"MainWindowClass", L"WorldPopulationAPP",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 490, 200,
        NULL, NULL, hInst, NULL
    );
 
    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
