/**
 * LIWIT - Linux-Windows Text Editor
 * Version 1.0
 * Author: Khud Bakhtiyar Iqbal Sofi
 */
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <linux/limits.h>

// Configuration
char *clipboard = NULL;

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
void save_file(EditorState *ed); //save file function prototype
void open_file(EditorState *ed, const char *filename); // open file function prototype

// Implementation of file operations
void save_file(EditorState *ed) {
    if (!ed->filename) {
        // Prompt for filename
        echo();
        char filename[MAX_LINE_LENGTH] = {0};
        mvprintw(ed->screen_rows - 1, 0, "Save as: ");
        clrtoeol();
        move(ed->screen_rows - 1, 9);               // place cursor after "Save as: "
        getnstr(filename, sizeof(filename) - 1);    // expects char*, int
        noecho();

        if (strlen(filename) > 0) {
            ed->filename = strdup(filename);
        } else {
            return;
        }
    }

    FILE *file = fopen(ed->filename, "w");
    if (!file) {
        mvprintw(ed->screen_rows - 1, 0, "Error: Cannot save file!");
        clrtoeol();
        refresh();
        napms(2000);
        return;
    }

    for (int i = 0; i < ed->line_count; i++) {
        fprintf(file, "%s\n", ed->lines[i] ? ed->lines[i] : "");
    }

    fclose(file);
    ed->modified = 0;

    mvprintw(ed->screen_rows - 1, 0, "File saved!");
    clrtoeol();
    refresh();
    napms(1000);
}

void open_file(EditorState *ed, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        mvprintw(ed->screen_rows - 1, 0, "Error: Cannot open file!");
        clrtoeol();
        refresh();
        napms(2000);
        return;
    }

    // Clear existing content (free each line)
    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
        ed->lines[i] = NULL;
    }

    ed->line_count = 0;
    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), file) && ed->line_count < MAX_LINES) {
        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        ed->lines[ed->line_count] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        strcpy(ed->lines[ed->line_count], buffer);
        ed->line_count++;
    }

    fclose(file);

    if (ed->line_count == 0) {
        // Ensure at least one editable empty line exists
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        ed->line_count = 1;
    }

    free(ed->filename);
    ed->filename = strdup(filename);
    ed->cursor_x = 0;
    ed->cursor_y = 0;
}



int main(int argc, char *argv[]) {
    EditorState editor;
    
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    
    // Initialize colors
    if (has_colors()) {
        start_color();
    }
    
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
    ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    
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
    
    // Draw menu bar with colors
    if (has_colors()) {
        init_pair(1, COLOR_BLACK, COLOR_CYAN);
        init_pair(2, COLOR_WHITE, COLOR_BLUE);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        
        attron(COLOR_PAIR(1));
        mvprintw(0, 0, " F1:Help ");
        mvprintw(0, 10, " Ctrl+S:Save ");
        mvprintw(0, 25, " Ctrl+O:Open ");
        mvprintw(0, 40, " Ctrl+Q:Quit ");
        for (int i = 54; i < ed->screen_cols; i++) {
            addch(' ');
        }
        attroff(COLOR_PAIR(1));
    } else {
        attron(A_REVERSE);
        mvprintw(0, 0, "F1:Help | Ctrl+S:Save | Ctrl+O:Open | Ctrl+Q:Quit");
        for (int i = 50; i < ed->screen_cols; i++) {
            addch(' ');
        }
        attroff(A_REVERSE);
    }
    
    // Draw line numbers with color
    for (int i = 0; i < ed->line_count && i < ed->screen_rows - 2; i++) {
        if (has_colors()) {
            attron(COLOR_PAIR(3));
            mvprintw(i + 1, 0, "%4d ", i + 1);
            attroff(COLOR_PAIR(3));
            mvprintw(i + 1, 5, "%s", ed->lines[i]);
        } else {
            mvprintw(i + 1, 0, "%4d %s", i + 1, ed->lines[i]);
        }
    }
    
    // Status bar
    if (has_colors()) {
        attron(COLOR_PAIR(2));
    } else {
        attron(A_REVERSE);
    }
    mvprintw(ed->screen_rows - 1, 0, " %s%s | Ln %d, Col %d | %d lines ", 
             ed->filename ? ed->filename : "[New File]",
             ed->modified ? " [Modified]" : "",
             ed->cursor_y + 1, ed->cursor_x + 1,
             ed->line_count);
    for (int i = 50; i < ed->screen_cols; i++) {
        addch(' ');
    }
    if (has_colors()) {
        attroff(COLOR_PAIR(2));
    } else {
        attroff(A_REVERSE);
    }
    
    // Position cursor (adjust for line numbers)
    move(ed->cursor_y + 1, ed->cursor_x + 5);
    
    refresh();
}



void handle_input(EditorState *ed) {
    int ch = getch();

    if (ch == KEY_RESIZE) {
        getmaxyx(stdscr, ed->screen_rows, ed->screen_cols);
        return;
    }

    switch (ch) {
        case KEY_BACKSPACE:
        case 127:
        case 8:
            if (ed->cursor_x > 0) {
                char *line = ed->lines[ed->cursor_y];
                int len = strlen(line);
                memmove(line + ed->cursor_x - 1, line + ed->cursor_x, len - ed->cursor_x + 1);
                ed->cursor_x--;
                ed->modified = 1;
            }
                break;

        case '\n':
        case KEY_ENTER:
            if (ed->line_count < MAX_LINES) {
                // Create new line
                char *new_line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
                char *current = ed->lines[ed->cursor_y];
                
                // Copy text after cursor to new line
                strcpy(new_line, current + ed->cursor_x);
                current[ed->cursor_x] = '\0';
                
                // Shift lines down
                for (int i = ed->line_count; i > ed->cursor_y + 1; i--) {
                    ed->lines[i] = ed->lines[i - 1];
                }
                
                ed->lines[ed->cursor_y + 1] = new_line;
                ed->line_count++;
                ed->cursor_y++;
                ed->cursor_x = 0;
                ed->modified = 1;
            }
                break;

    
        case 19: // Ctrl+S
            save_file(ed);
            break;

        case 15:  // Ctrl+O
            {
                char filename[256];
                echo();
                mvprintw(ed->screen_rows - 1, 0, "Open file: ");
                clrtoeol();
                getnstr(filename, sizeof(filename) - 1);
                noecho();
                
                if (strlen(filename) > 0) {
                    open_file(ed, filename);
                }
            }
            break;

        case 17: // Ctrl+Q
            cleanup_editor(ed);
            endwin();
            exit(0);
            break;

        case 3: // Ctrl+C - Copy line
            if (clipboard) free(clipboard);
            clipboard = strdup(ed->lines[ed->cursor_y]);
            mvprintw(ed->screen_rows - 1, 0, "Line copied");
            clrtoeol();
            refresh();
            napms(1000);
            break;

        case 22: // Ctrl+V - Paste
            if (clipboard) {
                for (int i = 0; clipboard[i] != '\0'; i++) {
                    char *line = ed->lines[ed->cursor_y];
                    int len = strlen(line);
                    memmove(line + ed->cursor_x + 1, line + ed->cursor_x, len - ed->cursor_x + 1);
                    line[ed->cursor_x] = clipboard[i];
                    ed->cursor_x++;
                }
                ed->modified = 1;
            }
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
