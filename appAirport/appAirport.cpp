// appAirport.cpp : Аэропорт - Система управления рейсами
//

#include "framework.h"
#include "appAirport.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// ID контролов
#define IDC_LIST_FLIGHTS       1001
#define IDC_LIST_RESULTS       1002
#define IDC_BTN_ADD_FLIGHT     1003
#define IDC_BTN_DEL_FLIGHT     1004
#define IDC_BTN_SAVE           1005
#define IDC_BTN_LOAD           1006
#define IDC_BTN_CLEAR          1007
#define IDC_BTN_CHECK_SEATS    1008
#define IDC_BTN_FIND_DEST      1009
#define IDC_BTN_MAX_PRICE      1010
#define IDC_BTN_NEAREST        1011
#define IDC_BTN_SAVE_QUERY     1012
#define IDC_BTN_SELL_TICKET    1013
#define IDC_EDIT_FLIGHT_NUM    1014
#define IDC_EDIT_DEST          1015
#define IDC_EDIT_DEPART        1016
#define IDC_EDIT_TIME          1017
#define IDC_EDIT_DURATION      1018
#define IDC_EDIT_SEATS         1019
#define IDC_EDIT_QUERY_FLIGHT  1020
#define IDC_EDIT_QUERY_DEST    1021
#define IDC_EDIT_TICKET_FLIGHT 1022
#define IDC_EDIT_PASSENGER     1023
#define IDC_EDIT_PRICE         1024
#define IDC_EDIT_TICKET_CASHIER 1025

// Глобальные переменные
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Хэндлы контролов
HWND hListFlights, hListResults;
HWND hEditFlightNum, hEditDest, hEditDepart, hEditTime, hEditDuration, hEditSeats;
HWND hEditQueryFlight, hEditQueryDest;
HWND hEditTicketFlight, hEditPassenger, hEditPrice, hEditTicketCashier;

// Объявления функций
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hWnd);

// Точка входа
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_APPAIRPORT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPAIRPORT));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPAIRPORT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_APPAIRPORT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"Аэропорт - Система управления рейсами",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1400, 950, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

// Создание интерфейса
void CreateControls(HWND hWnd) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // ========== ТАБЛИЦА РЕЙСОВ ==========
    hListFlights = CreateWindowW(WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 10, 950, 320, hWnd, (HMENU)IDC_LIST_FLIGHTS, hInst, NULL);

    LVCOLUMNW lvc = { LVCF_TEXT | LVCF_WIDTH };
    const wchar_t* cols[] = { L"Номер рейса", L"Назначение", L"Отправление", L"Время вылета", L"Длит.(мин)", L"Мест" };
    int widths[] = { 130, 170, 170, 210, 100, 90 };
    for (int i = 0; i < 6; i++) {
        lvc.cx = widths[i];
        lvc.pszText = const_cast<wchar_t*>(cols[i]);
        ListView_InsertColumn(hListFlights, i, &lvc);
    }

    // ========== ПАНЕЛЬ ДОБАВЛЕНИЯ РЕЙСА ==========
    CreateWindowW(L"STATIC", L"ДОБАВЛЕНИЕ РЕЙСА", WS_CHILD | WS_VISIBLE | SS_CENTER,
        980, 10, 370, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 980, 50, 100, 30, hWnd, NULL, hInst, NULL);
    hEditFlightNum = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 50, 240, 30, hWnd, (HMENU)IDC_EDIT_FLIGHT_NUM, hInst, NULL);

    CreateWindowW(L"STATIC", L"Назначение:", WS_CHILD | WS_VISIBLE, 980, 90, 100, 30, hWnd, NULL, hInst, NULL);
    hEditDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 90, 240, 30, hWnd, (HMENU)IDC_EDIT_DEST, hInst, NULL);

    CreateWindowW(L"STATIC", L"Отправление:", WS_CHILD | WS_VISIBLE, 980, 130, 100, 30, hWnd, NULL, hInst, NULL);
    hEditDepart = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 130, 240, 30, hWnd, (HMENU)IDC_EDIT_DEPART, hInst, NULL);

    CreateWindowW(L"STATIC", L"Время (ГГГГ-ММ-ДД ЧЧ:ММ):", WS_CHILD | WS_VISIBLE, 980, 170, 200, 30, hWnd, NULL, hInst, NULL);
    hEditTime = CreateWindowW(L"EDIT", L"2025-05-10 08:00", WS_CHILD | WS_VISIBLE | WS_BORDER, 1190, 170, 160, 30, hWnd, (HMENU)IDC_EDIT_TIME, hInst, NULL);

    CreateWindowW(L"STATIC", L"Длительность (мин):", WS_CHILD | WS_VISIBLE, 980, 210, 130, 30, hWnd, NULL, hInst, NULL);
    hEditDuration = CreateWindowW(L"EDIT", L"120", WS_CHILD | WS_VISIBLE | WS_BORDER, 1120, 210, 120, 30, hWnd, (HMENU)IDC_EDIT_DURATION, hInst, NULL);

    CreateWindowW(L"STATIC", L"Кол-во мест:", WS_CHILD | WS_VISIBLE, 980, 250, 100, 30, hWnd, NULL, hInst, NULL);
    hEditSeats = CreateWindowW(L"EDIT", L"50", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 250, 120, 30, hWnd, (HMENU)IDC_EDIT_SEATS, hInst, NULL);

    CreateWindowW(L"BUTTON", L"+ ДОБАВИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        980, 295, 170, 40, hWnd, (HMENU)IDC_BTN_ADD_FLIGHT, hInst, NULL);
    CreateWindowW(L"BUTTON", L"- УДАЛИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1165, 295, 170, 40, hWnd, (HMENU)IDC_BTN_DEL_FLIGHT, hInst, NULL);

    // ========== ПАНЕЛЬ УПРАВЛЕНИЯ ==========
    CreateWindowW(L"BUTTON", L"ЗАГРУЗИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 345, 140, 40, hWnd, (HMENU)IDC_BTN_LOAD, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        160, 345, 140, 40, hWnd, (HMENU)IDC_BTN_SAVE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ОЧИСТИТЬ ВСЁ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        310, 345, 140, 40, hWnd, (HMENU)IDC_BTN_CLEAR, hInst, NULL);

    // ========== ЗАПРОСЫ ==========
    CreateWindowW(L"STATIC", L"ЗАПРОСЫ", WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 400, 650, 30, hWnd, NULL, hInst, NULL);

    // Запрос 1
    CreateWindowW(L"BUTTON", L"Свободные места", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 440, 140, 35, hWnd, (HMENU)IDC_BTN_CHECK_SEATS, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 165, 445, 90, 30, hWnd, NULL, hInst, NULL);
    hEditQueryFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 265, 445, 150, 30, hWnd, (HMENU)IDC_EDIT_QUERY_FLIGHT, hInst, NULL);

    // Запрос 2
    CreateWindowW(L"BUTTON", L"По направлению", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 485, 140, 35, hWnd, (HMENU)IDC_BTN_FIND_DEST, hInst, NULL);
    CreateWindowW(L"STATIC", L"Город:", WS_CHILD | WS_VISIBLE, 165, 490, 90, 30, hWnd, NULL, hInst, NULL);
    hEditQueryDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 265, 490, 150, 30, hWnd, (HMENU)IDC_EDIT_QUERY_DEST, hInst, NULL);

    // Запрос 3
    CreateWindowW(L"BUTTON", L"Макс.цена билета", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 530, 140, 35, hWnd, (HMENU)IDC_BTN_MAX_PRICE, hInst, NULL);
    CreateWindowW(L"STATIC", L"(используйте номер рейса выше)", WS_CHILD | WS_VISIBLE, 165, 535, 220, 30, hWnd, NULL, hInst, NULL);

    // Запрос 4
    CreateWindowW(L"BUTTON", L"Ближайший рейс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 575, 140, 35, hWnd, (HMENU)IDC_BTN_NEAREST, hInst, NULL);
    CreateWindowW(L"STATIC", L"(используйте город выше)", WS_CHILD | WS_VISIBLE, 165, 580, 200, 30, hWnd, NULL, hInst, NULL);

    // Сохранение результата
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ РЕЗУЛЬТАТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 625, 180, 40, hWnd, (HMENU)IDC_BTN_SAVE_QUERY, hInst, NULL);

    // ========== ТАБЛИЦА РЕЗУЛЬТАТОВ ==========
    CreateWindowW(L"STATIC", L"РЕЗУЛЬТАТЫ ЗАПРОСОВ", WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 680, 650, 30, hWnd, NULL, hInst, NULL);

    hListResults = CreateWindowW(WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT,
        10, 715, 950, 100, hWnd, (HMENU)IDC_LIST_RESULTS, hInst, NULL);
    lvc.cx = 930;
    lvc.pszText = const_cast<wchar_t*>(L"Результат");
    ListView_InsertColumn(hListResults, 0, &lvc);

    // ========== ПРОДАЖА БИЛЕТОВ ==========
    CreateWindowW(L"STATIC", L"ПРОДАЖА БИЛЕТОВ", WS_CHILD | WS_VISIBLE | SS_CENTER,
        980, 345, 370, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 980, 390, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 390, 240, 30, hWnd, (HMENU)IDC_EDIT_TICKET_FLIGHT, hInst, NULL);

    CreateWindowW(L"STATIC", L"Пассажир (ФИО):", WS_CHILD | WS_VISIBLE, 980, 430, 110, 30, hWnd, NULL, hInst, NULL);
    hEditPassenger = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1100, 430, 280, 30, hWnd, (HMENU)IDC_EDIT_PASSENGER, hInst, NULL);

    CreateWindowW(L"STATIC", L"Цена (руб):", WS_CHILD | WS_VISIBLE, 980, 470, 100, 30, hWnd, NULL, hInst, NULL);
    hEditPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 470, 180, 30, hWnd, (HMENU)IDC_EDIT_PRICE, hInst, NULL);

    CreateWindowW(L"STATIC", L"Касса №:", WS_CHILD | WS_VISIBLE, 980, 510, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketCashier = CreateWindowW(L"EDIT", L"001", WS_CHILD | WS_VISIBLE | WS_BORDER, 1090, 510, 100, 30, hWnd, (HMENU)IDC_EDIT_TICKET_CASHIER, hInst, NULL);

    CreateWindowW(L"BUTTON", L"ПРОДАТЬ БИЛЕТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1090, 560, 200, 45, hWnd, (HMENU)IDC_BTN_SELL_TICKET, hInst, NULL);

    // Информационная строка
    CreateWindowW(L"STATIC", L"Совет: Для запросов Макс.цена и Ближайший рейс используйте номер рейса и город из полей выше",
        WS_CHILD | WS_VISIBLE, 10, 830, 900, 30, hWnd, NULL, hInst, NULL);
}

// Обработчик сообщений
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
            // Заглушки для кнопок
        case IDC_BTN_ADD_FLIGHT:
            MessageBoxW(hWnd, L"Добавление рейса", L"Информация", MB_OK);
            break;
        case IDC_BTN_DEL_FLIGHT:
            MessageBoxW(hWnd, L"Удаление рейса", L"Информация", MB_OK);
            break;
        case IDC_BTN_LOAD:
            MessageBoxW(hWnd, L"Загрузка данных", L"Информация", MB_OK);
            break;
        case IDC_BTN_SAVE:
            MessageBoxW(hWnd, L"Сохранение данных", L"Информация", MB_OK);
            break;
        case IDC_BTN_CLEAR:
            MessageBoxW(hWnd, L"Очистка данных", L"Информация", MB_OK);
            break;
        case IDC_BTN_CHECK_SEATS:
            MessageBoxW(hWnd, L"Проверка свободных мест", L"Информация", MB_OK);
            break;
        case IDC_BTN_FIND_DEST:
            MessageBoxW(hWnd, L"Поиск по направлению", L"Информация", MB_OK);
            break;
        case IDC_BTN_MAX_PRICE:
            MessageBoxW(hWnd, L"Максимальная цена билета", L"Информация", MB_OK);
            break;
        case IDC_BTN_NEAREST:
            MessageBoxW(hWnd, L"Ближайший рейс", L"Информация", MB_OK);
            break;
        case IDC_BTN_SAVE_QUERY:
            MessageBoxW(hWnd, L"Сохранение результата", L"Информация", MB_OK);
            break;
        case IDC_BTN_SELL_TICKET:
            MessageBoxW(hWnd, L"Продажа билета", L"Информация", MB_OK);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}