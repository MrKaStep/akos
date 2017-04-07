#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
struct list_elem {
    char *s;
    struct list_elem *next;
};

struct list_elem * dup_handler(struct list_elem *v) {
    struct list_elem *prev = NULL;
    struct list_elem *new_st = v;

    while (v != NULL) {
        int max_cnt_dup = 0;
        int i, j, k;
        for (i = 0; v->s[i] != 0; ++i) {
            if (v->s[i] != ' ' && (i == 0 || v->s[i - 1] == ' ')) {
                int cnt_dup = 0;
                for (j = 0; j < i; ++j) {
                    if (v->s[i] != ' ' && (j == 0 || (v->s[j - 1] == ' '))) {
                        int flag = 1;
                        for (k = 0; v->s[i + k] != ' ' && v->s[i + k] != 0; ++k) {
                            if (v->s[j + k] != v->s[i + k]) {
                                flag = 0;
                                break;
                            }
                        }
                        if (v->s[j + k] != ' ' && v->s[j + k] != 0)
                            flag = 0;
                        if (flag) {
                            ++cnt_dup;
                        }
                    }
                }
                if (cnt_dup != 0) {
                    for (j = i; v->s[j] != ' ' && v->s[j] != 0; ++j);
                    for (k = j; v->s[k] != 0; ++k) {
                        v->s[i + k - j] = v->s[k];
                    }
                    v->s[i + k - j] = 0;
                } else {
                    for (j = i + 1; v->s[j] != 0; ++j) {
                        if (v->s[i] != ' ' && v->s[j - 1] == ' ') {
                            int flag = 1;
                            for (k = 0; v->s[j + k] != ' ' && v->s[j + k] != 0; ++k) {
                                if (v->s[j + k] != v->s[i + k]) {
                                    flag = 0;
                                    break;
                                }
                            }
                            if (v->s[i + k] != ' ' && v->s[i + k] != 0)
                                flag = 0;
                            if (flag) {
                                ++cnt_dup;
                            }
                        }
                    }
                    if (cnt_dup > max_cnt_dup)
                        max_cnt_dup = cnt_dup;
                }
            }
        }
        if (max_cnt_dup > 0) {
            char *dup_s = (char *)malloc(sizeof(char) * 64);
            int len;
            int razryad = 100;
            int flag;
            struct list_elem *new_elem;
            strcpy(dup_s, "dublicates - ");
            len = (int)strlen(dup_s);
            flag = 0;
            while (razryad > 0) {
                if ((max_cnt_dup % (10 * razryad)) / razryad > 0 || flag != 0) {
                    flag = 1;
                    dup_s[len++] = (char)((int)'0' + (max_cnt_dup % (10 * razryad)) / razryad);
                }
                razryad /= 10;
            }
            dup_s[len] = 0;
            new_elem = (struct list_elem *)malloc(sizeof(struct list_elem));
            new_elem->next = v;
            new_elem->s = dup_s;
            if (prev == NULL) {
                new_st = new_elem;
            } else {
                prev->next = new_elem;
            }
        }
        prev = v;
        v = v->next;
    }
    return new_st;
}

void tester() {
    struct list_elem *v = (struct list_elem *)malloc(sizeof(struct list_elem));
    struct list_elem *v1 = (struct list_elem *)malloc(sizeof(struct list_elem));
    char *s = (char *)malloc(sizeof(char) * 64);
    char *s1 = (char *)malloc(sizeof(char) * 64);
    strcpy(s,"aaa aa aa aa aa aaa");
    strcpy(s1,"abc abc abd");
    v->next = v1;
    v->s = s;
    v1->next = NULL;
    v1->s = s1;
    v = dup_handler(v);
    while (v != NULL) {
        printf("%s\n", v->s);
        v = v->next;
    }
    return;
}

int main() {
    tester();

    return 0;
}
