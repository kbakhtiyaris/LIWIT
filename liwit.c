/**
 * LIWIT - Linux-Windows Text Editor
 * Version 1.0 - Starter Template
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// CONFIGURATION - Adjust these as needed
// ============================================================================

#define VERSION "1.0"
#define MAX_LINES 5000
#define MAX_LINE_LENGTH 1024
#define TAB_SIZE 4

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Main editor state structure
 * This holds all information about the current editing session
 */
typedef struct {
    char **lines;              // Array of text lines
    int line_count;           // Number of lines in file
    int cursor_x;             // Cursor column position (0-based)
    int cursor_y;             // Cursor row position (0-based)
    int offset_x;             // Horizontal scroll offset
    int offset_y;             // Vertical scroll offset
    int screen_rows;          // Terminal height
    int screen_cols;          // Terminal width
    char *filename;           // Current filename (NULL if new)
    int modified;             // 1 if file has unsaved changes
    int insert_mode;          // 1 for insert, 0 for overwrite
} EditorState;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

char *clipboard = NULL;       // For copy/paste functionality

// ============================================================================
// FUNCTION PROTOTYPES - Declare all functions here
// ============================================================================

// Core functions
void init_editor(EditorState *ed);
void cleanup_editor(EditorState *ed);
void draw_screen(EditorState *ed);
void handle_input(EditorState *ed);

// Display functions
void draw_menu_bar(EditorState *ed);
void draw_status_bar(EditorState *ed);
void draw_text_area(EditorState *ed);
void show_message(EditorState *ed, const char *msg, int duration_ms);

// File operations
void save_file(EditorState *ed);
void open_file(EditorState *ed, const char *filename);

// Edit operations
void insert_char(EditorState *ed, char ch);
void delete_char_backspace(EditorState *ed);
void insert_newline(EditorState *ed);
void copy_line(EditorState *ed);
void paste_clipboard(EditorState *ed);

// Navigation
void move_cursor(EditorState *ed, int dy, int dx);
void move_to_line_start(EditorState *ed);
void move_to_line_end(EditorState *ed);
void scroll_if_needed(EditorState *ed);

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char *argv[]) {
    EditorState editor;
    
    // Initialize ncurses
    initscr();                // Start ncurses mode
    raw();                    // Disable line buffering (get each key)
    noecho();                 // Don't echo typed characters
    keypad(stdscr, TRUE);     // Enable function keys (F1, arrows, etc.)
    
    // Initialize colors if supported
    if (has_colors()) {
        start_color();
        // Define color pairs: init_pair(pair_number, foreground, background)
        init_pair(1, COLOR_BLACK, COLOR_CYAN);    // Menu bar
        init_pair(2, COLOR_WHITE, COLOR_BLUE);     // Status bar
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // Line numbers
        init_pair(4, COLOR_GREEN, COLOR_BLACK);    // Success messages
        init_pair(5, COLOR_RED, COLOR_BLACK);      // Error messages
    }
    
    // Initialize editor state
    init_editor(&editor);
    
    // Load file if specified as command line argument
    if (argc > 1) {
        open_file(&editor, argv[1]);
    }
    
    // Main editor loop - runs until user quits
    while (1) {
        draw_screen(&editor);    // Redraw the screen
        handle_input(&editor);   // Wait for and process input
    }
    
    // Cleanup (this won't be reached due to exit() in Ctrl+Q handler)
    cleanup_editor(&editor);
    endwin();
    
    return 0;
}

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

/**
 * Initialize the editor state with default values
 */
void init_editor(EditorState *ed) {
    // Allocate memory for lines array
    ed->lines = (char **)malloc(MAX_LINES * sizeof(char *));
    
    // Create first empty line
    ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    
    // Initialize state
    ed->line_count = 1;
    ed->cursor_x = 0;
    ed->cursor_y = 0;
    ed->offset_x = 0;
    ed->offset_y = 0;
    ed->filename = NULL;
    ed->modified = 0;
    ed->insert_mode = 1;  // Start in insert mode
    
    // Get terminal dimensions
    getmaxyx(stdscr, ed->screen_rows, ed->screen_cols);
}

/**
 * Free all allocated memory
 */
void cleanup_editor(EditorState *ed) {
    // Free each line
    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
    }
    
    // Free lines array
    free(ed->lines);
    
    // Free filename if set
    if (ed->filename) {
        free(ed->filename);
    }
    
    // Free clipboard if used
    if (clipboard) {
        free(clipboard);
    }
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

/**
 * Main screen drawing function - calls all sub-drawing functions
 */
void draw_screen(EditorState *ed) {
    clear();  // Clear the screen
    
    draw_menu_bar(ed);
    draw_text_area(ed);
    draw_status_bar(ed);
    
    // Position the cursor (add 1 for menu bar, add 5 for line numbers)
    move(ed->cursor_y - ed->offset_y + 1, ed->cursor_x - ed->offset_x + 5);
    
    refresh();  // Update the physical screen
}

/**
 * Draw the top menu bar with shortcuts
 */
void draw_menu_bar(EditorState *ed) {
    if (has_colors()) {
        attron(COLOR_PAIR(1));  // Use menu bar colors
    } else {
        attron(A_REVERSE);      // Use reverse video if no colors
    }
    
    // Draw menu items
    mvprintw(0, 0, " LIWIT v%s ", VERSION);
    mvprintw(0, 15, " Ctrl+S:Save ");
    mvprintw(0, 30, " Ctrl+O:Open ");
    mvprintw(0, 45, " Ctrl+Q:Quit ");
    mvprintw(0, 60, " F1:Help ");
    
    // Fill rest of line with spaces
    for (int i = 70; i < ed->screen_cols; i++) {
        addch(' ');
    }
    
    if (has_colors()) {
        attroff(COLOR_PAIR(1));
    } else {
        attroff(A_REVERSE);
    }
}

/**
 * Draw the text editing area with line numbers
 */
void draw_text_area(EditorState *ed) {
    int visible_rows = ed->screen_rows - 2;  // Minus menu and status bars
    
    for (int screen_row = 0; screen_row < visible_rows; screen_row++) {
        int file_line = ed->offset_y + screen_row;
        
        // Stop if we've displayed all lines
        if (file_line >= ed->line_count) {
            break;
        }
        
        int screen_y = screen_row + 1;  // +1 for menu bar
        
        // Draw line number
        if (has_colors()) {
            attron(COLOR_PAIR(3));
        }
        mvprintw(screen_y, 0, "%4d ", file_line + 1);
        if (has_colors()) {
            attroff(COLOR_PAIR(3));
        }
        
        // Draw line content (with horizontal scrolling)
        char *line = ed->lines[file_line];
        int line_len = strlen(line);
        int visible_cols = ed->screen_cols - 5;  // Minus line number width
        
        for (int x = ed->offset_x; x < ed->offset_x + visible_cols && x < line_len; x++) {
            mvaddch(screen_y, 5 + (x - ed->offset_x), line[x]);
        }
    }
}

/**
 * Draw the bottom status bar
 */
void draw_status_bar(EditorState *ed) {
    int status_y = ed->screen_rows - 1;
    
    if (has_colors()) {
        attron(COLOR_PAIR(2));
    } else {
        attron(A_REVERSE);
    }
    
    // Left side: filename and modified indicator
    mvprintw(status_y, 0, " %s%s ",
             ed->filename ? ed->filename : "[New File]",
             ed->modified ? " [+]" : "");
    
    // Center: mode indicator
    const char *mode = ed->insert_mode ? "INSERT" : "OVERWRITE";
    int center_x = (ed->screen_cols - strlen(mode)) / 2;
    mvprintw(status_y, center_x, "%s", mode);
    
    // Right side: position and line count
    char right_info[64];
    snprintf(right_info, sizeof(right_info), "Ln %d/%d, Col %d ",
             ed->cursor_y + 1, ed->line_count, ed->cursor_x + 1);
    mvprintw(status_y, ed->screen_cols - strlen(right_info), "%s", right_info);
    
    // Fill gaps with spaces
    for (int x = 0; x < ed->screen_cols; x++) {
        if (mvinch(status_y, x) == ' ') {
            mvaddch(status_y, x, ' ');
        }
    }
    
    if (has_colors()) {
        attroff(COLOR_PAIR(2));
    } else {
        attroff(A_REVERSE);
    }
}

/**
 * Show a temporary message in the status bar
 */
void show_message(EditorState *ed, const char *msg, int duration_ms) {
    int msg_y = ed->screen_rows - 1;
    
    // Clear status line
    move(msg_y, 0);
    clrtoeol();
    
    // Show message
    mvprintw(msg_y, 2, "%s", msg);
    refresh();
    
    // Wait for specified duration
    napms(duration_ms);
}

// ============================================================================
// FILE OPERATIONS
// ============================================================================

/**
 * Save current file to disk
 */
void save_file(EditorState *ed) {
    // If no filename, prompt for one
    if (!ed->filename) {
        char filename[256];
        
        echo();  // Enable character echoing for input
        mvprintw(ed->screen_rows - 1, 0, "Save as: ");
        clrtoeol();
        getnstr(filename, sizeof(filename) - 1);
        noecho();  // Disable echoing again
        
        if (strlen(filename) > 0) {
            ed->filename = strdup(filename);
        } else {
            show_message(ed, "Save cancelled", 1000);
            return;
        }
    }
    
    // Open file for writing
    FILE *file = fopen(ed->filename, "w");
    if (!file) {
        show_message(ed, "ERROR: Cannot save file!", 2000);
        return;
    }
    
    // Write each line
    for (int i = 0; i < ed->line_count; i++) {
        fprintf(file, "%s\n", ed->lines[i]);
    }
    
    fclose(file);
    ed->modified = 0;
    show_message(ed, "File saved successfully!", 1000);
}

/**
 * Open a file from disk
 */
void open_file(EditorState *ed, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        show_message(ed, "ERROR: Cannot open file!", 2000);
        return;
    }
    
    // Free existing lines
    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
    }
    
    // Read file line by line
    ed->line_count = 0;
    char buffer[MAX_LINE_LENGTH];
    
    while (fgets(buffer, sizeof(buffer), file) && ed->line_count < MAX_LINES) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        // Allocate and copy line
        ed->lines[ed->line_count] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        strcpy(ed->lines[ed->line_count], buffer);
        ed->line_count++;
    }
    
    fclose(file);
    
    // Ensure at least one line exists
    if (ed->line_count == 0) {
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        ed->line_count = 1;
    }
    
    // Update state
    ed->filename = strdup(filename);
    ed->cursor_x = 0;
    ed->cursor_y = 0;
    ed->offset_x = 0;
    ed->offset_y = 0;
    ed->modified = 0;
}

// ============================================================================
// EDIT OPERATIONS
// ============================================================================

/**
 * Insert a character at cursor position
 */
void insert_char(EditorState *ed, char ch) {
    char *line = ed->lines[ed->cursor_y];
    int len = strlen(line);
    
    // Check line length limit
    if (len >= MAX_LINE_LENGTH - 1) {
        show_message(ed, "Line too long!", 1000);
        return;
    }
    
    if (ed->insert_mode) {
        // Insert mode: shift characters right
        memmove(line + ed->cursor_x + 1, line + ed->cursor_x, len - ed->cursor_x + 1);
    }
    
    // Insert the character
    line[ed->cursor_x] = ch;
    ed->cursor_x++;
    ed->modified = 1;
    
    scroll_if_needed(ed);
}

/**
 * Delete character before cursor (Backspace)
 */
void delete_char_backspace(EditorState *ed) {
    if (ed->cursor_x > 0) {
        // Delete character in current line
        char *line = ed->lines[ed->cursor_y];
        int len = strlen(line);
        
        memmove(line + ed->cursor_x - 1, line + ed->cursor_x, len - ed->cursor_x + 1);
        ed->cursor_x--;
        ed->modified = 1;
    } else if (ed->cursor_y > 0) {
        // Merge with previous line
        int prev_len = strlen(ed->lines[ed->cursor_y - 1]);
        
        // Check if merged line would be too long
        if (prev_len + strlen(ed->lines[ed->cursor_y]) < MAX_LINE_LENGTH) {
            strcat(ed->lines[ed->cursor_y - 1], ed->lines[ed->cursor_y]);
            
            // Delete current line
            free(ed->lines[ed->cursor_y]);
            for (int i = ed->cursor_y; i < ed->line_count - 1; i++) {
                ed->lines[i] = ed->lines[i + 1];
            }
            ed->line_count--;
            
            ed->cursor_y--;
            ed->cursor_x = prev_len;
            ed->modified = 1;
        }
    }
    
    scroll_if_needed(ed);
}

/**
 * Insert a new line (Enter key)
 */
void insert_newline(EditorState *ed) {
    if (ed->line_count >= MAX_LINES) {
        show_message(ed, "Maximum lines reached!", 1000);
        return;
    }
    
    // Split current line at cursor
    char *current = ed->lines[ed->cursor_y];
    char *new_line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    
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
    
    scroll_if_needed(ed);
}

/**
 * Copy current line to clipboard
 */
void copy_line(EditorState *ed) {
    if (clipboard) {
        free(clipboard);
    }
    clipboard = strdup(ed->lines[ed->cursor_y]);
    show_message(ed, "Line copied", 800);
}

/**
 * Paste clipboard content at cursor
 */
void paste_clipboard(EditorState *ed) {
    if (!clipboard) {
        show_message(ed, "Clipboard is empty", 1000);
        return;
    }
    
    // Insert each character from clipboard
    for (int i = 0; clipboard[i] != '\0'; i++) {
        insert_char(ed, clipboard[i]);
    }
}

// ============================================================================
// NAVIGATION
// ============================================================================

/**
 * Move cursor by delta (with bounds checking)
 */
void move_cursor(EditorState *ed, int dy, int dx) {
    ed->cursor_y += dy;
    ed->cursor_x += dx;
    
    // Vertical bounds
    if (ed->cursor_y < 0) ed->cursor_y = 0;
    if (ed->cursor_y >= ed->line_count) ed->cursor_y = ed->line_count - 1;
    
    // Horizontal bounds (snap to line end if past it)
    int line_len = strlen(ed->lines[ed->cursor_y]);
    if (ed->cursor_x < 0) ed->cursor_x = 0;
    if (ed->cursor_x > line_len) ed->cursor_x = line_len;
    
    scroll_if_needed(ed);
}

/**
 * Move cursor to start of current line
 */
void move_to_line_start(EditorState *ed) {
    ed->cursor_x = 0;
    scroll_if_needed(ed);
}

/**
 * Move cursor to end of current line
 */
void move_to_line_end(EditorState *ed) {
    ed->cursor_x = strlen(ed->lines[ed->cursor_y]);
    scroll_if_needed(ed);
}

/**
 * Adjust scroll offsets if cursor moves out of view
 */
void scroll_if_needed(EditorState *ed) {
    int visible_rows = ed->screen_rows - 2;  // Minus menu and status
    int visible_cols = ed->screen_cols - 5;  // Minus line numbers
    
    // Vertical scrolling
    if (ed->cursor_y < ed->offset_y) {
        ed->offset_y = ed->cursor_y;
    } else if (ed->cursor_y >= ed->offset_y + visible_rows) {
        ed->offset_y = ed->cursor_y - visible_rows + 1;
    }
    
    // Horizontal scrolling
    if (ed->cursor_x < ed->offset_x) {
        ed->offset_x = ed->cursor_x;
    } else if (ed->cursor_x >= ed->offset_x + visible_cols) {
        ed->offset_x = ed->cursor_x - visible_cols + 1;
    }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * Handle keyboard input - this is where you add new shortcuts!
 */
void handle_input(EditorState *ed) {
    int ch = getch();  // Wait for key press
    
    switch (ch) {
        // ===== FILE OPERATIONS =====
        case 19:  // Ctrl+S
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
            
        case 17:  // Ctrl+Q
            if (ed->modified) {
                mvprintw(ed->screen_rows - 1, 0, "Save changes? (y/n): ");
                clrtoeol();
                refresh();
                
                int response = getch();
                if (response == 'y' || response == 'Y') {
                    save_file(ed);
                }
            }
            cleanup_editor(ed);
            endwin();
            exit(0);
            break;
        
        // ===== EDIT OPERATIONS =====
        case 3:  // Ctrl+C
            copy_line(ed);
            break;
            
        case 22:  // Ctrl+V
            paste_clipboard(ed);
            break;
            
        case KEY_IC:  // Insert key
            ed->insert_mode = !ed->insert_mode;
            break;
        
        // ===== NAVIGATION =====
        case KEY_UP:
            move_cursor(ed, -1, 0);
            break;
            
        case KEY_DOWN:
            move_cursor(ed, 1, 0);
            break;
            
        case KEY_LEFT:
            move_cursor(ed, 0, -1);
            break;
            
        case KEY_RIGHT:
            move_cursor(ed, 0, 1);
            break;
            
        case KEY_HOME:
            move_to_line_start(ed);
            break;
            
        case KEY_END:
            move_to_line_end(ed);
            break;
            
        case KEY_PPAGE:  // Page Up
            move_cursor(ed, -10, 0);
            break;
            
        case KEY_NPAGE:  // Page Down
            move_cursor(ed, 10, 0);
            break;
        
        // ===== TEXT EDITING =====
        case '\n':
        case KEY_ENTER:
            insert_newline(ed);
            break;
            
        case KEY_BACKSPACE:
        case 127:
        case 8:
            delete_char_backspace(ed);
            break;
            
        case KEY_DC:  // Delete key
            if (ed->cursor_x < strlen(ed->lines[ed->cursor_y])) {
                ed->cursor_x++;
                delete_char_backspace(ed);
            }
            break;
            
        case '\t':  // Tab key
            for (int i = 0; i < TAB_SIZE; i++) {
                insert_char(ed, ' ');
            }
            break;
        
        // ===== REGULAR CHARACTERS =====
        default:
            if (ch >= 32 && ch <= 126) {
                insert_char(ed, (char)ch);
            }
            break;
    }
}