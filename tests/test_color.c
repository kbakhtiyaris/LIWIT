#include <ncurses.h>

int main() {
    initscr();
    
    // Check if terminal supports colors
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support colors\n");
        return 1;
    }
    
    start_color();
    
    // Initialize color pairs (foreground, background)
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLUE);
    
    // Use colors
    attron(COLOR_PAIR(1));
    printw("This is RED text\n");
    attroff(COLOR_PAIR(1));
    
    attron(COLOR_PAIR(2));
    printw("This is GREEN text\n");
    attroff(COLOR_PAIR(2));
    
    attron(COLOR_PAIR(5));
    printw("This is WHITE on BLUE\n");
    attroff(COLOR_PAIR(5));
    
    refresh();
    getch();
    int ch;
    int y = 0, x = 0;
    
    initscr();              // Initialize
    raw();                  // Disable line buffering
    keypad(stdscr, TRUE);   // Enable function keys
    noecho();              // Don't echo input
    attron(COLOR_PAIR(4));
    printw("Press arrow keys to move, 'q' to quit\n");
    attroff(COLOR_PAIR(4));
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
       attron(COLOR_PAIR(1));
        mvprintw(y, x, "X");  // Draw at position
	attroff(COLOR_PAIR(1));
        mvprintw(0, 0, "Position: (%d, %d)", y, x);
	
        refresh();
    }
    endwin();
    
    return 0;
}
