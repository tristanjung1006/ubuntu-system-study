#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

#define MAX_ENTRIES 100

typedef struct {
    char name[30];
    char phone[20];
    char memo[50];
} Entry;

Entry phonebook[MAX_ENTRIES];
int entry_count = 0;

// 데이터 파일에서 데이터를 읽는다.
void load_data(const char *filename) {
    FILE *file = fopen(filename, "r");  // 읽기 모드로 열람
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    
    // 파일에서 데이터를 읽어와서 전화번호부에 저장한다.
    while (fscanf(file, "%29[^:]:%19[^:]:%49[^\n]\n", 
                  phonebook[entry_count].name, 
                  phonebook[entry_count].phone, 
                  phonebook[entry_count].memo) != EOF) {
        entry_count++;
    }
    fclose(file);
}

// 전화번호부 데이터를 파일에 저장한다.
void save_data(const char *filename) {
    FILE *file = fopen(filename, "w");  // 쓰기 모드로 열람 
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    
    // 전화번호부의 모든 항목을 파일에 쓴다.
    for (int i = 0; i < entry_count; i++) {
        fprintf(file, "%s:%s:%s\n", 
                phonebook[i].name, 
                phonebook[i].phone, 
                phonebook[i].memo);
    }
    fclose(file);
}

// 전화번호부의 모든 항목을 리스트 형식으로 출력한다.
void list_entries() {
    clear();  // 화면 지우기
    for (int i = 0; i < entry_count; i++) {
        printw("%d. %s, %s, %s\n", i + 1, phonebook[i].name, phonebook[i].phone, phonebook[i].memo);
    }
    refresh();  // 화면 갱신
    getch();  // 사용자의 입력 대기
}

// 전화번호부에 새로운 항목을 추가한다.
void add_entry() {
    if (entry_count >= MAX_ENTRIES) {
        printw("There is no space in phonebook!\n");
        refresh();
        getch();
        return;
    }

    char name[30], phone[20], memo[50];
    echo();  // 사용자의 입력을 화면에 표시한다.
    printw("Enter name: ");
    getnstr(name, sizeof(name) - 1);
    printw("Enter phone number: ");
    getnstr(phone, sizeof(phone) - 1);
    printw("Enter description: ");
    getnstr(memo, sizeof(memo) - 1);
    noecho();  // 사용자의 입력을 화면에 표시하지 않는다.

    // 입력받은 데이터를 바탕으로 전화번호부에 일괄 저장한다.
    strcpy(phonebook[entry_count].name, name);
    strcpy(phonebook[entry_count].phone, phone);
    strcpy(phonebook[entry_count].memo, memo);
    entry_count++;

    save_data("data.txt");
    printw("successfully add entry!\n");
    refresh();
    getch();
}

// 전화번호부에서 항목을 삭제한다.
void delete_entry() {
    int index;
    echo();
    printw("Enter entry number to delete: ");
    scanw("%d", &index);  // 삭제할 항목 정보를 입력받는다.
    noecho();

    if (index < 1 || index > entry_count) {
        printw("wrong type of entry number!\n");
    } else {
        // 삭제할 항목 이후의 모든 항목을 앞으로 한 칸씩 이동시킨다.
	for (int i = index - 1; i < entry_count - 1; i++) {
            phonebook[i] = phonebook[i + 1];
        }
        entry_count--;
        save_data("data.txt");
        printw("successfully deleted entry number!\n");
    }
    refresh();
    getch();
}

// 전화번호부에서 이름 기준으로  항목을 검색한다.
void search_entry() {
    char name[30];
    echo();
    printw("Enter name to search: ");
    getnstr(name, sizeof(name) - 1);
    noecho();

    clear();
    // 전화번호부의 모든 항목을 검색하여 일치하는 항목을 출력한다.
    for (int i = 0; i < entry_count; i++) {
        if (strstr(phonebook[i].name, name) != NULL) {
            printw("%d. %s, %s, %s\n", i + 1, phonebook[i].name, phonebook[i].phone, phonebook[i].memo);
        }
    }
    refresh();
    getch();
}

// 프로그램 메인 메뉴 UI를 화면 정중앙에  표시
void print_center(WINDOW *win, int starty, int startx, int width, char *string) {
    int length, x, y;
    float temp;

    // 화면의 X좌표값과 Y좌표값을 측정
    if(win == NULL)
        win = stdscr;
    getyx(win, y, x);
    if(startx != 0)
        x = startx;
    if(starty != 0)
        y = starty;
    if(width == 0)
        width = 80;
    
    length = strlen(string);
    temp = (width - length) / 2;
    x = startx + (int)temp;
    mvwprintw(win, y, x, "%s", string);
    refresh();
}

// 메뉴 항목들의 애니메이션 효과 표시
void animate_highlight(WINDOW *win, int y, int x, int width, char *text) {
    int len = strlen(text);
    for (int i = 0; i < width - len; i++) {
        mvwprintw(win, y, x + i, "%s", text);
        wrefresh(win);
        mvwprintw(win, y, x + i, "%*s", len, "");  // 이전 텍스트 삭제
    }
    mvwprintw(win, y, x + (width - len) / 2, "%s", text);
}

// 메뉴 UI를 표시하고 사용자 액션을 처리한다.
void menu() {
    int choice;
    int highlight = 0;
    const char *choices[] = {
        "List entries",
        "Add entry",
        "Delete entry",
        "Search entry",
        "Exit"
    };
    int n_choices = sizeof(choices) / sizeof(choices[0]);

    while (1) {
        clear();
        int startx = (COLS - 40) / 2;
        int starty = (LINES - 12) / 2;
        WINDOW *menu_win = newwin(12, 40, starty, startx);
        box(menu_win, 0, 0);

        wattron(menu_win, A_BOLD | COLOR_PAIR(1));
        print_center(menu_win, 1, 0, 40, "Phonebook Menu");
        wattroff(menu_win, A_BOLD | COLOR_PAIR(1));

        for (int i = 0; i < n_choices; i++) {
            if (i == highlight) {
                wattron(menu_win, A_REVERSE);
                animate_highlight(menu_win, i + 3, 2, 36, (char *)choices[i]);
                wattroff(menu_win, A_REVERSE);
            } else {
                mvwprintw(menu_win, i + 3, 2, "%s", choices[i]);
            }
        }

        wrefresh(menu_win);
	// 사용자가 입력한 방향키의 방향대로 메뉴를 선택하고 텍스트에 하이라이트
        int c = getch();
        switch (c) {
            case KEY_UP:
                highlight = (highlight == 0) ? n_choices - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == n_choices - 1) ? 0 : highlight + 1;
                break;
            case 10:  // Enter key 입력시 명령 수행
                delwin(menu_win);
                switch (highlight) {
                    case 0:
                        list_entries();
                        break;
                    case 1:
                        add_entry();
                        break;
                    case 2:
                        delete_entry();
                        break;
                    case 3:
                        search_entry();
                        break;
                    case 4:
                        return;
                }
                break;
        }
    }
}

// 프로그램 시작
int main() {
    initscr();  // ncurses 초기화
    noecho();  // 사용자 입력을 화면에 표시하지 않는다.
    cbreak();  // 입력 즉시 처리
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    load_data("data.txt");
    menu();
    attroff(COLOR_PAIR(1));

    endwin();  // ncurses 종료
    return 0;
}
