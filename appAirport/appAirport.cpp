// appAirport.cpp : Аэропорт - Система управления рейсами
//

#include "framework.h"
#include "appAirport.h"
#include <commctrl.h>
#include "Aircraft.h"
#include "Flight.h"
#include "Ticket.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <codecvt>
#include <locale>
#include <set>

#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// ID контролов
#define IDC_TAB_MAIN           1001
#define IDC_LIST_FLIGHTS       1002
#define IDC_LIST_RESULTS       1003
#define IDC_BTN_ADD_FLIGHT     1004
#define IDC_BTN_DEL_FLIGHT     1005
#define IDC_BTN_SAVE           1006
#define IDC_BTN_LOAD           1007
#define IDC_BTN_CLEAR          1008
#define IDC_BTN_CHECK_SEATS    1009
#define IDC_BTN_FIND_DEST      1010
#define IDC_BTN_MAX_PRICE      1011
#define IDC_BTN_NEAREST        1012
#define IDC_BTN_SAVE_QUERY     1013
#define IDC_BTN_SELL_TICKET    1014
#define IDC_EDIT_FLIGHT_NUM    1015
#define IDC_EDIT_DEST          1016
#define IDC_EDIT_DEPART        1017
#define IDC_EDIT_TIME          1018
#define IDC_EDIT_DURATION      1019
#define IDC_EDIT_SEATS         1020
#define IDC_EDIT_QUERY_FLIGHT  1021
#define IDC_EDIT_QUERY_DEST    1022
#define IDC_EDIT_TICKET_FLIGHT 1023
#define IDC_EDIT_PASSENGER     1024
#define IDC_EDIT_PRICE         1025
#define IDC_EDIT_TICKET_CASHIER 1026
#define IDC_COMBO_AIRCRAFT     1027

// Глобальные переменные
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND g_hWnd;
HWND hTabControl;

// Данные приложения
std::vector<Aircraft> aircrafts;
std::vector<Flight> flights;
std::vector<Ticket> tickets;
std::vector<std::string> lastQueryResults;

// Файлы
const std::string AIRCRAFTS_FILE = "aircrafts.txt";
const std::string FLIGHTS_FILE = "flights.txt";
const std::string TICKETS_FILE = "tickets.txt";

// Хэндлы контролов ВКЛАДКИ 1
HWND hListFlights;
HWND hEditFlightNum, hEditDest, hEditDepart, hEditTime, hEditDuration, hEditSeats, hComboAircraft;
HWND hBtnAddFlight, hBtnDeleteFlight, hBtnLoad, hBtnSave, hBtnClear;
HWND hEditTicketFlight, hEditPassenger, hEditPrice, hEditTicketCashier, hBtnSellTicket;

// Хэндлы контролов ВКЛАДКИ 2
HWND hListResults;
HWND hEditQueryFlight, hEditQueryDest;
HWND hBtnCheckSeats, hBtnFindDest, hBtnMaxPrice, hBtnNearest, hBtnSaveQuery;

// Объявления функций
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hWnd);
void RefreshFlightsList();
void RefreshResultsListW(const std::vector<std::wstring>& results);
void UpdateAircraftCombo();
void LoadAircrafts();
void LoadData();
void SaveData();
void LoadTickets();
void SaveTickets();
void AddFlight();
void DeleteFlight();
void SellTicket();
void QueryFreeSeats();
void QueryFlightsByDestination();
void QueryMaxTicketPrice();
void QueryNearestFlight();
void SaveQueryResult();
std::time_t ParseDateTime(const std::string& str);
std::string WCharToString(const wchar_t* wstr);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);
double SafeStod(const std::string& str, double defaultValue = 0.0);
int SafeStoi(const std::string& str, int defaultValue = 0);
std::string TrimString(const std::string& str);

// ========== ТОЧКА ВХОДА ==========
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
    g_hWnd = CreateWindowW(szWindowClass, L"Аэропорт - Система управления рейсами",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1400, 850, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) return FALSE;
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    return TRUE;
}

// ========== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==========
std::string TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// ========== ЗАГРУЗКА САМОЛЁТОВ ==========
void LoadAircrafts() {
    aircrafts.clear();

    std::ifstream file(AIRCRAFTS_FILE);
    if (!file.is_open()) {
        std::ofstream newFile(AIRCRAFTS_FILE);
        if (newFile.is_open()) {
            newFile << "Boeing 737-800;Среднемагистральный;180\n";
            newFile << "Airbus A320;Среднемагистральный;160\n";
            newFile << "Sukhoi Superjet 100;Ближнемагистральный;100\n";
            newFile << "Boeing 777-300ER;Дальнемагистральный;300\n";
            newFile << "Airbus A380;Широкофюзеляжный;500\n";
            newFile.close();
        }
        file.open(AIRCRAFTS_FILE);
    }

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 3) {
                aircrafts.push_back(Aircraft(parts[0], parts[1], SafeStoi(parts[2], 150)));
            }
        }
        file.close();
    }

    if (aircrafts.empty()) {
        aircrafts.push_back(Aircraft("Boeing 737-800", "Среднемагистральный", 180));
        aircrafts.push_back(Aircraft("Airbus A320", "Среднемагистральный", 160));
        aircrafts.push_back(Aircraft("Sukhoi Superjet 100", "Ближнемагистральный", 100));
    }
}

// ========== ОБНОВЛЕНИЕ КОМБОБОКСА ==========
void UpdateAircraftCombo() {
    SendMessage(hComboAircraft, CB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < aircrafts.size(); i++) {
        std::string display = aircrafts[i].getName() + " (" + aircrafts[i].getCategory() + ", " + std::to_string(aircrafts[i].getSeats()) + " мест)";
        std::wstring wdisplay = StringToWString(display);
        SendMessage(hComboAircraft, CB_ADDSTRING, 0, (LPARAM)wdisplay.c_str());
    }
    SendMessage(hComboAircraft, CB_SETCURSEL, 0, 0);
}

// ========== ПРЕОБРАЗОВАНИЕ КОДИРОВОК ==========
std::string WCharToString(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 1) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, NULL, NULL);
    return result;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    if (size <= 0) return L"";
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], size);
    return result;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 1) return "";
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

// ========== СОЗДАНИЕ ИНТЕРФЕЙСА ==========
void CreateControls(HWND hWnd) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // Создаём TabControl
    hTabControl = CreateWindowW(WC_TABCONTROL, NULL,
        WS_CHILD | WS_VISIBLE,
        5, 5, 1360, 780, hWnd, (HMENU)IDC_TAB_MAIN, hInst, NULL);

    TCITEMW tie = { TCIF_TEXT };
    wchar_t tab1[] = L"1. Управление рейсами и продажа билетов";
    wchar_t tab2[] = L"2. Запросы";
    tie.pszText = tab1;
    TabCtrl_InsertItem(hTabControl, 0, &tie);
    tie.pszText = tab2;
    TabCtrl_InsertItem(hTabControl, 1, &tie);

    // ==================== ВКЛАДКА 1 ====================
    // Таблица рейсов (слева)
    hListFlights = CreateWindowW(WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 40, 700, 450, hTabControl, (HMENU)IDC_LIST_FLIGHTS, hInst, NULL);

    LVCOLUMNW lvc = { LVCF_TEXT | LVCF_WIDTH };
    const wchar_t* cols[] = { L"Номер", L"Самолёт", L"Назначение", L"Отправление", L"Время", L"Длит.", L"Мест" };
    int widths[] = { 80, 170, 120, 120, 130, 50, 50 };
    for (int i = 0; i < 7; i++) {
        lvc.cx = widths[i];
        lvc.pszText = const_cast<wchar_t*>(cols[i]);
        ListView_InsertColumn(hListFlights, i, &lvc);
    }

    // Панель добавления рейса (справа сверху)
    CreateWindowW(L"BUTTON", L"ДОБАВЛЕНИЕ РЕЙСА", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        730, 40, 370, 260, hTabControl, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер:", WS_CHILD | WS_VISIBLE, 750, 70, 60, 25, hTabControl, NULL, hInst, NULL);
    hEditFlightNum = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 820, 70, 150, 25, hTabControl, (HMENU)IDC_EDIT_FLIGHT_NUM, hInst, NULL);

    CreateWindowW(L"STATIC", L"Назначение:", WS_CHILD | WS_VISIBLE, 750, 105, 80, 25, hTabControl, NULL, hInst, NULL);
    hEditDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 840, 105, 130, 25, hTabControl, (HMENU)IDC_EDIT_DEST, hInst, NULL);

    CreateWindowW(L"STATIC", L"Отправление:", WS_CHILD | WS_VISIBLE, 750, 140, 80, 25, hTabControl, NULL, hInst, NULL);
    hEditDepart = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 840, 140, 130, 25, hTabControl, (HMENU)IDC_EDIT_DEPART, hInst, NULL);

    CreateWindowW(L"STATIC", L"Время:", WS_CHILD | WS_VISIBLE, 750, 175, 50, 25, hTabControl, NULL, hInst, NULL);
    hEditTime = CreateWindowW(L"EDIT", L"2025-05-10 08:00", WS_CHILD | WS_VISIBLE | WS_BORDER, 810, 175, 160, 25, hTabControl, (HMENU)IDC_EDIT_TIME, hInst, NULL);

    CreateWindowW(L"STATIC", L"Длит.(мин):", WS_CHILD | WS_VISIBLE, 750, 210, 70, 25, hTabControl, NULL, hInst, NULL);
    hEditDuration = CreateWindowW(L"EDIT", L"120", WS_CHILD | WS_VISIBLE | WS_BORDER, 830, 210, 80, 25, hTabControl, (HMENU)IDC_EDIT_DURATION, hInst, NULL);

    CreateWindowW(L"STATIC", L"Мест:", WS_CHILD | WS_VISIBLE, 750, 245, 50, 25, hTabControl, NULL, hInst, NULL);
    hEditSeats = CreateWindowW(L"EDIT", L"50", WS_CHILD | WS_VISIBLE | WS_BORDER, 810, 245, 80, 25, hTabControl, (HMENU)IDC_EDIT_SEATS, hInst, NULL);

    CreateWindowW(L"STATIC", L"Самолёт:", WS_CHILD | WS_VISIBLE, 750, 280, 60, 25, hTabControl, NULL, hInst, NULL);
    hComboAircraft = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        820, 280, 250, 120, hTabControl, (HMENU)IDC_COMBO_AIRCRAFT, hInst, NULL);

    // Кнопки добавления/удаления
    hBtnAddFlight = CreateWindowW(L"BUTTON", L"+ ДОБАВИТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        740, 320, 120, 30, hTabControl, (HMENU)IDC_BTN_ADD_FLIGHT, hInst, NULL);
    hBtnDeleteFlight = CreateWindowW(L"BUTTON", L"- УДАЛИТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        880, 320, 120, 30, hTabControl, (HMENU)IDC_BTN_DEL_FLIGHT, hInst, NULL);

    // Кнопки управления
    CreateWindowW(L"BUTTON", L"ЗАГРУЗИТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 510, 100, 35, hTabControl, (HMENU)IDC_BTN_LOAD, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        120, 510, 100, 35, hTabControl, (HMENU)IDC_BTN_SAVE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ОЧИСТИТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        230, 510, 100, 35, hTabControl, (HMENU)IDC_BTN_CLEAR, hInst, NULL);

    // Продажа билетов (справа снизу)
    CreateWindowW(L"BUTTON", L"ПРОДАЖА БИЛЕТОВ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        730, 360, 370, 200, hTabControl, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 750, 390, 80, 25, hTabControl, NULL, hInst, NULL);
    hEditTicketFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 840, 390, 150, 25, hTabControl, (HMENU)IDC_EDIT_TICKET_FLIGHT, hInst, NULL);

    CreateWindowW(L"STATIC", L"Пассажир:", WS_CHILD | WS_VISIBLE, 750, 425, 70, 25, hTabControl, NULL, hInst, NULL);
    hEditPassenger = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 830, 425, 200, 25, hTabControl, (HMENU)IDC_EDIT_PASSENGER, hInst, NULL);

    CreateWindowW(L"STATIC", L"Цена:", WS_CHILD | WS_VISIBLE, 750, 460, 50, 25, hTabControl, NULL, hInst, NULL);
    hEditPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 810, 460, 100, 25, hTabControl, (HMENU)IDC_EDIT_PRICE, hInst, NULL);

    CreateWindowW(L"STATIC", L"Касса:", WS_CHILD | WS_VISIBLE, 750, 495, 50, 25, hTabControl, NULL, hInst, NULL);
    hEditTicketCashier = CreateWindowW(L"EDIT", L"001", WS_CHILD | WS_VISIBLE | WS_BORDER, 810, 495, 80, 25, hTabControl, (HMENU)IDC_EDIT_TICKET_CASHIER, hInst, NULL);

    hBtnSellTicket = CreateWindowW(L"BUTTON", L"ПРОДАТЬ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        940, 490, 100, 35, hTabControl, (HMENU)IDC_BTN_SELL_TICKET, hInst, NULL);

    // ==================== ВКЛАДКА 2 ====================
    CreateWindowW(L"BUTTON", L"1. Свободные места", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 40, 600, 60, hTabControl, NULL, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 30, 65, 70, 25, hTabControl, NULL, hInst, NULL);
    hEditQueryFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 65, 150, 25, hTabControl, (HMENU)IDC_EDIT_QUERY_FLIGHT, hInst, NULL);
    hBtnCheckSeats = CreateWindowW(L"BUTTON", L"Проверить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 63, 100, 28, hTabControl, (HMENU)IDC_BTN_CHECK_SEATS, hInst, NULL);

    CreateWindowW(L"BUTTON", L"2. По направлению", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 110, 600, 60, hTabControl, NULL, hInst, NULL);
    CreateWindowW(L"STATIC", L"Город:", WS_CHILD | WS_VISIBLE, 30, 135, 70, 25, hTabControl, NULL, hInst, NULL);
    hEditQueryDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 135, 150, 25, hTabControl, (HMENU)IDC_EDIT_QUERY_DEST, hInst, NULL);
    hBtnFindDest = CreateWindowW(L"BUTTON", L"Найти", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 133, 100, 28, hTabControl, (HMENU)IDC_BTN_FIND_DEST, hInst, NULL);

    CreateWindowW(L"BUTTON", L"3. Макс. цена билета", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 180, 600, 50, hTabControl, NULL, hInst, NULL);
    hBtnMaxPrice = CreateWindowW(L"BUTTON", L"Найти макс. цену", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        170, 193, 140, 28, hTabControl, (HMENU)IDC_BTN_MAX_PRICE, hInst, NULL);

    CreateWindowW(L"BUTTON", L"4. Ближайший рейс", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 240, 600, 50, hTabControl, NULL, hInst, NULL);
    hBtnNearest = CreateWindowW(L"BUTTON", L"Найти ближайший", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        170, 253, 140, 28, hTabControl, (HMENU)IDC_BTN_NEAREST, hInst, NULL);

    hBtnSaveQuery = CreateWindowW(L"BUTTON", L"СОХРАНИТЬ РЕЗУЛЬТАТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 310, 200, 35, hTabControl, (HMENU)IDC_BTN_SAVE_QUERY, hInst, NULL);

    CreateWindowW(L"STATIC", L"РЕЗУЛЬТАТЫ ЗАПРОСОВ", WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 360, 600, 25, hTabControl, NULL, hInst, NULL);
    hListResults = CreateWindowW(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT,
        20, 390, 600, 320, hTabControl, (HMENU)IDC_LIST_RESULTS, hInst, NULL);
    lvc.cx = 580;
    lvc.pszText = const_cast<wchar_t*>(L"Результат");
    ListView_InsertColumn(hListResults, 0, &lvc);

    // Скрываем элементы второй вкладки
    ShowWindow(hListResults, SW_HIDE);
    ShowWindow(hEditQueryFlight, SW_HIDE);
    ShowWindow(hEditQueryDest, SW_HIDE);
    ShowWindow(hBtnCheckSeats, SW_HIDE);
    ShowWindow(hBtnFindDest, SW_HIDE);
    ShowWindow(hBtnMaxPrice, SW_HIDE);
    ShowWindow(hBtnNearest, SW_HIDE);
    ShowWindow(hBtnSaveQuery, SW_HIDE);
    for (int i = 1008; i <= 1013; i++) {
        ShowWindow(GetDlgItem(hTabControl, i), SW_HIDE);
    }
    ShowWindow(GetDlgItem(hTabControl, 1020), SW_HIDE);
    ShowWindow(GetDlgItem(hTabControl, 1021), SW_HIDE);
    ShowWindow(GetDlgItem(hTabControl, 1022), SW_HIDE);
    ShowWindow(GetDlgItem(hTabControl, 1023), SW_HIDE);
    ShowWindow(GetDlgItem(hTabControl, 1024), SW_HIDE);
    ShowWindow(GetDlgItem(hTabControl, 1025), SW_HIDE);

    // Загружаем данные
    LoadAircrafts();
    UpdateAircraftCombo();
    LoadData();
    LoadTickets();
}

// ========== ОБНОВЛЕНИЕ ТАБЛИЦЫ РЕЙСОВ ==========
void RefreshFlightsList() {
    ListView_DeleteAllItems(hListFlights);
    for (size_t i = 0; i < flights.size(); i++) {
        std::wstring num = StringToWString(flights[i].getNumber());
        std::wstring aircraft = StringToWString(flights[i].getAircraft() ? flights[i].getAircraft()->getName() : "Нет");
        std::wstring dest = StringToWString(flights[i].getDestination());
        std::wstring depart = StringToWString(flights[i].getDeparturePoint());
        std::wstring time = StringToWString(flights[i].getDepartureTimeStr());
        std::wstring dur = std::to_wstring(flights[i].getDurationMinutes());
        std::wstring seats = std::to_wstring(flights[i].getFreeSeats());

        LVITEMW lvi = { LVIF_TEXT, (int)i, 0 };
        lvi.pszText = (LPWSTR)num.c_str();
        ListView_InsertItem(hListFlights, &lvi);
        ListView_SetItemText(hListFlights, (int)i, 1, (LPWSTR)aircraft.c_str());
        ListView_SetItemText(hListFlights, (int)i, 2, (LPWSTR)dest.c_str());
        ListView_SetItemText(hListFlights, (int)i, 3, (LPWSTR)depart.c_str());
        ListView_SetItemText(hListFlights, (int)i, 4, (LPWSTR)time.c_str());
        ListView_SetItemText(hListFlights, (int)i, 5, (LPWSTR)dur.c_str());
        ListView_SetItemText(hListFlights, (int)i, 6, (LPWSTR)seats.c_str());
    }
}

// ========== ОБНОВЛЕНИЕ ТАБЛИЦЫ РЕЗУЛЬТАТОВ ==========
void RefreshResultsListW(const std::vector<std::wstring>& results) {
    ListView_DeleteAllItems(hListResults);
    for (size_t i = 0; i < results.size(); i++) {
        LVITEMW lvi = { LVIF_TEXT, (int)i, 0 };
        lvi.pszText = const_cast<LPWSTR>(results[i].c_str());
        ListView_InsertItem(hListResults, &lvi);
    }
}

// ========== ЗАГРУЗКА/СОХРАНЕНИЕ ==========
void LoadData() {
    flights.clear();
    std::ifstream file(FLIGHTS_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 7) {
                Flight flight;
                flight.setNumber(parts[0]);

                for (auto& ac : aircrafts) {
                    if (ac.getName() == parts[1]) {
                        flight.setAircraft(&ac);
                        break;
                    }
                }

                flight.setDeparturePoint(TrimString(parts[2]));
                flight.setDestination(TrimString(parts[3]));
                flight.setDepartureTime(ParseDateTime(parts[4]));
                flight.setDurationMinutes(SafeStoi(parts[5], 120));
                flight.setFreeSeats(SafeStoi(parts[6], 50));
                flights.push_back(flight);
            }
        }
        file.close();
    }
    RefreshFlightsList();
}

void SaveData() {
    std::ofstream file(FLIGHTS_FILE);
    for (const auto& f : flights) {
        std::string aircraftName = f.getAircraft() ? f.getAircraft()->getName() : "";
        file << f.getNumber() << ";" << aircraftName << ";"
            << f.getDeparturePoint() << ";" << f.getDestination() << ";"
            << f.getDepartureTimeStr() << ";" << f.getDurationMinutes() << ";" << f.getFreeSeats() << std::endl;
    }
    file.close();
}

void LoadTickets() {
    tickets.clear();
    std::ifstream file(TICKETS_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 4) {
                Ticket ticket;
                ticket.setCashierNumber(parts[0]);
                ticket.setFlightNumber(parts[1]);
                ticket.setPassengerName(parts[2]);
                ticket.setPrice(SafeStod(parts[3], 0.0));
                tickets.push_back(ticket);
            }
        }
        file.close();
    }
}

void SaveTickets() {
    std::ofstream file(TICKETS_FILE);
    for (const auto& t : tickets) {
        file << t.getCashierNumber() << ";" << t.getFlightNumber() << ";"
            << t.getPassengerName() << ";" << t.getPrice() << std::endl;
    }
    file.close();
}

// ========== ДОБАВЛЕНИЕ РЕЙСА ==========
void AddFlight() {
    wchar_t buffer[256];
    GetWindowTextW(hEditFlightNum, buffer, 256);
    std::string number = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditDepart, buffer, 256);
    std::string depart = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditTime, buffer, 256);
    std::string timeStr = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditDuration, buffer, 256);
    int duration = _wtoi(buffer);
    GetWindowTextW(hEditSeats, buffer, 256);
    int seats = _wtoi(buffer);

    int selIndex = (int)SendMessage(hComboAircraft, CB_GETCURSEL, 0, 0);
    if (selIndex == CB_ERR) selIndex = 0;
    Aircraft* selectedAircraft = &aircrafts[selIndex];

    if (number.empty() || dest.empty() || depart.empty()) {
        MessageBoxW(g_hWnd, L"Заполните все поля!", L"Ошибка", MB_OK);
        return;
    }

    if (duration <= 0) duration = 120;
    if (seats <= 0) seats = selectedAircraft->getSeats();

    Flight flight;
    flight.setNumber(number);
    flight.setAircraft(selectedAircraft);
    flight.setDestination(dest);
    flight.setDeparturePoint(depart);
    flight.setDepartureTime(ParseDateTime(timeStr));
    flight.setDurationMinutes(duration);
    flight.setFreeSeats(seats);

    flights.push_back(flight);
    SaveData();
    RefreshFlightsList();

    SetWindowTextW(hEditFlightNum, L"");
    SetWindowTextW(hEditDest, L"");
    SetWindowTextW(hEditDepart, L"");
    SetWindowTextW(hEditDuration, L"");
    SetWindowTextW(hEditSeats, L"");
    MessageBoxW(g_hWnd, L"Рейс добавлен!", L"Успех", MB_OK);
}

void DeleteFlight() {
    int sel = ListView_GetSelectionMark(hListFlights);
    if (sel < 0) {
        MessageBoxW(g_hWnd, L"Выберите рейс!", L"Ошибка", MB_OK);
        return;
    }
    wchar_t buffer[256];
    ListView_GetItemText(hListFlights, sel, 0, buffer, 256);
    std::string number = TrimString(WCharToString(buffer));

    for (size_t i = 0; i < flights.size(); i++) {
        if (flights[i].getNumber() == number) {
            flights.erase(flights.begin() + i);
            break;
        }
    }
    SaveData();
    RefreshFlightsList();
    MessageBoxW(g_hWnd, L"Рейс удалён!", L"Успех", MB_OK);
}

void SellTicket() {
    wchar_t buffer[256];
    GetWindowTextW(hEditTicketFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditPassenger, buffer, 256);
    std::string passenger = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditPrice, buffer, 256);
    double price = _wtof(buffer);
    GetWindowTextW(hEditTicketCashier, buffer, 256);
    std::string cashier = TrimString(WCharToString(buffer));

    if (flightNum.empty() || passenger.empty() || price <= 0) {
        MessageBoxW(g_hWnd, L"Заполните все поля!", L"Ошибка", MB_OK);
        return;
    }

    Flight* found = nullptr;
    for (auto& f : flights) {
        if (f.getNumber() == flightNum) {
            found = &f;
            break;
        }
    }
    if (!found) {
        MessageBoxW(g_hWnd, L"Рейс не найден!", L"Ошибка", MB_OK);
        return;
    }
    if (found->getFreeSeats() <= 0) {
        MessageBoxW(g_hWnd, L"Нет свободных мест!", L"Ошибка", MB_OK);
        return;
    }

    found->setFreeSeats(found->getFreeSeats() - 1);

    Ticket ticket;
    ticket.setCashierNumber(cashier);
    ticket.setFlightNumber(flightNum);
    ticket.setPassengerName(passenger);
    ticket.setPrice(price);
    tickets.push_back(ticket);

    SaveData();
    SaveTickets();
    RefreshFlightsList();

    SetWindowTextW(hEditTicketFlight, L"");
    SetWindowTextW(hEditPassenger, L"");
    SetWindowTextW(hEditPrice, L"");
    MessageBoxW(g_hWnd, L"Билет продан!", L"Успех", MB_OK);
}

// ========== ЗАПРОСЫ ==========
void QueryFreeSeats() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;

    for (const auto& f : flights) {
        if (f.getNumber() == flightNum) {
            results.push_back(L"Рейс " + StringToWString(flightNum) + L" - свободных мест: " + std::to_wstring(f.getFreeSeats()));
            lastQueryResults.clear();
            for (const auto& r : results) {
                lastQueryResults.push_back(WStringToString(r));
            }
            RefreshResultsListW(results);
            return;
        }
    }
    results.push_back(L"Рейс " + StringToWString(flightNum) + L" не найден");
    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryFlightsByDestination() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;

    for (const auto& f : flights) {
        if (TrimString(f.getDestination()) == dest) {
            results.push_back(L"Рейс " + StringToWString(f.getNumber()) + L" - свободных мест: " + std::to_wstring(f.getFreeSeats()));
        }
    }
    if (results.empty()) {
        results.push_back(L"Нет рейсов в " + StringToWString(dest));
    }
    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryMaxTicketPrice() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    double maxPrice = -1;
    std::string maxPassenger;

    for (const auto& t : tickets) {
        if (t.getFlightNumber() == flightNum && t.getPrice() > maxPrice) {
            maxPrice = t.getPrice();
            maxPassenger = t.getPassengerName();
        }
    }

    if (maxPrice >= 0) {
        results.push_back(L"Макс. цена билета на рейс " + StringToWString(flightNum) + L": " + std::to_wstring(maxPrice) + L" руб.");
        results.push_back(L"Пассажир: " + StringToWString(maxPassenger));
    }
    else {
        results.push_back(L"Нет проданных билетов на рейс " + StringToWString(flightNum));
    }
    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryNearestFlight() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    std::time_t now = std::time(nullptr);
    const Flight* nearest = nullptr;
    time_t minDiff = -1;

    for (const auto& f : flights) {
        if (TrimString(f.getDestination()) == dest && f.getFreeSeats() > 0) {
            time_t diff = std::difftime(f.getDepartureTime(), now);
            if (diff > 0 && (nearest == nullptr || diff < minDiff)) {
                nearest = &f;
                minDiff = diff;
            }
        }
    }

    if (nearest) {
        results.push_back(L"Ближайший рейс в " + StringToWString(dest) + L": " + StringToWString(nearest->getNumber()) +
            L" через " + std::to_wstring(minDiff / 60) + L" мин");
    }
    else {
        results.push_back(L"Нет рейсов в " + StringToWString(dest));
    }
    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void SaveQueryResult() {
    if (lastQueryResults.empty()) {
        MessageBoxW(g_hWnd, L"Нет результатов для сохранения!", L"Ошибка", MB_OK);
        return;
    }
    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        for (const auto& line : lastQueryResults) {
            file << line << std::endl;
        }
        file.close();
        MessageBoxW(g_hWnd, L"Результат сохранён в файл query_result.txt", L"Успех", MB_OK);
    }
    else {
        MessageBoxW(g_hWnd, L"Ошибка сохранения файла!", L"Ошибка", MB_OK);
    }
}

// ========== ВСПОМОГАТЕЛЬНЫЕ ==========
std::time_t ParseDateTime(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    if (ss.fail()) {
        return (std::time_t)0;
    }
    return std::mktime(&tm);
}

double SafeStod(const std::string& str, double defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    double result = strtod(str.c_str(), &endPtr);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return result;
}

int SafeStoi(const std::string& str, int defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    long result = strtol(str.c_str(), &endPtr, 10);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return (int)result;
}

// ========== ОБРАБОТЧИК СООБЩЕНИЙ ==========
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); break;
        case IDM_EXIT: DestroyWindow(hWnd); break;
        case IDC_BTN_ADD_FLIGHT: AddFlight(); break;
        case IDC_BTN_DEL_FLIGHT: DeleteFlight(); break;
        case IDC_BTN_LOAD: LoadData(); LoadTickets(); RefreshFlightsList(); break;
        case IDC_BTN_SAVE: SaveData(); SaveTickets(); break;
        case IDC_BTN_CLEAR: flights.clear(); tickets.clear(); SaveData(); SaveTickets(); RefreshFlightsList(); break;
        case IDC_BTN_CHECK_SEATS: QueryFreeSeats(); break;
        case IDC_BTN_FIND_DEST: QueryFlightsByDestination(); break;
        case IDC_BTN_MAX_PRICE: QueryMaxTicketPrice(); break;
        case IDC_BTN_NEAREST: QueryNearestFlight(); break;
        case IDC_BTN_SAVE_QUERY: SaveQueryResult(); break;
        case IDC_BTN_SELL_TICKET: SellTicket(); break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_NOTIFY: {
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->idFrom == IDC_TAB_MAIN && pnmh->code == TCN_SELCHANGE) {
            int sel = TabCtrl_GetCurSel(hTabControl);
            if (sel == 0) {
                // Вкладка 1: показываем
                ShowWindow(hListFlights, SW_SHOW);
                ShowWindow(hEditFlightNum, SW_SHOW);
                ShowWindow(hEditDest, SW_SHOW);
                ShowWindow(hEditDepart, SW_SHOW);
                ShowWindow(hEditTime, SW_SHOW);
                ShowWindow(hEditDuration, SW_SHOW);
                ShowWindow(hEditSeats, SW_SHOW);
                ShowWindow(hComboAircraft, SW_SHOW);
                ShowWindow(hBtnAddFlight, SW_SHOW);
                ShowWindow(hBtnDeleteFlight, SW_SHOW);
                ShowWindow(hBtnLoad, SW_SHOW);
                ShowWindow(hBtnSave, SW_SHOW);
                ShowWindow(hBtnClear, SW_SHOW);
                ShowWindow(hEditTicketFlight, SW_SHOW);
                ShowWindow(hEditPassenger, SW_SHOW);
                ShowWindow(hEditPrice, SW_SHOW);
                ShowWindow(hEditTicketCashier, SW_SHOW);
                ShowWindow(hBtnSellTicket, SW_SHOW);
                // Скрываем элементы вкладки 2
                ShowWindow(hListResults, SW_HIDE);
                ShowWindow(hEditQueryFlight, SW_HIDE);
                ShowWindow(hEditQueryDest, SW_HIDE);
                ShowWindow(hBtnCheckSeats, SW_HIDE);
                ShowWindow(hBtnFindDest, SW_HIDE);
                ShowWindow(hBtnMaxPrice, SW_HIDE);
                ShowWindow(hBtnNearest, SW_HIDE);
                ShowWindow(hBtnSaveQuery, SW_HIDE);
            }
            else {
                // Вкладка 2: показываем
                ShowWindow(hListResults, SW_SHOW);
                ShowWindow(hEditQueryFlight, SW_SHOW);
                ShowWindow(hEditQueryDest, SW_SHOW);
                ShowWindow(hBtnCheckSeats, SW_SHOW);
                ShowWindow(hBtnFindDest, SW_SHOW);
                ShowWindow(hBtnMaxPrice, SW_SHOW);
                ShowWindow(hBtnNearest, SW_SHOW);
                ShowWindow(hBtnSaveQuery, SW_SHOW);
                // Скрываем элементы вкладки 1
                ShowWindow(hListFlights, SW_HIDE);
                ShowWindow(hEditFlightNum, SW_HIDE);
                ShowWindow(hEditDest, SW_HIDE);
                ShowWindow(hEditDepart, SW_HIDE);
                ShowWindow(hEditTime, SW_HIDE);
                ShowWindow(hEditDuration, SW_HIDE);
                ShowWindow(hEditSeats, SW_HIDE);
                ShowWindow(hComboAircraft, SW_HIDE);
                ShowWindow(hBtnAddFlight, SW_HIDE);
                ShowWindow(hBtnDeleteFlight, SW_HIDE);
                ShowWindow(hBtnLoad, SW_HIDE);
                ShowWindow(hBtnSave, SW_HIDE);
                ShowWindow(hBtnClear, SW_HIDE);
                ShowWindow(hEditTicketFlight, SW_HIDE);
                ShowWindow(hEditPassenger, SW_HIDE);
                ShowWindow(hEditPrice, SW_HIDE);
                ShowWindow(hEditTicketCashier, SW_HIDE);
                ShowWindow(hBtnSellTicket, SW_HIDE);
            }
        }
        break;
    }
    case WM_DESTROY:
        SaveData();
        SaveTickets();
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
    case WM_INITDIALOG: return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}