#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>

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
    String key;
    String value;
    /* int has_value; */
} Attr;

typedef struct {
    Attr *items;
    int len;
    int cap;
} AttrList;
AttrList attrlist_new(int cap) {
    AttrList list;
    list.len = 0;
    list.cap = cap;
    list.items = malloc(cap * sizeof(Attr));
    if (!list.items) {
        perror("malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return list;
}
void attrlist_push(AttrList *list, Attr item) {
    if (list->len >= list->cap) {
        int cap = list->cap + 10;  // Genius algorithm
        Attr *new_items = realloc(list->items, cap * sizeof(Attr));
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

enum ParseStatus {
    OK,
    UNEXPECTED_TAG_START,
    UNEXPECTED_TAG_END,
    UNEXPECTED_EOF,
    UNEXPECTED_WHITESPACE_IN_TAG,
};

typedef struct {
    String name;
    int is_closing;
    AttrList attrs;
} TagToken;

enum ParseStatus parse_tag_attributes(AttrList *attrs, char *token) {
    Attr attr;

    attr.key.items = "missing";
    attr.key.len = 4;
    attr.value.items = "none";
    attr.value.len = 4;

    attrlist_push(attrs, attr);

    return OK;
}

enum ParseStatus parse_tag_token(TagToken *tag, char *token) {
    tag->name = string_new(10);

    if (iswspace(token[0])) {
        return UNEXPECTED_WHITESPACE_IN_TAG;
    }

    tag->is_closing = 0;
    if (token[0] == '/') {
        tag->is_closing = 1;
        token++;

        if (iswspace(token[0])) {
            return UNEXPECTED_WHITESPACE_IN_TAG;
        }
    }

    for (; token[0] != '\0'; token++) {
        if (iswspace(*token)) {
            break;
        }
        string_push(&tag->name, *token);
    }

    AttrList attrs = attrlist_new(0);
    enum ParseStatus err = parse_tag_attributes(&attrs, token);
    if (err) {
        return err;
    }

    tag->attrs = attrs;

    return OK;
}

typedef struct {
    enum TokenType {
        TEXT,
        TAG,
    } type;
    union {
        String text;
        TagToken tag;
    } data;
} Token;

typedef struct {
    Token *items;
    int len;
    int cap;
} TokenList;
TokenList tokenlist_new(int cap) {
    TokenList list;
    list.len = 0;
    list.cap = cap;
    list.items = malloc(cap * sizeof(Token));
    if (!list.items) {
        perror("malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return list;
}
void tokenlist_push(TokenList *list, Token item) {
    if (list->len >= list->cap) {
        int cap = list->cap + 10;  // Genius algorithm
        Token *new_items = realloc(list->items, cap * sizeof(Token));
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

int str_trim_len(char *str) {
    char *start = NULL;
    char *end = NULL;

    for (char *i = str; *i != '\0'; i++) {
        if (start == NULL) {
            if (!iswspace(*i)) {
                start = i;
            }
        } else {
            if (!iswspace(*i)) {
                end = i;
            }
        }
    }

    if (start == NULL || end == NULL) {
        return 0;
    }
    return end - start;
}

enum ParseStatus parse_tokens(TokenList *tokens) {
    String current_token = string_new(10);
    int is_tag = 0;

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '<') {
            if (is_tag) {
                return UNEXPECTED_TAG_START;
            }
            is_tag = 1;
            if (str_trim_len(current_token.items) > 0) {
                Token token = {
                    .type = TEXT,
                    .data = {.text = current_token},
                };
                tokenlist_push(tokens, token);
            }
            current_token = string_new(10);
        } else if (ch == '>') {
            if (!is_tag) {
                return UNEXPECTED_TAG_END;
            }
            is_tag = 0;
            if (current_token.len > 0) {
                TagToken tag;
                enum ParseStatus err =
                    parse_tag_token(&tag, current_token.items);
                if (err) {
                    return err;
                }
                Token token = {
                    .type = TAG,
                    .data = {.tag = tag},
                };
                tokenlist_push(tokens, token);
                current_token = string_new(10);
            }
        } else {
            string_push(&current_token, ch);
        }
    }

    if (str_trim_len(current_token.items) > 0) {
        if (is_tag) {
            return UNEXPECTED_EOF;
        }
        Token token = {
            .type = TEXT,
            .data = {.text = current_token},
        };
        tokenlist_push(tokens, token);
    }

    return 0;
}

int main(int argc, char **argv) {
    TokenList tokenlist = tokenlist_new(10);
    enum ParseStatus err = parse_tokens(&tokenlist);
    if (err) {
        fprintf(stderr, "Parse error: %d\n", err);
        exit(1);
    }

    for (int i = 0; i < tokenlist.len; i++) {
        Token token = tokenlist.items[i];

        if (token.type == TEXT) {
            printf("%s\n", token.data.text.items);
        } else {
            TagToken tag = token.data.tag;
            if (tag.is_closing) {
                printf("(/)\n");
            } else {
                printf("()\n");
            }
            String name = tag.name;
            printf("<%s>\n", name.items);
            for (int i = 0; i < tag.attrs.len; i++) {
                Attr attr = tag.attrs.items[i];
                printf("[%s]", attr.key.items);
                printf("= [%s]\n", attr.value.items);
            }
        }

        printf("---------------------\n");
    }
}
