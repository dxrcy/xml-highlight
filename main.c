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
    Token *items;
    int len;
    int cap;
} TokenList;

String parse_tokens() {
    String string = string_new(10);
    int ch;
    while ((ch = getchar()) != EOF) {
        printf("%c", ch);
        string_push(&string, ch);
    }
    return string;
}

int main(int argc, char **argv) {
    String string = parse_tokens();

    char item;
    for (int i = 0; (item = string.items[i]); i++) {
        printf("%c.", item);
    }
}
