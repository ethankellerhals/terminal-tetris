#include <ncurses.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

const int width = 10;
const int height = 20;

// Shapes
const std::vector<std::vector<std::vector<int>>> shapes = {
    {{1, 1, 1, 1}},       // I
    {{1, 1}, {1, 1}},     // O
    {{0, 1, 0}, {1, 1, 1}}, // T
    {{1, 0, 0}, {1, 1, 1}}, // L
    {{0, 0, 1}, {1, 1, 1}}, // J
    {{1, 1, 0}, {0, 1, 1}}, // S
    {{0, 1, 1}, {1, 1, 0}}  // Z
};

std::vector<std::vector<int>> board(height, std::vector<int>(width, 0));
std::vector<std::vector<int>> currentShape;
std::vector<std::vector<int>> nextShape;
int shapeX, shapeY;
int currentColor;
int nextColor;

int offsetX, offsetY;

void initColors() {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_MAGENTA, -1);
    init_pair(4, COLOR_BLUE, -1);
    init_pair(5, COLOR_GREEN, -1);
    init_pair(6, COLOR_RED, -1);
    init_pair(7, COLOR_WHITE, -1);
}

void drawBoard() {
    clear();

    for (int y = 0; y <= height; ++y) {
        mvprintw(offsetY + y, offsetX - 2, "|");
        mvprintw(offsetY + y, offsetX + width * 2, "|");
    }

    for (int x = 0; x <= width * 2; x += 2) {
        mvprintw(offsetY - 1, offsetX + x, "-");
        mvprintw(offsetY + height, offsetX + x, "-");
    }

    mvprintw(offsetY - 1, offsetX - 2, "+");
    mvprintw(offsetY - 1, offsetX + width * 2, "+");
    mvprintw(offsetY + height, offsetX - 2, "+");
    mvprintw(offsetY + height, offsetX + width * 2, "+");

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (board[y][x]) {
                attron(COLOR_PAIR(board[y][x]));
                mvprintw(offsetY + y, offsetX + x * 2, "[]");
                attroff(COLOR_PAIR(board[y][x]));
            } else {
                mvprintw(offsetY + y, offsetX + x * 2, "  ");
            }
        }
    }

    for (int y = 0; y < currentShape.size(); ++y) {
        for (int x = 0; x < currentShape[y].size(); ++x) {
            if (currentShape[y][x]) {
                attron(COLOR_PAIR(currentColor));
                mvprintw(offsetY + shapeY + y, offsetX + (shapeX + x) * 2, "[]");
                attroff(COLOR_PAIR(currentColor));
            }
        }
    }

    mvprintw(offsetY, offsetX + width * 2 + 4, "Next:");
    for (int y = 0; y < nextShape.size(); ++y) {
        for (int x = 0; x < nextShape[y].size(); ++x) {
            if (nextShape[y][x]) {
                attron(COLOR_PAIR(nextColor));
                mvprintw(offsetY + y + 2, offsetX + width * 2 + 4 + x * 2, "[]");
                attroff(COLOR_PAIR(nextColor));
            }
        }
    }

    refresh();
}

bool checkCollision(int newShapeX, int newShapeY, std::vector<std::vector<int>> newShape) {
    for (int y = 0; y < newShape.size(); ++y) {
        for (int x = 0; x < newShape[y].size(); ++x) {
            if (newShape[y][x] &&
                (newShapeX + x < 0 || newShapeX + x >= width || newShapeY + y >= height || board[newShapeY + y][newShapeX + x])) {
                return true;
            }
        }
    }
    return false;
}

void mergeShape() {
    for (int y = 0; y < currentShape.size(); ++y) {
        for (int x = 0; x < currentShape[y].size(); ++x) {
            if (currentShape[y][x]) {
                board[shapeY + y][shapeX + x] = currentColor;
            }
        }
    }
}

void rotateShape() {
    std::vector<std::vector<int>> newShape(currentShape[0].size(), std::vector<int>(currentShape.size()));
    for (int y = 0; y < currentShape.size(); ++y) {
        for (int x = 0; x < currentShape[y].size(); ++x) {
            newShape[x][currentShape.size() - 1 - y] = currentShape[y][x];
        }
    }

    if (!checkCollision(shapeX, shapeY, newShape)) {
        currentShape = newShape;
    }
}

void newShape() {
    currentShape = nextShape;
    shapeX = width / 2 - currentShape[0].size() / 2;
    shapeY = 0;
    currentColor = nextColor;

    nextShape = shapes[rand() % shapes.size()];
    nextColor = rand() % 7 + 1;

    if (checkCollision(shapeX, shapeY, currentShape)) {
        endwin();
        std::cout << "Game Over!" << std::endl;
        exit(0);
    }
}

void removeFullLines() {
    for (int y = height - 1; y >= 0; --y) {
        bool fullLine = true;
        for (int x = 0; x < width; ++x) {
            if (!board[y][x]) {
                fullLine = false;
                break;
            }
        }

        if (fullLine) {
            for (int ny = y; ny > 0; --ny) {
                board[ny] = board[ny - 1];
            }
            board[0] = std::vector<int>(width, 0);
            y++;
        }
    }
}

int main() {
    srand(time(0));
    initscr();
    noecho();
    curs_set(0);
    timeout(100);
    keypad(stdscr, TRUE);

    int termWidth, termHeight;
    getmaxyx(stdscr, termHeight, termWidth);
    offsetX = (termWidth - width * 2) / 2;
    offsetY = (termHeight - height) / 2;

    initColors();
    nextShape = shapes[rand() % shapes.size()];
    nextColor = rand() % 7 + 1;
    newShape();

    while (true) {
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (!checkCollision(shapeX - 1, shapeY, currentShape)) shapeX--;
                break;
            case KEY_RIGHT:
                if (!checkCollision(shapeX + 1, shapeY, currentShape)) shapeX++;
                break;
            case KEY_DOWN:
                if (!checkCollision(shapeX, shapeY + 1, currentShape)) shapeY++;
                else {
                    mergeShape();
                    removeFullLines();
                    newShape();
                }
                break;
            case ' ':
                rotateShape();
                break;
        }

        if (!checkCollision(shapeX, shapeY + 1, currentShape)) {
            shapeY++;
        } else {
            mergeShape();
            removeFullLines();
            newShape();
        }

        drawBoard();
    }

    endwin();
    return 0;
}
