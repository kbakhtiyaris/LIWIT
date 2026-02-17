#include <ncurses.h>

int main() {
    int ch;
    int y = 0, x = 0;
    
    initscr();              // Initialize
    raw();                  // Disable line buffering
    keypad(stdscr, TRUE);   // Enable function keys
    noecho();              // Don't echo input
    
    printw("Press arrow keys to move, 'q' to quit\n");
    refresh();
    
    // Main loop
    while((ch = getch()) != 'q') {
        switch(ch) {
            case KEY_UP:
                y--;
                break;
            case KEY_DOWN:
                y++;
                break;
            case KEY_LEFT:
                x--;
                break;
            case KEY_RIGHT:
                x++;
                break;
        }
        
        // Clear and redraw
        clear();
        mvprintw(y, x, "X");  // Draw at position
        mvprintw(0, 0, "Position: (%d, %d)", y, x);
        refresh();
    }
    
    endwin();
    return 0;
}
