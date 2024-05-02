#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *items;
    int len;
    int cap;
} String;
String string_new(int cap) {
    String list;
    list.len = 0;
    list.cap = cap;
    list.items = malloc(cap * sizeof(char));
    if (!list.items) {
        perror("malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return list;
}
void string_push(String *list, char item) {
    if (list->len >= list->cap) {
        int cap = list->cap + 10;  // Genius algorithm
        char *new_items = realloc(list->items, cap * sizeof(char));
        if (!new_items) {
            perror("realloc failed.\n");
            exit(EXIT_FAILURE);
        }
        list->cap = cap;
        list->items = new_items;
    }
    list->items[list->len] = item;
    list->items[list->len + 1] = '\0';
    list->len++;
}

typedef struct {
    String text;
} Token;

typedef struct {
    String *items;
    int len;
    int cap;
} TokenList;
TokenList tokenlist_new(int cap) {
    TokenList list;
    list.len = 0;
    list.cap = cap;
    list.items = malloc(cap * sizeof(String));
    if (!list.items) {
        perror("malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return list;
}
void tokenlist_push(TokenList *list, String item) {
    if (list->len >= list->cap) {
        int cap = list->cap + 10;  // Genius algorithm
        String *new_items = realloc(list->items, cap * sizeof(String));
        if (!new_items) {
            perror("realloc failed.\n");
            exit(EXIT_FAILURE);
        }
        list->cap = cap;
        list->items = new_items;
    }
    list->items[list->len] = item;
    list->len++;
}

TokenList parse_tokens() {
    TokenList tokenlist = tokenlist_new(10);
    String string = string_new(10);

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '\n') {
            if (string.len > 0) {
                tokenlist_push(&tokenlist, string);
                string = string_new(10);
            }
        } else {
            string_push(&string, ch);
        }
    }

    if (string.len > 0) {
        tokenlist_push(&tokenlist, string);
    }

    return tokenlist;
}

int main(int argc, char **argv) {
    TokenList tokenlist = parse_tokens();

    for (int i = 0; i < tokenlist.len; i++) {
        String string = tokenlist.items[i];

        for (int i = 0; i < string.len; i++) {
            printf("%c", string.items[i]);
        }

        printf("\n---------------------\n");
    }
}
