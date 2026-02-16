/**
 * LIWIT - Linux-Windows Text Editor
 * Version 1.0 - Multi-line selection + cut/copy/paste
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// CONFIGURATION
#define VERSION "1.0"
#define MAX_LINES 5000
#define MAX_LINE_LENGTH 1024
#define TAB_SIZE 4

// DATA STRUCTURES
typedef struct {
    char **lines;              // Array of text lines
    int line_count;            // Number of lines in file
    int cursor_x;              // Cursor column position (0-based)
    int cursor_y;              // Cursor row position (0-based)
    int offset_x;              // Horizontal scroll offset
    int offset_y;              // Vertical scroll offset
    int screen_rows;           // Terminal height
    int screen_cols;           // Terminal width
    char *filename;            // Current filename (NULL if new)
    int modified;              // 1 if file has unsaved changes
    int insert_mode;           // 1 for insert, 0 for overwrite

    int selecting;             // 1 if selection active
    int sel_start_y;           // selection start line
    int sel_end_y;             // selection end line
} EditorState;

// GLOBALS
char *clipboard = NULL;

// PROTOTYPES
void init_editor(EditorState *ed);
void cleanup_editor(EditorState *ed);
void draw_screen(EditorState *ed);
void handle_input(EditorState *ed);

void draw_menu_bar(EditorState *ed);
void draw_status_bar(EditorState *ed);
void draw_text_area(EditorState *ed);
void show_message(EditorState *ed, const char *msg, int duration_ms);

void save_file(EditorState *ed);
void open_file(EditorState *ed, const char *filename);

void insert_char(EditorState *ed, char ch);
void delete_char_backspace(EditorState *ed);
void insert_newline(EditorState *ed);
void copy_line(EditorState *ed);
void cut_line(EditorState *ed);
void paste_clipboard(EditorState *ed);

void move_cursor(EditorState *ed, int dy, int dx);
void move_to_line_start(EditorState *ed);
void move_to_line_end(EditorState *ed);
void scroll_if_needed(EditorState *ed);

// selection helpers
void get_selection_range(EditorState *ed, int *start, int *end);
void copy_selection(EditorState *ed);
void cut_selection(EditorState *ed);
void delete_selection(EditorState *ed);

// MAIN
int main(int argc, char *argv[]) {
    EditorState editor;

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_CYAN);    // Menu bar
        init_pair(2, COLOR_WHITE, COLOR_BLUE);    // Status bar
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Line numbers
        init_pair(4, COLOR_GREEN, COLOR_BLACK);   // Success messages
        init_pair(5, COLOR_RED, COLOR_BLACK);     // Error messages
    }

    init_editor(&editor);

    if (argc > 1) {
        open_file(&editor, argv[1]);
    }

    while (1) {
        draw_screen(&editor);
        handle_input(&editor);
    }

    cleanup_editor(&editor);
    endwin();
    return 0;
}

// INITIALIZATION & CLEANUP
void init_editor(EditorState *ed) {
    ed->lines = (char **)malloc(MAX_LINES * sizeof(char *));
    ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));

    ed->line_count = 1;
    ed->cursor_x = 0;
    ed->cursor_y = 0;
    ed->offset_x = 0;
    ed->offset_y = 0;
    ed->filename = NULL;
    ed->modified = 0;
    ed->insert_mode = 1;

    ed->selecting = 0;
    ed->sel_start_y = 0;
    ed->sel_end_y = 0;

    getmaxyx(stdscr, ed->screen_rows, ed->screen_cols);
}

void cleanup_editor(EditorState *ed) {
    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
    }
    free(ed->lines);
    if (ed->filename) free(ed->filename);
    if (clipboard) free(clipboard);
}

// DISPLAY
void draw_screen(EditorState *ed) {
    clear();
    draw_menu_bar(ed);
    draw_text_area(ed);
    draw_status_bar(ed);
    move(ed->cursor_y - ed->offset_y + 1,
         ed->cursor_x - ed->offset_x + 5);
    refresh();
}

void draw_menu_bar(EditorState *ed) {
    if (has_colors()) attron(COLOR_PAIR(1));
    else attron(A_REVERSE);

    mvprintw(0, 0, " LIWIT v%s ", VERSION);
    mvprintw(0, 15, " Ctrl+S:Save ");
    mvprintw(0, 30, " Ctrl+O:Open ");
    mvprintw(0, 45, " Ctrl+Q:Quit ");
    mvprintw(0, 60, " F1:Help ");
    mvprintw(0, 72, " F2:Select ");

    for (int i = 82; i < ed->screen_cols; i++) addch(' ');

    if (has_colors()) attroff(COLOR_PAIR(1));
    else attroff(A_REVERSE);
}

void get_selection_range(EditorState *ed, int *start, int *end) {
    if (!ed->selecting) {
        *start = *end = -1;
        return;
    }
    int s = ed->sel_start_y;
    int e = ed->sel_end_y;
    if (s > e) { int tmp = s; s = e; e = tmp; }
    if (s < 0) s = 0;
    if (e >= ed->line_count) e = ed->line_count - 1;
    *start = s;
    *end = e;
}

void draw_text_area(EditorState *ed) {
    int visible_rows = ed->screen_rows - 2;
    int sel_start, sel_end;
    get_selection_range(ed, &sel_start, &sel_end);

    for (int screen_row = 0; screen_row < visible_rows; screen_row++) {
        int file_line = ed->offset_y + screen_row;
        if (file_line >= ed->line_count) break;

        int screen_y = screen_row + 1;
        int is_selected = ed->selecting &&
                          file_line >= sel_start &&
                          file_line <= sel_end;

        if (is_selected) attron(A_REVERSE);

        if (has_colors()) attron(COLOR_PAIR(3));
        mvprintw(screen_y, 0, "%4d ", file_line + 1);
        if (has_colors()) attroff(COLOR_PAIR(3));

        char *line = ed->lines[file_line];
        int line_len = strlen(line);
        int visible_cols = ed->screen_cols - 5;

        for (int x = ed->offset_x;
             x < ed->offset_x + visible_cols && x < line_len;
             x++) {
            mvaddch(screen_y, 5 + (x - ed->offset_x), line[x]);
        }

        if (is_selected) attroff(A_REVERSE);
    }
}

void draw_status_bar(EditorState *ed) {
    int status_y = ed->screen_rows - 1;

    if (has_colors()) attron(COLOR_PAIR(2));
    else attron(A_REVERSE);

    mvprintw(status_y, 0, " %s%s ",
             ed->filename ? ed->filename : "[New File]",
             ed->modified ? " [+]" : "");

    const char *mode = ed->insert_mode ? "INSERT" : "OVERWRITE";
    int center_x = (ed->screen_cols - (int)strlen(mode)) / 2;
    mvprintw(status_y, center_x, "%s", mode);

    char right_info[64];
    snprintf(right_info, sizeof(right_info),
             "Ln %d/%d, Col %d ",
             ed->cursor_y + 1, ed->line_count, ed->cursor_x + 1);
    mvprintw(status_y,
             ed->screen_cols - (int)strlen(right_info),
             "%s", right_info);

    for (int x = 0; x < ed->screen_cols; x++) {
        mvaddch(status_y, x, mvinch(status_y, x) & A_CHARTEXT);
    }

    if (has_colors()) attroff(COLOR_PAIR(2));
    else attroff(A_REVERSE);
}

void show_message(EditorState *ed, const char *msg, int duration_ms) {
    int msg_y = ed->screen_rows - 1;
    move(msg_y, 0);
    clrtoeol();
    mvprintw(msg_y, 2, "%s", msg);
    refresh();
    napms(duration_ms);
}

// FILE OPS
void save_file(EditorState *ed) {
    if (!ed->filename) {
        char filename[256];
        echo();
        mvprintw(ed->screen_rows - 1, 0, "Save as: ");
        clrtoeol();
        getnstr(filename, sizeof(filename) - 1);
        noecho();
        if (strlen(filename) > 0) {
            ed->filename = strdup(filename);
        } else {
            show_message(ed, "Save cancelled", 1000);
            return;
        }
    }

    FILE *file = fopen(ed->filename, "w");
    if (!file) {
        show_message(ed, "ERROR: Cannot save file!", 2000);
        return;
    }

    for (int i = 0; i < ed->line_count; i++) {
        fprintf(file, "%s\n", ed->lines[i]);
    }

    fclose(file);
    ed->modified = 0;
    show_message(ed, "File saved successfully!", 1000);
}

void open_file(EditorState *ed, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        show_message(ed, "ERROR: Cannot open file!", 2000);
        return;
    }

    for (int i = 0; i < ed->line_count; i++) {
        free(ed->lines[i]);
    }

    ed->line_count = 0;
    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), file) &&
           ed->line_count < MAX_LINES) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        ed->lines[ed->line_count] =
            (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        strcpy(ed->lines[ed->line_count], buffer);
        ed->line_count++;
    }

    fclose(file);

    if (ed->line_count == 0) {
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
        ed->line_count = 1;
    }

    if (ed->filename) free(ed->filename);
    ed->filename = strdup(filename);
    ed->cursor_x = 0;
    ed->cursor_y = 0;
    ed->offset_x = 0;
    ed->offset_y = 0;
    ed->modified = 0;
}

// EDIT OPS
void insert_char(EditorState *ed, char ch) {
    char *line = ed->lines[ed->cursor_y];
    int len = strlen(line);

    if (len >= MAX_LINE_LENGTH - 1) {
        show_message(ed, "Line too long!", 1000);
        return;
    }

    if (ed->insert_mode) {
        memmove(line + ed->cursor_x + 1,
                line + ed->cursor_x,
                len - ed->cursor_x + 1);
    }

    line[ed->cursor_x] = ch;
    ed->cursor_x++;
    ed->modified = 1;

    scroll_if_needed(ed);
}

void delete_char_backspace(EditorState *ed) {
    if (ed->cursor_x > 0) {
        char *line = ed->lines[ed->cursor_y];
        int len = strlen(line);
        memmove(line + ed->cursor_x - 1,
                line + ed->cursor_x,
                len - ed->cursor_x + 1);
        ed->cursor_x--;
        ed->modified = 1;
    } else if (ed->cursor_y > 0) {
        int prev_len = strlen(ed->lines[ed->cursor_y - 1]);
        if (prev_len + (int)strlen(ed->lines[ed->cursor_y]) <
            MAX_LINE_LENGTH) {
            strcat(ed->lines[ed->cursor_y - 1],
                   ed->lines[ed->cursor_y]);
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

void insert_newline(EditorState *ed) {
    if (ed->line_count >= MAX_LINES) {
        show_message(ed, "Maximum lines reached!", 1000);
        return;
    }

    char *current = ed->lines[ed->cursor_y];
    char *new_line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));

    strcpy(new_line, current + ed->cursor_x);
    current[ed->cursor_x] = '\0';

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

void copy_line(EditorState *ed) {
    if (clipboard) free(clipboard);
    clipboard = strdup(ed->lines[ed->cursor_y]);
    show_message(ed, "Line copied", 800);
}

void cut_line(EditorState *ed) {
    if (clipboard) free(clipboard);
    clipboard = strdup(ed->lines[ed->cursor_y]);

    free(ed->lines[ed->cursor_y]);

    if (ed->line_count == 1) {
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    } else {
        for (int i = ed->cursor_y; i < ed->line_count - 1; i++) {
            ed->lines[i] = ed->lines[i + 1];
        }
        ed->line_count--;
        if (ed->cursor_y >= ed->line_count)
            ed->cursor_y = ed->line_count - 1;
    }

    ed->cursor_x = 0;
    ed->modified = 1;
    scroll_if_needed(ed);
    show_message(ed, "Line cut", 800);
}

void copy_selection(EditorState *ed) {
    int start, end;
    get_selection_range(ed, &start, &end);
    if (start == -1) {
        copy_line(ed);
        return;
    }

    size_t buf_size = 0;
    for (int i = start; i <= end; i++) {
        buf_size += strlen(ed->lines[i]) + 1;
    }
    char *buf = (char *)malloc(buf_size + 1);
    buf[0] = '\0';

    for (int i = start; i <= end; i++) {
        strcat(buf, ed->lines[i]);
        if (i != end) strcat(buf, "\n");
    }

    if (clipboard) free(clipboard);
    clipboard = buf;

    show_message(ed, "Selection copied", 800);
}

void cut_selection(EditorState *ed) {
    int start, end;
    get_selection_range(ed, &start, &end);
    if (start == -1) {
        cut_line(ed);
        return;
    }

    copy_selection(ed);

    int count = end - start + 1;
    for (int i = start; i <= end; i++) {
        free(ed->lines[i]);
    }
    for (int i = start; i + count < ed->line_count; i++) {
        ed->lines[i] = ed->lines[i + count];
    }
    ed->line_count -= count;

    if (ed->line_count <= 0) {
        ed->line_count = 1;
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    }

    ed->cursor_y = start;
    if (ed->cursor_y >= ed->line_count)
        ed->cursor_y = ed->line_count - 1;
    ed->cursor_x = 0;
    ed->modified = 1;
    ed->selecting = 0;

    scroll_if_needed(ed);
    show_message(ed, "Selection cut", 800);
}

void delete_selection(EditorState *ed) {
    int start, end;
    get_selection_range(ed, &start, &end);
    if (start == -1) return;

    int count = end - start + 1;
    for (int i = start; i <= end; i++) {
        free(ed->lines[i]);
    }
    for (int i = start; i + count < ed->line_count; i++) {
        ed->lines[i] = ed->lines[i + count];
    }
    ed->line_count -= count;

    if (ed->line_count <= 0) {
        ed->line_count = 1;
        ed->lines[0] = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    }

    ed->cursor_y = start;
    if (ed->cursor_y >= ed->line_count)
        ed->cursor_y = ed->line_count - 1;
    ed->cursor_x = 0;
    ed->modified = 1;
    ed->selecting = 0;

    scroll_if_needed(ed);
    show_message(ed, "Selection deleted", 800);
}

void paste_clipboard(EditorState *ed) {
    if (!clipboard) {
        show_message(ed, "Clipboard is empty", 1000);
        return;
    }

    const char *p = clipboard;
    while (*p) {
        if (*p == '\n') {
            insert_newline(ed);
        } else {
            insert_char(ed, *p);
        }
        p++;
    }

    show_message(ed, "Pasted", 800);
}

// NAVIGATION
void move_cursor(EditorState *ed, int dy, int dx) {
    ed->cursor_y += dy;
    ed->cursor_x += dx;

    if (ed->cursor_y < 0) ed->cursor_y = 0;
    if (ed->cursor_y >= ed->line_count)
        ed->cursor_y = ed->line_count - 1;

    int line_len = strlen(ed->lines[ed->cursor_y]);
    if (ed->cursor_x < 0) ed->cursor_x = 0;
    if (ed->cursor_x > line_len) ed->cursor_x = line_len;

    scroll_if_needed(ed);
}

void move_to_line_start(EditorState *ed) {
    ed->cursor_x = 0;
    scroll_if_needed(ed);
}

void move_to_line_end(EditorState *ed) {
    ed->cursor_x = strlen(ed->lines[ed->cursor_y]);
    scroll_if_needed(ed);
}

void scroll_if_needed(EditorState *ed) {
    int visible_rows = ed->screen_rows - 2;
    int visible_cols = ed->screen_cols - 5;

    if (ed->cursor_y < ed->offset_y) {
        ed->offset_y = ed->cursor_y;
    } else if (ed->cursor_y >= ed->offset_y + visible_rows) {
        ed->offset_y = ed->cursor_y - visible_rows + 1;
    }

    if (ed->cursor_x < ed->offset_x) {
        ed->offset_x = ed->cursor_x;
    } else if (ed->cursor_x >= ed->offset_x + visible_cols) {
        ed->offset_x = ed->cursor_x - visible_cols + 1;
    }
}

// INPUT
void handle_input(EditorState *ed) {
    int ch = getch();

    switch (ch) {
        // FILE
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
                mvprintw(ed->screen_rows - 1, 0,
                         "Save changes? (y/n): ");
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

        // SELECTION
        case KEY_F(2):  // Toggle selection mode
            if (!ed->selecting) {
                ed->selecting = 1;
                ed->sel_start_y = ed->sel_end_y = ed->cursor_y;
                show_message(ed, "Selection started", 800);
            } else {
                ed->selecting = 0;
                show_message(ed, "Selection cleared", 800);
            }
            break;

        // EDIT
        case 3:  // Ctrl+C
            if (ed->selecting) copy_selection(ed);
            else copy_line(ed);
            break;

        case 24:  // Ctrl+X
            if (ed->selecting) cut_selection(ed);
            else cut_line(ed);
            break;

        case 22:  // Ctrl+V
            paste_clipboard(ed);
            break;

        case KEY_IC:
            ed->insert_mode = !ed->insert_mode;
            break;

        // NAVIGATION
        case KEY_UP:
            move_cursor(ed, -1, 0);
            if (ed->selecting) ed->sel_end_y = ed->cursor_y;
            break;

        case KEY_DOWN:
            move_cursor(ed, 1, 0);
            if (ed->selecting) ed->sel_end_y = ed->cursor_y;
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

        case KEY_PPAGE:
            move_cursor(ed, -10, 0);
            if (ed->selecting) ed->sel_end_y = ed->cursor_y;
            break;

        case KEY_NPAGE:
            move_cursor(ed, 10, 0);
            if (ed->selecting) ed->sel_end_y = ed->cursor_y;
            break;

        // TEXT
        case '\n':
        case KEY_ENTER:
            insert_newline(ed);
            break;

        case KEY_BACKSPACE:
        case 127:
        case 8:
            if (ed->selecting) {
                delete_selection(ed);
            } else {
                delete_char_backspace(ed);
            }
            break;

        case KEY_DC:
            if (ed->selecting) {
                delete_selection(ed);
            } else if (ed->cursor_x <
                       (int)strlen(ed->lines[ed->cursor_y])) {
                ed->cursor_x++;
                delete_char_backspace(ed);
            }
            break;

        case '\t':
            for (int i = 0; i < TAB_SIZE; i++) {
                insert_char(ed, ' ');
            }
            break;

        default:
            if (ch >= 32 && ch <= 126) {
                if (ed->selecting) {
                    // typing clears selection
                    ed->selecting = 0;
                }
                insert_char(ed, (char)ch);
            }
            break;
    }
}
