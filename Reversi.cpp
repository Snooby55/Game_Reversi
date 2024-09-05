
#include <iostream>
#include <conio.h>
#include <Windows.h>

#define KEY_ARROW_RIGHT 77
#define KEY_ARROW_LEFT 75
#define KEY_ARROW_UP 72
#define KEY_ARROW_DOWN 80
#define KEY_ENTER 13
#define KEY_ESC 27

// Розміри дошки (За правилами гри може бути тільки парний NxN розмір дошки, найчастіше 8 на 8, але ця гра підтримує і непарну NxM розмірність)
#define AreaX 8
#define AreaY 8

using namespace std;

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Структура з двомірним масивом (наша дошка)
struct Board {
    char board[AreaX][AreaY];
};

// Гра
void DrawBoard(Board, COORD, int, int); // Малює дошку
void initBoard(Board&); // Задає координати перших 4 фішок та заповнює пробілами всі інші частини дошки
void selectMove(Board, COORD&, int, int, int&, int&); // Вибираємо координати ходу
int countSymbols(Board, char); // Рахує кількість фішок конкретного ігрока
bool isValidMove(Board, int, int, char); // Дивиться чи можливо зробити хід у конкретних координатах
bool isGameOver(Board); // Дивится чи не закінчилась гра
void updateBoard(Board&, int, int, char); // Оновлює дошку
void Game(COORD, int, int); // Реалізація гри

// Меню
void DrawMenu(COORD, int, int, int); // Малює меню
void DrawAbout(COORD, int, int); // Малює вікно About
int NumMenu(COORD, int, int, int); // Функція вибору з запропонованих варіантів у меню
void PrintText(COORD&, const char*, int); // При дуже великих текстах переносить його на новий рядок (задається максимальна довжина одного рядка)

int main() {

    system("title Reversi");

    // Розміри консолі
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // Налаштування курсору
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hStdOut, &cci);
    cci.bVisible = TRUE;
    cci.dwSize = 100;
    SetConsoleCursorInfo(hStdOut, &cci);

    // Положення курсору
    COORD cursorPos;
    cursorPos.X = 0;
    cursorPos.Y = 0;

    // Змінна для зчитування клавіш
    char ch;

    int enter = 0, theme = 0;

    while (enter != 4) {
        theme == 0 ? SetConsoleTextAttribute(hStdOut, 15 | BACKGROUND_BLUE | BACKGROUND_GREEN) : SetConsoleTextAttribute(hStdOut, 0 | 15 << 4);

        enter = NumMenu(cursorPos, width, height, theme);
        switch (enter) {
            case 1: Game(cursorPos, width, height); break;
            case 2: theme == 0 ? theme = 1 : theme = 0; break;
            case 3: DrawAbout(cursorPos, width, height); break;
        }
    }

}

void Game(COORD cursorPos, int width, int height) {

    Board board;
    initBoard(board);
 
    //Додатковий курсор для вибору ходу
    COORD cursorBoard;
    cursorBoard.X = width / 2 - AreaX + 1;
    cursorBoard.Y = 2;

    char player = 'X';
    int chance, skip = 0, col = 0, row = 0;
    bool validMove;

    DrawBoard(board, cursorPos, width, height);

    while (!isGameOver(board)) {
        validMove = false;

        cursorPos.X = 2; cursorPos.Y = AreaY + 3;
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << "                                           ";
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << player << "'s turn";

        if (skip != 0) {
            cursorPos.X = 4; cursorPos.Y = AreaY  + 5;
            SetConsoleCursorPosition(hStdOut, cursorPos);
            cout << "Player " << (player == 'X' ? 'O' : 'X') << " cannot walk, so he misses his turn";
            skip = 0;
        }

        while (!validMove) {

            chance = 0;
            for (int i = 0; i < AreaX; i++)
                for (int j = 0; j < AreaY; j++)
                    if (isValidMove(board, i, j, player))
                        chance++;

            if (chance != 0) {
                selectMove(board, cursorBoard, width, height, col, row);
            } else {
                skip = 1;
                validMove = true;
                continue;
            }

            if (isValidMove(board, col, row, player)) {
                validMove = true;
                cursorPos.X = 4; cursorPos.Y = AreaY + 5;
                SetConsoleCursorPosition(hStdOut, cursorPos);
                cout << "                                           ";
            } else {
                cursorPos.X = 4; cursorPos.Y = AreaY + 5;
                SetConsoleCursorPosition(hStdOut, cursorPos);
                cout << "                                           ";
                SetConsoleCursorPosition(hStdOut, cursorPos);
                cout << "Invalid move! Try again.";
            }
        }

        if (skip == 0)
            updateBoard(board, col, row, player);

        DrawBoard(board, cursorPos, width, height);
        player = (player == 'X' ? 'O' : 'X');

    }

    cursorPos.X = 2; cursorPos.Y = AreaY + 3;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "        ";

    int numX = countSymbols(board, 'X');
    int numO = countSymbols(board, 'O');

    SetConsoleCursorPosition(hStdOut, cursorPos);
    if (numX > numO)
        cout << "Player X wins!!";
    else if (numX < numO)
        cout << "Player O wins!!";
    else
        cout << "It's a draw!";

    cursorPos.X = 4; cursorPos.Y = AreaY + 5;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Player X has " << numX << " chips, and player O has " << numO << " chips.";

    cursorPos.Y = height - 2;
    cursorPos.X = width - 6;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Exit";

    cursorPos.X = width - 6;
    SetConsoleCursorPosition(hStdOut, cursorPos);

    char ch;
    while (true) {
        ch = _getch();
        if (ch == KEY_ESC || ch == KEY_ENTER)
            break;
    }

    return;

}

void DrawMenu(COORD cursorPos, int width, int height, int theme) {
    system("cls");

    cursorPos.X = width / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Start";

    cursorPos.X = (3 * width) / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Theme ";
    theme == 0 ? cout << "(blue)" : cout << "(white)";

    cursorPos.X = (5 * width) / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "About/Rule";

    cursorPos.X = (7 * width) / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Exit";

    cursorPos.X = width / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);
}

void DrawAbout(COORD cursorPos, int width, int height) {
    system("cls");

    int textLength = width - 40;
    char ch;
    const char* text1 = "In this version of the game, X is black, O is white. Black makes the first move. Then the players take turns.";
    const char* text2 = "When making a move, the player must place his checker on one of the cells of the board in such a way that between this placed checker and one of the checkers of his color already on the board there is a continuous row of opponent's checkers, horizontal, vertical or diagonal. All of the opponent's chips falling into the 'closed' row at the same time are changed from X to O and vice versa, and already belong to the player who performed this move.";
    const char* text3 = "If as a result of one move, more than one row of the opponent's chips is 'closed' at the same time, then all the chips that were in all 'closed' rows are turned over.";
    const char* text4 = "The player has the right to choose any of the moves possible for him. if a player has possible moves, he cannot refuse a move. If the player has no valid moves, the move goes to the opponent.";
    const char* text5 = "The game stops when all the chips are on the board or when none of the players can make a move. At the end of the game, the chips of each color (X and O) are counted, and the player with more chips on the board is declared the winner. In case of the same number of chips, a draw is counted.";

    cursorPos.X = 1; cursorPos.Y = 1;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "This game is made by Illia Parnenko";
    
    cursorPos.Y = 4;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "RULE:";
    cursorPos.Y += 2;
    PrintText(cursorPos, text1, textLength);
    cursorPos.Y += 2;
    PrintText(cursorPos, text2, textLength);
    cursorPos.Y += 2;
    PrintText(cursorPos, text3, textLength);
    cursorPos.Y += 2;
    PrintText(cursorPos, text4, textLength);
    cursorPos.Y += 2;
    PrintText(cursorPos, text5, textLength);

    // Загальні координати наших клавіш у консолі
    int coordX = width - 27, coordY = height / 2 + 3;

    // Малюємо клавіши стрілочок
    cursorPos.X = coordX + 5; cursorPos.Y = coordY - 3;
    for (int i = 0; i < 4; i++) {
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << char(201);
        for (int j = 0; j < 3; j++)
            cout << char(205);
        cout << char(187);
        cursorPos.Y++;
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << char(186);
        switch (i) {
            case 0: cout << " ^ "; break;
            case 1: cout << " < "; break;
            case 2: cout << " v "; break;
            case 3: cout << " > "; break;
        }
        cout << char(186);
        cursorPos.Y++;
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << char(200);
        for (int j = 0; j < 3; j++)
            cout << char(205);
        cout << char(188);
        cursorPos.X = coordX + i*5;
        cursorPos.Y = coordY;
    }

    // Малюємо клавішу ESC
    cursorPos.X = coordX - 5; cursorPos.Y = coordY - 6;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(201);
    for (int j = 0; j < 5; j++)
        cout << char(205);
    cout << char(187);
    cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(186) << " ESC " << char(186);
    cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(200);
    for (int j = 0; j < 5; j++)
        cout << char(205);
    cout << char(188);

    // Малюємо клавішу Enter
    cursorPos.X = coordX + 13; cursorPos.Y = coordY - 7;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(201);
    for (int j = 0; j < 6; j++)
        cout << char(205);
    cout << char(187);
    cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(186) << "      " << char(186);
    cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(200) << char(187) << "Enter" << char(186);
    cursorPos.X++; cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(186) << "     " << char(186);
    cursorPos.Y++;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(200);
    for (int j = 0; j < 5; j++)
        cout << char(205);
    cout << char(188);

    cursorPos.X = coordX + 1; cursorPos.Y = coordY - 10;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "CONTROL KEYS:";


    cursorPos.Y = height - 2;
    cursorPos.X = width - 6;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << "Exit";

    cursorPos.X = width - 6;
    SetConsoleCursorPosition(hStdOut, cursorPos);

    while (true) {
        ch = _getch();
        if (ch == KEY_ESC || ch == KEY_ENTER)
            break;
    }

    return;
}

void PrintText(COORD& cursorPos, const char* text, int length) {

    int textSize = strlen(text);
    int start = 0, end = length;

    SetConsoleCursorPosition(hStdOut, cursorPos);

    while (end < textSize) {
        // Знайдемо пробіл на кінці рядка (якщо він там є)
        int spacePos = -1;
        for (int i = end; i >= start; i--)
            if (text[i] == ' ') {
                spacePos = i;
                break;
            }

        // Якщо пробіл є, то розділимо текст по ньому
        if (spacePos != -1 && spacePos > start)
            end = spacePos;

        // Виведемо частину тексту у консоль
        for (int i = start; i <= end; i++)
            cout << text[i];

        cursorPos.Y++;
        SetConsoleCursorPosition(hStdOut, cursorPos); // Перенесемо рядок

        // Початок та кінець нового рядка
        start = end + 1;
        end = start + length;
    }

    // Виводимо залишок тексту
    for (int i = start; i <= textSize; i++)
        cout << text[i];

    return;
}

int NumMenu(COORD cursorPos, int width, int height, int theme) {

    char ch;
    int enter = 1;

    cursorPos.Y = height / 2 - 1;
    cursorPos.X = width / 8;
    SetConsoleCursorPosition(hStdOut, cursorPos);

    DrawMenu(cursorPos, width, height, theme);
    while (TRUE) {
        ch = _getch();
        switch (ch) {
        case KEY_ARROW_LEFT:
            if (cursorPos.X != width / 8) {
                cursorPos.X -= width / 4;
                enter--;
            }
            break;
        case KEY_ARROW_RIGHT:
            if (cursorPos.X != (7 * width) / 8) {
                cursorPos.X += width / 4;
                enter++;
            }
            break;
        case KEY_ENTER: return enter; DrawMenu(cursorPos, width, height, theme);
        case KEY_ESC: return 4;
        default: break;
        }
        SetConsoleCursorPosition(hStdOut, cursorPos);
    }
}

void DrawBoard(Board board, COORD cursorPos, int width, int height) {

    system("cls");

    // Намалюємо верхню рамку
    cursorPos.X = width / 2 - AreaX; cursorPos.Y = 1;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(201);  // "╔"
    for (int i = 0; i < AreaX * 2 - 1; i++) cout << char(205);  // "═"
    cout << char(187) << endl;  // "╗"

    // Намалюємо бічні рамки
    cursorPos.Y = 2;
    for (int j = 0; cursorPos.Y < AreaY + 2; cursorPos.Y++, j++) {
        cursorPos.X = width / 2 - AreaX;
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << char(186);  // "║"
        for (int i = 0; i < AreaX; i++) cout << board.board[i][j] << " ";
        cursorPos.X = width / 2 + AreaX;
        SetConsoleCursorPosition(hStdOut, cursorPos);
        cout << char(186) << endl;  // "║"
    }

    // Намалюємо нижню рамку
    cursorPos.X = width / 2 - AreaX;
    SetConsoleCursorPosition(hStdOut, cursorPos);
    cout << char(200);  // "╚"
    for (int i = 0; i < AreaX * 2 - 1; i++) cout << char(205);  // "═"
    cout << char(188) << endl;  // "╝"
}

void initBoard(Board& board) {
    for (int i = 0; i < AreaX; i++)
        for (int j = 0; j < AreaY; j++)
            board.board[i][j] = ' ';

    board.board[AreaX / 2 - 1][AreaY / 2 - 1] = 'O';
    board.board[AreaX / 2 - 1][AreaY / 2] = 'X';
    board.board[AreaX / 2][AreaY / 2 - 1] = 'X';
    board.board[AreaX / 2][AreaY / 2] = 'O';
}

void selectMove(Board board, COORD& cursorPos, int width, int height, int& col, int& row) {

    char ch;

    SetConsoleCursorPosition(hStdOut, cursorPos);

    while (TRUE) {
        ch = _getch();
        switch (ch) {
        case KEY_ARROW_LEFT:
            if (col > 0) {
                col--;
                cursorPos.X -= 2;
            }
            break;
        case KEY_ARROW_RIGHT:
            if (col < AreaX - 1) {
                col++;
                cursorPos.X += 2;
            }
            break;
        case KEY_ARROW_UP:
            if (row > 0) {
                row--;
                cursorPos.Y--;
            }
            break;
        case KEY_ARROW_DOWN:
            if (row < AreaY - 1) {
                row++;
                cursorPos.Y++;
            }
            break;
        case KEY_ENTER: return;
        default: break;
        }

        SetConsoleCursorPosition(hStdOut, cursorPos);
    }
}

int countSymbols(Board board, char symbol) {
    int numPieces = 0;

    for (int i = 0; i < AreaX; i++)
        for (int j = 0; j < AreaY; j++)
            if (board.board[i][j] == symbol)
                numPieces++;

    return numPieces;
}

bool isValidMove(Board board, int col, int row, char player) {

    char enemy = (player == 'X' ? 'O' : 'X');

    if (col < 0 || col >= AreaX || row < 0 || row >= AreaY)
        return false;

    if (board.board[col][row] != ' ')
        return false;

    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++) {
            if (col + i < 0 || col + i >= AreaX || row + j < 0 || row + j >= AreaY)
                continue;

            if (board.board[col + i][row + j] == enemy)
                for (int x = i * 2, y = j * 2, stop = 0, fl = 1; stop < AreaX && fl == 1; x += i, y += j, stop++) {

                    if (col + x < 0 || col + x >= AreaX || row + y < 0 || row + y >= AreaY) {
                        fl = 0;
                        continue;
                    }

                    if (board.board[col + x][row + y] == ' ') {
                        fl = 0;
                        continue;
                    }

                    if (board.board[col + x][row + y] == player && (x != 0 || y != 0) && (x * i != 1 || y * j != 1))
                        return true;

                }
        }

    return false;
}

void updateBoard(Board& board, int col, int row, char player) {

    board.board[col][row] = player;

    char enemy = (player == 'X' ? 'O' : 'X');

    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++) {
            if (col + i < 0 || col + i >= AreaX || row + j < 0 || row + j >= AreaY)
                continue;

            if (board.board[col + i][row + j] == enemy) {
                for (int x = i, y = j, stop = 0, fl = 1; stop < AreaX && fl == 1; x += i, y += j, stop++) {

                    if (col + x < 0 || col + x >= AreaX || row + y < 0 || row + y >= AreaY) {
                        fl = 0;
                        continue;
                    }

                    if (board.board[col + x][row + y] == ' ') {
                        fl = 0;
                        continue;
                    }

                    if (board.board[col + x][row + y] == player) {
                        for (int x_ = i, y_ = j, fl_ = 1; fl_ == 1; x_ += i, y_ += j) {

                            if (board.board[col + x_][row + y_] == player && (x_ != 0 || y_ != 0))
                                fl_ = 0;

                            if (board.board[col + x_][row + y_] == enemy)
                                board.board[col + x_][row + y_] = player;


                        }
                        fl = 0;
                    }

                }
            }
        }

}

bool isGameOver(Board board) {
    int numX = countSymbols(board, 'X');
    int numO = countSymbols(board, 'O');

    if (numX + numO == AreaX * AreaY) return true;

    for (int i = 0; i < AreaX; i++)
        for (int j = 0; j < AreaY; j++)
            if (isValidMove(board, i, j, 'X') || isValidMove(board, i, j, 'O'))
                return false;

    return true;
}
