// sudoku_game.cpp
// A 32-bit native Sudoku application using the raw Win32 API (C++).
// This requires linking with user32.lib and gdi32.lib.

#include <windows.h>
#include <tchar.h>
#include <string>
#include <stdlib.h> // For rand(), srand()

// --- Global Constants and Game State ---

// Window properties
const TCHAR CLASS_NAME[] = _T("SudokuWin32Class");
const TCHAR WINDOW_TITLE[] = _T("Classic Sudoku Puzzle (Win32)");
i6
// Board dimensions and drawing constants
const int CELL_SIZE = 40;       // Size of one cell in pixels
const int BOARD_SIZE = 9 * CELL_SIZE; // Total width/height of the grid
const int MARGIN = 20;          // Margin from the window edge
const int WINDOW_WIDTH = BOARD_SIZE + 2 * MARGIN + 10;
const int WINDOW_HEIGHT = BOARD_SIZE + 2 * MARGIN + 35; // Extra height for title bar

// Game state representation (0 means empty/user-editable)
int g_Board[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
};

// Initial state (used to check if a cell is pre-filled and should not be edited)
int g_InitialBoard[9][9];

// User interaction state
int g_SelectedRow = -1;
int g_SelectedCol = -1;

// --- Forward Declarations ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DrawGame(HWND hwnd, HDC hdc);
void InitializeGame();

// --- Initialization and Utility Functions ---

void InitializeGame()
{
    // Copy the initial state for validation
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            g_InitialBoard[r][c] = g_Board[r][c];
        }
    }
}

// Checks if a 'num' can be legally placed at g_Board[row][col]
bool IsSafe(int row, int col, int num) {
    // 1. Check Row
    for (int c = 0; c < 9; ++c) {
        if (g_Board[row][c] == num) {
            return false;
        }
    }

    // 2. Check Column
    for (int r = 0; r < 9; ++r) {
        if (g_Board[r][col] == num) {
            return false;
        }
    }

    // 3. Check 3x3 Box
    int boxStartRow = row - row % 3;
    int boxStartCol = col - col % 3;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (g_Board[boxStartRow + r][boxStartCol + c] == num) {
                return false;
            }
        }
    }

    return true;
}

// Checks if the entire board is solved
bool IsSolved() {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (g_Board[r][c] == 0) {
                return false; // Not complete
            }
            // Temporarily remove the current value to check its validity
            int temp = g_Board[r][c];
            g_Board[r][c] = 0;
            if (!IsSafe(r, c, temp)) {
                g_Board[r][c] = temp; // Restore value
                return false; // Invalid entry
            }
            g_Board[r][c] = temp; // Restore value
        }
    }
    return true; // Complete and valid
}

// --- Drawing Function (GDI) ---

void DrawGame(HWND hwnd, HDC hdc)
{
    // Use a large font for numbers
    HFONT hFont = CreateFont(CELL_SIZE / 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Tahoma"));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetBkMode(hdc, TRANSPARENT);

    // Draw the selection highlight first
    if (g_SelectedRow != -1 && g_SelectedCol != -1)
    {
        RECT selectedRect = {
            MARGIN + g_SelectedCol * CELL_SIZE,
            MARGIN + g_SelectedRow * CELL_SIZE,
            MARGIN + (g_SelectedCol + 1) * CELL_SIZE,
            MARGIN + (g_SelectedRow + 1) * CELL_SIZE
        };

        // Use a light blue color to highlight the selected cell
        HBRUSH hBrushHighlight = CreateSolidBrush(RGB(200, 220, 255));
        FillRect(hdc, &selectedRect, hBrushHighlight);
        DeleteObject(hBrushHighlight);
    }

    // Draw the grid lines
    HPEN hPenThin = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN hPenThick = CreatePen(PS_SOLID, 3, RGB(50, 50, 50));
    HPEN hOldPen;

    for (int i = 0; i <= 9; ++i)
    {
        // Determine line thickness: thin for cell lines, thick for 3x3 box boundaries
        hOldPen = (HPEN)SelectObject(hdc, (i % 3 == 0) ? hPenThick : hPenThin);

        // Draw vertical lines
        MoveToEx(hdc, MARGIN + i * CELL_SIZE, MARGIN, NULL);
        LineTo(hdc, MARGIN + i * CELL_SIZE, MARGIN + BOARD_SIZE);

        // Draw horizontal lines
        MoveToEx(hdc, MARGIN, MARGIN + i * CELL_SIZE, NULL);
        LineTo(hdc, MARGIN + BOARD_SIZE, MARGIN + i * CELL_SIZE);

        SelectObject(hdc, hOldPen);
    }

    // Draw the numbers
    for (int r = 0; r < 9; ++r)
    {
        for (int c = 0; c < 9; ++c)
        {
            int value = g_Board[r][c];
            if (value != 0)
            {
                TCHAR buffer[2];
                _stprintf_s(buffer, 2, _T("%d"), value);

                // Set color: Black for initial numbers, Blue for user-entered numbers
                if (g_InitialBoard[r][c] != 0) {
                    SetTextColor(hdc, RGB(0, 0, 0)); // Black for fixed numbers
                } else {
                    SetTextColor(hdc, RGB(0, 0, 200)); // Dark Blue for user input
                }

                // Calculate the cell rectangle for text centering
                RECT cellRect = {
                    MARGIN + c * CELL_SIZE,
                    MARGIN + r * CELL_SIZE,
                    MARGIN + (c + 1) * CELL_SIZE,
                    MARGIN + (r + 1) * CELL_SIZE
                };

                // Draw the text centered in the cell
                DrawText(hdc, buffer, -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }

    // Cleanup GDI resources
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    DeleteObject(hPenThin);
    DeleteObject(hPenThick);
}

// --- Window Procedure ---

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            InitializeGame();
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Draw all game elements
            DrawGame(hwnd, hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            // Calculate coordinates relative to the client area
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // Check if the click is within the board boundaries
            if (x >= MARGIN && x < MARGIN + BOARD_SIZE &&
                y >= MARGIN && y < MARGIN + BOARD_SIZE)
            {
                int newCol = (x - MARGIN) / CELL_SIZE;
                int newRow = (y - MARGIN) / CELL_SIZE;

                // Update selection state
                g_SelectedCol = newCol;
                g_SelectedRow = newRow;

                // Force a redraw of the entire client area
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }

        case WM_CHAR:
        {
            if (g_SelectedRow != -1 && g_SelectedCol != -1)
            {
                int r = g_SelectedRow;
                int c = g_SelectedCol;
                TCHAR ch = (TCHAR)wParam;

                // Only allow input in cells that were not pre-filled
                if (g_InitialBoard[r][c] == 0)
                {
                    // Check for a digit 1-9
                    if (ch >= _T('1') && ch <= _T('9'))
                    {
                        g_Board[r][c] = ch - _T('0');
                        InvalidateRect(hwnd, NULL, TRUE); // Redraw
                    }
                    // Handle Backspace/Delete to clear the cell (ASCII 8)
                    else if (ch == 8)
                    {
                        g_Board[r][c] = 0;
                        InvalidateRect(hwnd, NULL, TRUE); // Redraw
                    }
                }
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// --- Entry Point ---

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = {0};
    HWND hwnd;
    MSG Msg;

    // 1. Register the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW; // Redraw on resize
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONERROR | MB_OK);
        return 0;
    }

    // 2. Create the Window
    hwnd = CreateWindowEx(
        0,                                   // Optional window styles
        CLASS_NAME,                          // Window class
        WINDOW_TITLE,                        // Window title
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,    // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,        // Position
        WINDOW_WIDTH, WINDOW_HEIGHT,         // Size
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONERROR | MB_OK);
        return 0;
    }

    // 3. The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg); // Translates virtual-key messages into character messages (WM_CHAR)
        DispatchMessage(&Msg);  // Sends message to the WndProc
    }

    return (int)Msg.wParam;
}
