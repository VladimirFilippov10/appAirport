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

// Идентификаторы контролов
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
#define IDC_COMBO_AIRCRAFT     1026

// Глобальные переменные
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND g_hWnd;
int g_yScrollPos = 0;
int g_yScrollMax = 0;

// Данные приложения
std::vector<Aircraft> aircrafts;
std::vector<Flight> flights;
std::vector<Ticket> tickets;
std::vector<std::string> lastQueryResults;

// Файлы
const std::string AIRCRAFTS_FILE = "aircrafts.txt";
const std::string FLIGHTS_FILE = "flights.txt";
const std::string TICKETS_FILE = "tickets.txt";

// Хэндлы контролов
HWND hListFlights, hListResults;
HWND hEditFlightNum, hEditDest, hEditDepart, hEditTime, hEditDuration, hEditSeats;
HWND hEditQueryFlight, hEditQueryDest;
HWND hEditTicketFlight, hEditPassenger, hEditPrice, hEditTicketCashier;
HWND hComboAircraft;

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
void ShowTicketsForSelectedFlight();
void DebugShowLoadedFlights();
std::time_t ParseDateTime(const std::string& str);
std::string WCharToString(const wchar_t* wstr);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);
double SafeStod(const std::string& str, double defaultValue = 0.0);
int SafeStoi(const std::string& str, int defaultValue = 0);
std::string TrimString(const std::string& str);
void UpdateScrollRange(HWND hWnd);
void ScrollWindowTo(HWND hWnd, int delta);

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

// Регистрация класса окна
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

// Создание и инициализация главного окна
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    g_hWnd = CreateWindowW(szWindowClass, L"Аэропорт - Система управления рейсами",
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,
        CW_USEDEFAULT, 0, 1400, 850, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) return FALSE;
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    return TRUE;
}

// Обновление диапазона вертикальной прокрутки
void UpdateScrollRange(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    g_yScrollMax = 950;
    SCROLLINFO si = { sizeof(SCROLLINFO) };
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = g_yScrollMax;
    si.nPage = rc.bottom;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
}

// Прокрутка окна на заданное смещение
void ScrollWindowTo(HWND hWnd, int delta) {
    int newPos = g_yScrollPos + delta;
    RECT rc;
    GetClientRect(hWnd, &rc);
    int maxPos = g_yScrollMax - rc.bottom;
    if (maxPos < 0) maxPos = 0;
    if (newPos < 0) newPos = 0;
    if (newPos > maxPos) newPos = maxPos;
    if (newPos != g_yScrollPos) {
        ScrollWindow(hWnd, 0, g_yScrollPos - newPos, NULL, NULL);
        g_yScrollPos = newPos;
        SetScrollPos(hWnd, SB_VERT, g_yScrollPos, TRUE);
        UpdateWindow(hWnd);
    }
}

// Удаление пробелов в начале и конце строки
std::string TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Загрузка списка самолётов из файла
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

// Обновление выпадающего списка самолётов
void UpdateAircraftCombo() {
    SendMessage(hComboAircraft, CB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < aircrafts.size(); i++) {
        std::string display = aircrafts[i].getName() + " (" + aircrafts[i].getCategory() + ", " + std::to_string(aircrafts[i].getSeats()) + " мест)";
        std::wstring wdisplay = StringToWString(display);
        SendMessage(hComboAircraft, CB_ADDSTRING, 0, (LPARAM)wdisplay.c_str());
    }
    SendMessage(hComboAircraft, CB_SETCURSEL, 0, 0);
}

// Преобразование wchar_t* в строку UTF-8
std::string WCharToString(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 1) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, NULL, NULL);
    return result;
}

// Преобразование строки UTF-8 в wstring
std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    if (size <= 0) return L"";
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], size);
    return result;
}

// Преобразование wstring в строку UTF-8
std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 1) return "";
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

// Создание всех элементов интерфейса
void CreateControls(HWND hWnd) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // Таблица рейсов
    hListFlights = CreateWindowW(WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 10, 1000, 320, hWnd, (HMENU)IDC_LIST_FLIGHTS, hInst, NULL);

    LVCOLUMNW lvc = { LVCF_TEXT | LVCF_WIDTH };
    const wchar_t* cols[] = { L"Номер рейса", L"Самолёт", L"Назначение", L"Отправление", L"Время вылета", L"Длит.", L"Мест" };
    int widths[] = { 120, 220, 160, 160, 180, 70, 70 };
    for (int i = 0; i < 7; i++) {
        lvc.cx = widths[i];
        lvc.pszText = const_cast<wchar_t*>(cols[i]);
        ListView_InsertColumn(hListFlights, i, &lvc);
    }

    // Панель добавления рейса
    CreateWindowW(L"STATIC", L"ДОБАВЛЕНИЕ РЕЙСА", WS_CHILD | WS_VISIBLE | SS_CENTER,
        1030, 10, 420, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 1030, 50, 100, 30, hWnd, NULL, hInst, NULL);
    hEditFlightNum = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 50, 280, 30, hWnd, (HMENU)IDC_EDIT_FLIGHT_NUM, hInst, NULL);

    CreateWindowW(L"STATIC", L"Назначение (куда):", WS_CHILD | WS_VISIBLE, 1030, 90, 130, 30, hWnd, NULL, hInst, NULL);
    hEditDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1170, 90, 250, 30, hWnd, (HMENU)IDC_EDIT_DEST, hInst, NULL);

    CreateWindowW(L"STATIC", L"Отправление (откуда):", WS_CHILD | WS_VISIBLE, 1030, 130, 140, 30, hWnd, NULL, hInst, NULL);
    hEditDepart = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1180, 130, 240, 30, hWnd, (HMENU)IDC_EDIT_DEPART, hInst, NULL);

    CreateWindowW(L"STATIC", L"Время (ГГГГ-ММ-ДД ЧЧ:ММ):", WS_CHILD | WS_VISIBLE, 1030, 170, 200, 30, hWnd, NULL, hInst, NULL);
    hEditTime = CreateWindowW(L"EDIT", L"2025-05-10 08:00", WS_CHILD | WS_VISIBLE | WS_BORDER, 1240, 170, 180, 30, hWnd, (HMENU)IDC_EDIT_TIME, hInst, NULL);

    CreateWindowW(L"STATIC", L"Длительность (мин):", WS_CHILD | WS_VISIBLE, 1030, 210, 130, 30, hWnd, NULL, hInst, NULL);
    hEditDuration = CreateWindowW(L"EDIT", L"120", WS_CHILD | WS_VISIBLE | WS_BORDER, 1170, 210, 120, 30, hWnd, (HMENU)IDC_EDIT_DURATION, hInst, NULL);

    CreateWindowW(L"STATIC", L"Кол-во мест:", WS_CHILD | WS_VISIBLE, 1030, 250, 100, 30, hWnd, NULL, hInst, NULL);
    hEditSeats = CreateWindowW(L"EDIT", L"50", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 250, 120, 30, hWnd, (HMENU)IDC_EDIT_SEATS, hInst, NULL);

    CreateWindowW(L"STATIC", L"Самолёт:", WS_CHILD | WS_VISIBLE, 1030, 290, 100, 30, hWnd, NULL, hInst, NULL);
    hComboAircraft = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        1140, 290, 350, 150, hWnd, (HMENU)IDC_COMBO_AIRCRAFT, hInst, NULL);
    SendMessage(hComboAircraft, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    CreateWindowW(L"BUTTON", L"+ ДОБАВИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1030, 335, 180, 40, hWnd, (HMENU)IDC_BTN_ADD_FLIGHT, hInst, NULL);
    CreateWindowW(L"BUTTON", L"- УДАЛИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1225, 335, 180, 40, hWnd, (HMENU)IDC_BTN_DEL_FLIGHT, hInst, NULL);

    // Панель управления
    CreateWindowW(L"BUTTON", L"ЗАГРУЗИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 345, 160, 40, hWnd, (HMENU)IDC_BTN_LOAD, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        180, 345, 160, 40, hWnd, (HMENU)IDC_BTN_SAVE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ОЧИСТИТЬ ВСЁ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        350, 345, 160, 40, hWnd, (HMENU)IDC_BTN_CLEAR, hInst, NULL);

    // Запросы
    CreateWindowW(L"STATIC", L"ЗАПРОСЫ", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 400, 700, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"Свободные места", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 440, 150, 35, hWnd, (HMENU)IDC_BTN_CHECK_SEATS, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 175, 445, 90, 30, hWnd, NULL, hInst, NULL);
    hEditQueryFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 275, 445, 160, 30, hWnd, (HMENU)IDC_EDIT_QUERY_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"По направлению", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 485, 150, 35, hWnd, (HMENU)IDC_BTN_FIND_DEST, hInst, NULL);
    CreateWindowW(L"STATIC", L"Город (куда):", WS_CHILD | WS_VISIBLE, 175, 490, 100, 30, hWnd, NULL, hInst, NULL);
    hEditQueryDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 285, 490, 150, 30, hWnd, (HMENU)IDC_EDIT_QUERY_DEST, hInst, NULL);

    CreateWindowW(L"BUTTON", L"Макс.цена билета", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 530, 150, 35, hWnd, (HMENU)IDC_BTN_MAX_PRICE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"Ближайший рейс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 575, 150, 35, hWnd, (HMENU)IDC_BTN_NEAREST, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ РЕЗУЛЬТАТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 625, 200, 40, hWnd, (HMENU)IDC_BTN_SAVE_QUERY, hInst, NULL);

    // Таблица результатов
    CreateWindowW(L"STATIC", L"БИЛЕТЫ НА РЕЙС / РЕЗУЛЬТАТЫ ЗАПРОСОВ", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 680, 700, 30, hWnd, NULL, hInst, NULL);
    hListResults = CreateWindowW(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT, 10, 715, 1000, 150, hWnd, (HMENU)IDC_LIST_RESULTS, hInst, NULL);
    lvc.cx = 980;
    lvc.pszText = const_cast<wchar_t*>(L"Информация");
    ListView_InsertColumn(hListResults, 0, &lvc);

    // Продажа билетов
    CreateWindowW(L"STATIC", L"ПРОДАЖА БИЛЕТОВ", WS_CHILD | WS_VISIBLE | SS_CENTER, 1030, 400, 420, 30, hWnd, NULL, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 1030, 445, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 445, 280, 30, hWnd, (HMENU)IDC_EDIT_TICKET_FLIGHT, hInst, NULL);
    CreateWindowW(L"STATIC", L"Пассажир (ФИО):", WS_CHILD | WS_VISIBLE, 1030, 485, 110, 30, hWnd, NULL, hInst, NULL);
    hEditPassenger = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1150, 485, 310, 30, hWnd, (HMENU)IDC_EDIT_PASSENGER, hInst, NULL);
    CreateWindowW(L"STATIC", L"Цена (руб):", WS_CHILD | WS_VISIBLE, 1030, 525, 100, 30, hWnd, NULL, hInst, NULL);
    hEditPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 525, 200, 30, hWnd, (HMENU)IDC_EDIT_PRICE, hInst, NULL);
    CreateWindowW(L"STATIC", L"Касса №:", WS_CHILD | WS_VISIBLE, 1030, 565, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketCashier = CreateWindowW(L"EDIT", L"001", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 565, 120, 30, hWnd, (HMENU)IDC_EDIT_TICKET_CASHIER, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ПРОДАТЬ БИЛЕТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1140, 610, 240, 45, hWnd, (HMENU)IDC_BTN_SELL_TICKET, hInst, NULL);

    LoadAircrafts();
    UpdateAircraftCombo();
    LoadData();
    LoadTickets();

    UpdateScrollRange(hWnd);
}

// Обновление таблицы рейсов
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

// Обновление таблицы результатов
void RefreshResultsListW(const std::vector<std::wstring>& results) {
    ListView_DeleteAllItems(hListResults);
    for (size_t i = 0; i < results.size(); i++) {
        LVITEMW lvi = { LVIF_TEXT, (int)i, 0 };
        lvi.pszText = const_cast<LPWSTR>(results[i].c_str());
        ListView_InsertItem(hListResults, &lvi);
    }
}

// Отображение билетов для выбранного рейса
void ShowTicketsForSelectedFlight() {
    int sel = ListView_GetSelectionMark(hListFlights);
    if (sel < 0) {
        std::vector<std::wstring> results;
        results.push_back(L"Выберите рейс из таблицы выше");
        RefreshResultsListW(results);
        return;
    }

    wchar_t buffer[256];
    ListView_GetItemText(hListFlights, sel, 0, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));

    std::vector<std::wstring> results;
    results.push_back(L"=== БИЛЕТЫ НА РЕЙС " + StringToWString(flightNum) + L" ===");

    int ticketCount = 0;
    for (const auto& t : tickets) {
        if (t.getFlightNumber() == flightNum) {
            results.push_back(L"Пассажир: " + StringToWString(t.getPassengerName()) +
                L" | Цена: " + std::to_wstring(t.getPrice()) + L" руб. | Касса: " + StringToWString(t.getCashierNumber()));
            ticketCount++;
        }
    }

    if (ticketCount == 0) {
        results.push_back(L"Нет проданных билетов на этот рейс");
    }
    else {
        results.push_back(L"Всего билетов: " + std::to_wstring(ticketCount));
    }

    RefreshResultsListW(results);
}

// Загрузка рейсов из файла
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
                flight.setNumber(TrimString(parts[0]));

                for (auto& ac : aircrafts) {
                    if (ac.getName() == TrimString(parts[1])) {
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

// Сохранение рейсов в файл
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

// Загрузка билетов из файла
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

// Сохранение билетов в файл
void SaveTickets() {
    std::ofstream file(TICKETS_FILE);
    for (const auto& t : tickets) {
        file << t.getCashierNumber() << ";" << t.getFlightNumber() << ";"
            << t.getPassengerName() << ";" << t.getPrice() << std::endl;
    }
    file.close();
}

// Добавление нового рейса
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

// Удаление выбранного рейса
void DeleteFlight() {
    int sel = ListView_GetSelectionMark(hListFlights);
    if (sel < 0) {
        MessageBoxW(g_hWnd, L"Выберите рейс для удаления!", L"Ошибка", MB_OK);
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

// Функция продажи билета
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

    if (flightNum.empty()) {
        MessageBoxW(g_hWnd, L"Введите номер рейса!", L"Ошибка", MB_OK);
        return;
    }
    if (passenger.empty()) {
        MessageBoxW(g_hWnd, L"Введите ФИО пассажира!", L"Ошибка", MB_OK);
        return;
    }
    if (price <= 0) {
        MessageBoxW(g_hWnd, L"Введите корректную цену!", L"Ошибка", MB_OK);
        return;
    }
    if (cashier.empty()) {
        cashier = "001";
    }

    Flight* found = nullptr;
    for (auto& f : flights) {
        if (TrimString(f.getNumber()) == flightNum) {
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

// ЗАПРОС 1: свободные места на рейс
void QueryFreeSeats() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    lastQueryResults.clear();

    if (flightNum.empty()) {
        results.push_back(L"Введите номер рейса!");
        lastQueryResults.push_back("Введите номер рейса!");
        RefreshResultsListW(results);

        std::ofstream file("query_result.txt");
        if (file.is_open()) {
            file << "Введите номер рейса!" << std::endl;
            file.close();
        }
        return;
    }

    for (const auto& f : flights) {
        if (TrimString(f.getNumber()) == flightNum) {
            std::string msg = "Рейс " + flightNum + " - свободных мест: " + std::to_string(f.getFreeSeats());
            results.push_back(StringToWString(msg));
            lastQueryResults.push_back(msg);
            RefreshResultsListW(results);

            std::ofstream file("query_result.txt");
            if (file.is_open()) {
                file << "=== СВОБОДНЫЕ МЕСТА НА РЕЙС " << flightNum << " ===" << std::endl;
                file << msg << std::endl;
                file.close();
            }
            return;
        }
    }

    std::string msg = "Рейс " + flightNum + " не найден";
    results.push_back(StringToWString(msg));
    lastQueryResults.push_back(msg);
    RefreshResultsListW(results);

    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        file << msg << std::endl;
        file.close();
    }
}

// ЗАПРОС 2: поиск рейсов по направлению
void QueryFlightsByDestination() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    lastQueryResults.clear();

    if (dest.empty()) {
        results.push_back(L"Введите город назначения!");
        lastQueryResults.push_back("Введите город назначения!");
        RefreshResultsListW(results);

        std::ofstream file("query_result.txt");
        if (file.is_open()) {
            file << "Введите город назначения!" << std::endl;
            file.close();
        }
        return;
    }

    for (const auto& f : flights) {
        if (TrimString(f.getDestination()) == dest) {
            std::string msg = "Рейс " + f.getNumber() + " - свободных мест: " + std::to_string(f.getFreeSeats());
            results.push_back(StringToWString(msg));
            lastQueryResults.push_back(msg);
        }
    }

    if (results.empty()) {
        std::string msg = "Нет рейсов в " + dest;
        results.push_back(StringToWString(msg));
        lastQueryResults.push_back(msg);
    }

    RefreshResultsListW(results);

    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        file << "=== РЕЙСЫ ПО НАПРАВЛЕНИЮ " << dest << " ===" << std::endl;
        for (const auto& line : lastQueryResults) {
            file << line << std::endl;
        }
        file.close();
    }
}

// ЗАПРОС 3: максимальная цена билета на рейс
void QueryMaxTicketPrice() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    lastQueryResults.clear();
    double maxPrice = -1;
    std::string maxPricePassenger;

    if (flightNum.empty()) {
        results.push_back(L"Введите номер рейса!");
        lastQueryResults.push_back("Введите номер рейса!");
        RefreshResultsListW(results);

        std::ofstream file("query_result.txt");
        if (file.is_open()) {
            file << "Введите номер рейса!" << std::endl;
            file.close();
        }
        return;
    }

    for (const auto& t : tickets) {
        if (TrimString(t.getFlightNumber()) == flightNum && t.getPrice() > maxPrice) {
            maxPrice = t.getPrice();
            maxPricePassenger = t.getPassengerName();
        }
    }

    if (maxPrice >= 0) {
        std::string msg1 = "Рейс " + flightNum + " - максимальная цена: " + std::to_string(maxPrice) + " руб.";
        std::string msg2 = "Пассажир: " + maxPricePassenger;
        results.push_back(StringToWString(msg1));
        results.push_back(StringToWString(msg2));
        lastQueryResults.push_back(msg1);
        lastQueryResults.push_back(msg2);
    }
    else {
        std::string msg = "Нет проданных билетов на рейс " + flightNum;
        results.push_back(StringToWString(msg));
        lastQueryResults.push_back(msg);
    }

    RefreshResultsListW(results);

    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        file << "=== МАКСИМАЛЬНАЯ ЦЕНА БИЛЕТА НА РЕЙС " << flightNum << " ===" << std::endl;
        for (const auto& line : lastQueryResults) {
            file << line << std::endl;
        }
        file.close();
    }
}

// ЗАПРОС 4: ближайший рейс по направлению
void QueryNearestFlight() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    lastQueryResults.clear();
    std::time_t now = std::time(nullptr);
    const Flight* nearest = nullptr;
    time_t minDiff = -1;

    if (dest.empty()) {
        results.push_back(L"Введите город!");
        lastQueryResults.push_back("Введите город!");
        RefreshResultsListW(results);

        std::ofstream file("query_result.txt");
        if (file.is_open()) {
            file << "Введите город!" << std::endl;
            file.close();
        }
        return;
    }

    for (const auto& f : flights) {
        if (TrimString(f.getDestination()) == dest && f.getFreeSeats() > 0) {
            time_t diff = std::difftime(f.getDepartureTime(), now);
            if (diff > 0 && (nearest == nullptr || diff < minDiff)) {
                nearest = &f;
                minDiff = diff;
            }
        }
    }

    if (nearest == nullptr) {
        for (const auto& f : flights) {
            if (TrimString(f.getDestination()) == dest && f.getFreeSeats() > 0) {
                time_t diff = std::difftime(f.getDepartureTime(), now);
                if (nearest == nullptr || diff < minDiff) {
                    nearest = &f;
                    minDiff = diff;
                }
            }
        }
    }

    if (nearest) {
        std::string msg1, msg2, msg3;
        if (minDiff > 0) {
            msg1 = "Ближайший рейс в " + dest + ": " + nearest->getNumber() + " через " + std::to_string(minDiff / 60) + " мин";
        }
        else {
            msg1 = "Ближайший рейс в " + dest + ": " + nearest->getNumber();
        }
        msg2 = "Свободных мест: " + std::to_string(nearest->getFreeSeats());
        msg3 = "Время вылета: " + nearest->getDepartureTimeStr();

        results.push_back(StringToWString(msg1));
        results.push_back(StringToWString(msg2));
        results.push_back(StringToWString(msg3));
        lastQueryResults.push_back(msg1);
        lastQueryResults.push_back(msg2);
        lastQueryResults.push_back(msg3);
    }
    else {
        std::string msg = "Нет рейсов в " + dest;
        results.push_back(StringToWString(msg));
        lastQueryResults.push_back(msg);
    }

    RefreshResultsListW(results);

    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        file << "=== БЛИЖАЙШИЙ РЕЙС В " << dest << " ===" << std::endl;
        for (const auto& line : lastQueryResults) {
            file << line << std::endl;
        }
        file.close();
    }
}

// Сохранение результата запроса в файл (ручное)
void SaveQueryResult() {
    if (lastQueryResults.empty()) {
        MessageBoxW(g_hWnd, L"Нет результатов для сохранения! Сначала выполните запрос.", L"Ошибка", MB_OK);
        return;
    }

    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        file << "=== РЕЗУЛЬТАТ ЗАПРОСА ===" << std::endl;
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

// Парсинг строки даты и времени
std::time_t ParseDateTime(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    if (ss.fail()) {
        return (std::time_t)0;
    }
    return std::mktime(&tm);
}

// Безопасное преобразование строки в double
double SafeStod(const std::string& str, double defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    double result = strtod(str.c_str(), &endPtr);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return result;
}

// Безопасное преобразование строки в int
int SafeStoi(const std::string& str, int defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    long result = strtol(str.c_str(), &endPtr, 10);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return (int)result;
}

// Главная оконная процедура
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        break;
    case WM_NOTIFY: {
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->idFrom == IDC_LIST_FLIGHTS && pnmh->code == LVN_ITEMCHANGED) {
            ShowTicketsForSelectedFlight();
        }
        break;
    }
    case WM_SIZE:
        UpdateScrollRange(hWnd);
        break;
    case WM_VSCROLL: {
        int delta = 0;
        switch (LOWORD(wParam)) {
        case SB_LINEUP:      delta = -30; break;
        case SB_LINEDOWN:    delta = 30; break;
        case SB_PAGEUP:      delta = -200; break;
        case SB_PAGEDOWN:    delta = 200; break;
        case SB_THUMBTRACK: {
            SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
            GetScrollInfo(hWnd, SB_VERT, &si);
            delta = si.nTrackPos - g_yScrollPos;
            break;
        }
        default: break;
        }
        if (delta != 0) {
            ScrollWindowTo(hWnd, delta);
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        ScrollWindowTo(hWnd, -delta / 2);
        break;
    }
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

// Диалоговое окно "О программе"
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