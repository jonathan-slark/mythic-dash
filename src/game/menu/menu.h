// clang-format Language: C
#pragma once

// --- Types ---

typedef enum { MENU_CONTEXT_TITLE, MENU_CONTEXT_INGAME, MENU_CONTEXT_BOTH } menu_Context;

// --- Menu functions ---

void menu_open(menu_Context context);
void menu_close(void);
void menu_update(void);
void menu_draw(void);
void menu_back(void);
