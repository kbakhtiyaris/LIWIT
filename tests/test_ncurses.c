#include <ncurses.h>

int main() {
    // Initialize ncurses
    initscr();              // Start curses mode
    
    // Print hello world
    printw("Hello, LIWIT!");
    
    // Refresh screen to show changes
    refresh();
    
    // Wait for user input
    getch();
    
    // End ncurses mode
    endwin();
    
    return 0;
}
