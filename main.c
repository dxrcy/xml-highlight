#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

typedef struct {
    char *items;
    unsigned int len;
    unsigned int cap;
} String;
String string_new(unsigned int cap) {
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

String string_from(char *str) {
    String list = string_new(10);
    for (; str[0] != '\0'; str++) {
        string_push(&list, str[0]);
    }
    return list;
}

typedef struct {
    String key;
    String value;
    /* int has_value; */
} Attr;

typedef struct {
    Attr *items;
    unsigned int len;
    unsigned int cap;
} AttrList;
AttrList attrlist_new(unsigned int cap) {
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

typedef enum {
    OK,
    UNEXPECTED_TAG_START,
    UNEXPECTED_TAG_END,
    UNEXPECTED_EOF,
    UNEXPECTED_WHITESPACE_IN_TAG,
    UNEXPECTED_ATTRIBUTE_DELIMITER,
    UNEXPECTED_TAG_END_FOR_ATTRIBUTE,
    UNEXPECTED_NON_QUOTE_FOR_ATTRIBUTE,
    UNEXPECTED_XML_PROLOG,
    UNEXPECTED_OPENING_TAG,
    UNEXPECTED_CLOSING_TAG,
    MISMATCHED_TAGS,
    UNEXPECTED_EOF_FOR_CLOSING_TAG,
    UNEXPECTED_END_COMMENT,
    ENTITY_TOO_LONG,
    INVALID_ENTITY,
    UNEXPECTED_TEXT_END_FOR_ENTITY,
} ParseStatus;

char *parsestatus_msg(ParseStatus status) {
    switch (status) {
        case OK:
            return "";
        case UNEXPECTED_TAG_START:
            return "Unexpected `<`";
        case UNEXPECTED_TAG_END:
            return "Unexpected `>`";
        case UNEXPECTED_EOF:
            return "Unexpected end of file. Expected `>`";
        case UNEXPECTED_WHITESPACE_IN_TAG:
            return "Unexpected whitespace before tag name";
        case UNEXPECTED_ATTRIBUTE_DELIMITER:
            return "Unexpected `=`. Expected start of attribute key";
        case UNEXPECTED_TAG_END_FOR_ATTRIBUTE:
            return "Unexpected end of tag. Expected `'` or `\"`";
        case UNEXPECTED_NON_QUOTE_FOR_ATTRIBUTE:
            return "Unexpected non-quote character in attribute value. "
                   "Expected `'` or `\"`";
        case UNEXPECTED_XML_PROLOG:
            return "Unexpected XML prolog. Prolog must occur at beginning of "
                   "file";
        case UNEXPECTED_OPENING_TAG:
            return "Unexpected opening tag. Expected end of file. Only one "
                   "root node is allowed.";
        case UNEXPECTED_CLOSING_TAG:
            return "Unexpected closing tag. Expected end of file.";
        case MISMATCHED_TAGS:
            return "Mismatched tags. Opening tag does not match closing tag.";
        case UNEXPECTED_EOF_FOR_CLOSING_TAG:
            return "Unexpected end of file. Expected matching closing tag.";
        case UNEXPECTED_END_COMMENT:
            return "Unexpected comment ending identifier.";
        case ENTITY_TOO_LONG:
            return "XML entity code is too long.";
        case INVALID_ENTITY:
            return "Invalid XML entity code.";
        case UNEXPECTED_TEXT_END_FOR_ENTITY:
            return "Unexpected end of text. Expected end of entity code.";
        default:
            return "Unknown error";
    }
}

typedef struct {
    String name;
    int is_closing;
    AttrList attrs;
} TagToken;

ParseStatus parse_tag_attributes(AttrList *attrs, char *token) {
    String key_opt = string_new(10);
    String value_opt = string_new(10);
    int is_value_active = 0;
    int was_whitespace = 0;
    char quote_char = '\0';

    for (; token[0] != '\0'; token++) {
        char ch = token[0];
        if (key_opt.len == 0) {
            if (iswspace(ch)) {
                continue;
            }
            if (ch == '=') {
                return UNEXPECTED_ATTRIBUTE_DELIMITER;
            }
            string_push(&key_opt, token[0]);
        } else if (!is_value_active) {
            if (iswspace(ch)) {
                was_whitespace = 1;
                continue;
            }
            if (was_whitespace && ch != '=') {
                if (key_opt.len == 0) {
                    fprintf(stderr, "this string shouldn't be empty.");
                    exit(1);
                }
                Attr attr = {
                    .key = key_opt,
                    .value = value_opt,
                };
                attrlist_push(attrs, attr);
            }

            if (ch != '=') {
                string_push(&key_opt, token[0]);
                continue;
            }

            token++;
            for (; iswspace(token[0]); token++)
                ;

            char quote = token[0];
            if (quote == '\0') {
                return UNEXPECTED_TAG_END_FOR_ATTRIBUTE;
            }
            if (quote != '"' && quote != '\'') {
                return UNEXPECTED_NON_QUOTE_FOR_ATTRIBUTE;
            }

            is_value_active = 1;
            quote_char = quote;
        } else {
            if (ch != quote_char) {
                string_push(&value_opt, ch);
                continue;
            }
            if (key_opt.len == 0) {
                fprintf(stderr, "this string shouldn't be empty.");
                exit(1);
            }
            Attr attr = {
                .key = key_opt,
                .value = value_opt,
            };
            attrlist_push(attrs, attr);
            key_opt = string_new(10);
            value_opt = string_new(10);
            is_value_active = 0;
        }
    }

    if (key_opt.len > 0) {
        if (key_opt.items[0] == '?') {
            if (key_opt.items[1] != '\0') {
                fprintf(stderr,
                        "this error should have been handled better :(");
                exit(1);
            }
        } else {
            String a = string_new(5);
            a.items = "EMPTY";
            Attr attr = {
                .key = key_opt,
                .value = a,
            };
            attrlist_push(attrs, attr);
        }
    }
    if (value_opt.len > 0) {
        return UNEXPECTED_NON_QUOTE_FOR_ATTRIBUTE;
    }

    return OK;
}

ParseStatus parse_tag_token(TagToken *tag, char *token) {
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
        char ch = token[0];
        if (iswspace(ch)) {
            break;
        }
        string_push(&tag->name, ch);
    }

    AttrList attrs = attrlist_new(0);
    ParseStatus err = parse_tag_attributes(&attrs, token);
    if (err) {
        return err;
    }

    tag->attrs = attrs;

    return OK;
}

typedef struct {
    enum TokenType {
        TOKEN_TEXT,
        TOKEN_TAG,
    } type;
    union {
        String text;
        TagToken tag;
    } data;
} Token;

typedef struct {
    Token *items;
    unsigned int len;
    unsigned int cap;
} TokenList;
TokenList tokenlist_new(unsigned int cap) {
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

int str_starts_with(char *str, char *pattern) {
    char ch;
    for (int i = 0; (ch = pattern[i]) != '\0'; i++) {
        if (ch != str[i]) {
            return 0;
        }
    }
    return 1;
}

#define READ_BUFSIZE 8

ParseStatus parse_tokens(TokenList *tokens) {
    String current_token = string_new(10);
    int is_tag = 0;
    int is_comment = 0;
    char quote = '\0';

    char read_buf[READ_BUFSIZE];
    int skip_count = 0;
    int eof_index = -1;

    while (eof_index != 0) {
        if (eof_index > 0) {
            eof_index--;
        }

        for (int i = 0; i < READ_BUFSIZE; i++) {
            if (i < READ_BUFSIZE - skip_count - 1) {
                read_buf[i] = read_buf[i + skip_count + 1];
            } else if (eof_index < 0) {
                char next_ch = getchar();
                if (next_ch == EOF) {
                    eof_index = READ_BUFSIZE - 1;
                    next_ch = '\0';
                }
                read_buf[i] = next_ch;
            }
        }

        skip_count = 0;
        char ch = read_buf[0];

        if (is_tag && (ch == '"' || ch == '\'')) {
            if (ch == quote) {
                quote = '\0';
            } else {
                quote = ch;
            }
            string_push(&current_token, ch);
            continue;
        }

        if (quote == '\0') {
            if (str_starts_with(read_buf, "-->")) {
                if (!is_comment) {
                    return UNEXPECTED_END_COMMENT;
                    string_push(&current_token, ch);
                    continue;
                }
                is_comment = 0;
                skip_count = 2;
                continue;
            }
            if (is_comment) {
                continue;
            }
            if (str_starts_with(read_buf, "<!--")) {
                is_comment = 1;
                skip_count = 3;
                continue;
            }

            if (ch == '<') {
                if (is_tag) {
                    return UNEXPECTED_TAG_START;
                }
                is_tag = 1;
                if (str_trim_len(current_token.items) > 0) {
                    Token token = {
                        .type = TOKEN_TEXT,
                        .data = {.text = current_token},
                    };
                    tokenlist_push(tokens, token);
                }
                current_token = string_new(10);
                continue;
            }
            if (ch == '>') {
                if (!is_tag) {
                    return UNEXPECTED_TAG_END;
                }
                is_tag = 0;
                if (current_token.len > 0) {
                    TagToken tag;
                    ParseStatus err =
                        parse_tag_token(&tag, current_token.items);
                    if (err) {
                        return err;
                    }
                    Token token = {
                        .type = TOKEN_TAG,
                        .data = {.tag = tag},
                    };
                    tokenlist_push(tokens, token);
                    current_token = string_new(10);
                }
                continue;
            }
        }

        string_push(&current_token, ch);
    }

    if (str_trim_len(current_token.items) > 0) {
        if (is_tag) {
            return UNEXPECTED_EOF;
        }
        Token token = {
            .type = TOKEN_TEXT,
            .data = {.text = current_token},
        };
        tokenlist_push(tokens, token);
    }

    return 0;
}

typedef struct {
    struct Node *items;
    unsigned int len;
    unsigned int cap;
} NodeList;

typedef struct Element {
    String tag_name;
    AttrList attrs;
    NodeList children;
} Element;

typedef struct Node {
    enum {
        NODE_TEXT,
        NODE_ELEMENT,
    } type;
    union {
        String text;
        struct Element element;
    } data;
} Node;

NodeList nodelist_new(unsigned int cap) {
    NodeList list;
    list.len = 0;
    list.cap = cap;
    list.items = malloc(cap * sizeof(Node));
    if (!list.items) {
        perror("malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return list;
}
void nodelist_push(NodeList *list, Node item) {
    if (list->len >= list->cap) {
        int cap = list->cap + 10;  // Genius algorithm
        Node *new_items = realloc(list->items, cap * sizeof(Node));
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

#define ENTITY_BUFSIZE 6

ParseStatus replace_text_entities(String *output, String text) {
    for (int i = 0; i < text.len; i++) {
        char ch = text.items[i];

        if (ch == '&') {
            char buf[ENTITY_BUFSIZE] = "";
            int start = i;
            while (i++) {
                if (i - start >= ENTITY_BUFSIZE) {
                    return ENTITY_TOO_LONG;
                };
                if (i >= text.len) {
                    return UNEXPECTED_TEXT_END_FOR_ENTITY;
                }
                char ch2 = text.items[i];
                if (ch2 == ';') {
                    break;
                }
                buf[i - start - 1] = ch2;
            }

            char replace;
            if (!strcmp(buf, "lt")) {
                replace = '<';
            } else if (!strcmp(buf, "gt")) {
                replace = '>';
            } else if (!strcmp(buf, "amp")) {
                replace = '&';
            } else if (!strcmp(buf, "quot")) {
                replace = '"';
            } else if (!strcmp(buf, "apos")) {
                replace = '\'';
            } else {
                return INVALID_ENTITY;
            }
            string_push(output, replace);
            continue;
        }

        string_push(output, ch);
    }

    return OK;
}

ParseStatus parse_node_tree_part(NodeList *nodes, TokenList *tokens,
                                 unsigned int depth, char *current_tag_name) {
    while (tokens->len > 0) {
        Token token = tokens->items[0];
        tokens->items++;
        tokens->len--;
        tokens->cap--;

        if (token.type == TOKEN_TEXT) {
            String text = string_new(10);
            ParseStatus err = replace_text_entities(&text, token.data.text);
            if (err) {
                return err;
            }

            Node node = {.type = NODE_TEXT, .data = {.text = text}};
            nodelist_push(nodes, node);
            continue;
        }

        TagToken tag = token.data.tag;

        if (!strcmp(tag.name.items, "?xml")) {
            return UNEXPECTED_XML_PROLOG;
        }

        if (tag.is_closing) {
            if (depth == 0) {
                return UNEXPECTED_CLOSING_TAG;
            }
            if (current_tag_name != NULL &&
                strcmp(current_tag_name, tag.name.items)) {
                return MISMATCHED_TAGS;
            }
            return OK;
        }

        if (depth == 0 && nodes->len > 0) {
            return UNEXPECTED_OPENING_TAG;
        }

        NodeList children = nodelist_new(10);
        ParseStatus err =
            parse_node_tree_part(&children, tokens, depth + 1, tag.name.items);
        if (err) {
            return err;
        }

        Node node = {.type = NODE_ELEMENT,
                     .data = {.element = {
                                  .tag_name = tag.name,
                                  .attrs = tag.attrs,
                                  .children = children,
                              }}};
        nodelist_push(nodes, node);
    }

    return OK;
}

ParseStatus parse_node_tree(NodeList *nodes, TokenList tokens) {
    if (tokens.len > 0) {
        Token token = tokens.items[0];
        if (token.type == TOKEN_TAG &&
            !strcmp(token.data.tag.name.items, "?xml")) {
            tokens.items++;
            tokens.len--;
            tokens.cap--;
        }
    }

    return parse_node_tree_part(nodes, &tokens, 0, NULL);
}

void print_node_tree(NodeList *nodes, unsigned int depth, int single_line) {
    for (int i = 0; i < nodes->len; i++) {
        Node node = nodes->items[i];

        if (!single_line) {
            for (int j = 0; j < depth; j++) {
                printf("    ");
            }
        }
        if (node.type == NODE_TEXT) {
            printf("\x1b[0m%s\x1b[0m", node.data.text.items);
            if (!single_line) {
                printf("\n");
            }
            continue;
        }

        Element element = node.data.element;

        int single_line = 0;
        if (element.children.len <= 1) {
            single_line = 1;
        }

        printf("\x1b[2;37m<");
        printf("\x1b[0;36m%s\x1b[0m", element.tag_name.items);
        for (int j = 0; j < element.attrs.len; j++) {
            Attr attr = element.attrs.items[j];
            printf(" \x1b[35m%s", attr.key.items);
            printf("\x1b[2;37m=\"");
            printf("\x1b[0;33m%s", attr.value.items);
            printf("\x1b[2;37m\"\x1b[0m");
        }
        printf("\x1b[2;37m>\x1b[0m");
        if (!single_line) {
            printf("\n");
        }

        print_node_tree(&element.children, depth + 1, single_line);

        if (!single_line) {
            for (int j = 0; j < depth; j++) {
                printf("    ");
            }
        }

        printf("\x1b[2;37m</");
        printf("\x1b[0;36m%s\x1b[0m", element.tag_name.items);
        printf("\x1b[2;37m>\x1b[0m\n");
    }
}

int main(int argc, char **argv) {
    ParseStatus err;

    TokenList tokenlist = tokenlist_new(10);
    err = parse_tokens(&tokenlist);
    if (err) {
        fprintf(stderr, "Parse error (1): %d\n%s\n", err, parsestatus_msg(err));
        exit(1);
    }

    /* for (int i = 0; i < tokenlist.len; i++) { */
    /*     Token token = tokenlist.items[i]; */
    /*  */
    /*     if (token.type == TOKEN_TEXT) { */
    /*         printf("%s\n", token.data.text.items); */
    /*     } else { */
    /*         TagToken tag = token.data.tag; */
    /*         if (tag.is_closing) { */
    /*             printf("(/)\n"); */
    /*         } else { */
    /*             printf("()\n"); */
    /*         } */
    /*         String name = tag.name; */
    /*         printf("<%s>\n", name.items); */
    /*         for (int i = 0; i < tag.attrs.len; i++) { */
    /*             Attr attr = tag.attrs.items[i]; */
    /*             printf("[%s]", attr.key.items); */
    /*             printf("=[%s]\n", attr.value.items); */
    /*         } */
    /*     } */
    /*  */
    /*     printf("---------------------\n"); */
    /* } */

    NodeList nodes = nodelist_new(10);
    err = parse_node_tree(&nodes, tokenlist);
    if (err) {
        fprintf(stderr, "Parse error (2): %d\n%s\n", err, parsestatus_msg(err));
        exit(1);
    }

    print_node_tree(&nodes, 0, 0);
}
