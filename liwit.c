/**
 * LIWIT - Linux-Windows Text Editor
 * Version 1.0
 * Author: Khud Bakhtiyar Iqbal Sofi
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

// Configuration
#define VERSION "1.0"
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

// Editor state structure
typedef struct {
    char **lines;           // Array of text lines
    int line_count;        // Total lines
    int cursor_x;          // Cursor column
    int cursor_y;          // Cursor row
    int screen_rows;       // Terminal height
    int screen_cols;       // Terminal width
    char *filename;        // Current file
    int modified;          // Has changes?
} EditorState;

// Function prototypes
void init_editor(EditorState *ed);
void cleanup_editor(EditorState *ed);
void draw_screen(EditorState *ed);
void handle_input(EditorState *ed);

int main(int argc, char *argv[]) {
    EditorState editor;
    
    // Initialize ncurses
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    
    // Initialize editor
    init_editor(&editor);
    
    // Main loop
    while (1) {
        draw_screen(&editor);
        handle_input(&editor);
    }
    
    // Cleanup
    cleanup_editor(&editor);
    endwin();
    
    return 0;
}

void init_editor(EditorState *ed) {
    // Allocate memory for lines
    ed->lines = (char **)malloc(MAX_LINES * sizeof(char *));
    ed->lines = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    
    ed->line_count = 1;
    ed->cursor_x = 0;
    ed->cursor_y = 0;
    ed->filename = NULL;
    ed->modified = 0;
    
    // Get screen size
    getmaxyx(stdscr, ed->screen_rows, ed->screen_cols);
}

void cleanup_editor(EditorState *ed) {
    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
    }
    free(ed->lines);
    if (ed->filename) free(ed->filename);
}

void draw_screen(EditorState *ed) {
    clear();
    
    // Draw title bar
    attron(A_REVERSE);
    mvprintw(0, 0, "LIWIT v%s - %s", VERSION, 
             ed->filename ? ed->filename : "[New File]");
    for (int i = strlen("LIWIT") + strlen(VERSION) + 20; i < ed->screen_cols; i++) {
        addch(' ');
    }
    attroff(A_REVERSE);
    
    // Draw text lines
    for (int i = 0; i < ed->line_count && i < ed->screen_rows - 2; i++) {
        mvprintw(i + 1, 0, "%s", ed->lines[i]);
    }
    
    // Draw status bar
    attron(A_REVERSE);
    mvprintw(ed->screen_rows - 1, 0, "Ln %d, Col %d | Press F1 for help | Ctrl+Q to quit", 
             ed->cursor_y + 1, ed->cursor_x + 1);
    for (int i = 60; i < ed->screen_cols; i++) {
        addch(' ');
    }
    attroff(A_REVERSE);
    
    // Position cursor
    move(ed->cursor_y + 1, ed->cursor_x);
    
    refresh();
}

void handle_input(EditorState *ed) {
    int ch = getch();
    
    switch (ch) {
        case 17: // Ctrl+Q
            cleanup_editor(ed);
            endwin();
            exit(0);
            break;
            
        case KEY_UP:
            if (ed->cursor_y > 0) ed->cursor_y--;
            break;
            
        case KEY_DOWN:
            if (ed->cursor_y < ed->line_count - 1) ed->cursor_y++;
            break;
            
        case KEY_LEFT:
            if (ed->cursor_x > 0) ed->cursor_x--;
            break;
            
        case KEY_RIGHT:
            if (ed->cursor_x < strlen(ed->lines[ed->cursor_y])) ed->cursor_x++;
            break;
            
        default:
            // Handle regular character input
            if (ch >= 32 && ch <= 126) {
                char *line = ed->lines[ed->cursor_y];
                int len = strlen(line);
                
                // Shift characters right
                memmove(line + ed->cursor_x + 1, line + ed->cursor_x, len - ed->cursor_x + 1);
                line[ed->cursor_x] = ch;
                ed->cursor_x++;
                ed->modified = 1;
            }
            break;
    }
}
