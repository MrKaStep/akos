#define _XOPEN_SOURCE 1000

#include <linux/limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define E_OK     0
#define E_LSTAT  1
#define E_READ   2
#define E_WRITE  3
#define E_ARGS   4
#define E_MALLOC 5

struct Node_s
{
    struct Node_s* left;
    struct Node_s* right;
    int prior;
    ino_t key;
};

typedef struct Node_s Node;

void split(Node* root, Node** left, Node** right, ino_t key)
{
    if(root == NULL)
    {
        *left = *right = NULL;
        return;
    }
    if(root->key < key)
    {
        split(root->right, &(root->right), right, key);
        *left = root;
    }
    else {
        split(root->left, left, &(root->left), key);
        *right = root;
    }
}

void merge(Node* left, Node* right, Node** root)
{
    if(left == NULL)
    {
        *root = right;
        return;
    }
    if(right == NULL)
    {
        *root = left;
        return;
    }
    if(left->prior < right->prior)
    {
        merge(left->right, right, &(left->right));
        *root = left;
    }
    else
    {
        merge(left, right->left, &(right->left));
        *root = right;
    }
}

void insert(Node** root, Node* v)
{
    if(*root == NULL)
    {
        *root = v;
        return;
    }
    if(v->prior < (*root)->prior)
    {
        split(*root, &(v->left), &(v->right), v->key);
        *root = v;
        return;
    }
    if((*root)->key < v->key)
    {
        insert(&((*root)->right), v);
    }
    else
    {
        insert(&((*root)->left), v);
    }
}

int find(Node* root, ino_t key)
{
    if(root == NULL)
    {
        return 0;
    }
    if(root->key == key)
    {
        return 1;
    }
    if(key < root->key)
    {
        return find(root->left, key);
    }
    return find(root->right, key);
}

void init_node(Node* v, ino_t key)
{
    v->left = v->right = NULL;
    v->key = key;
    v->prior = rand();
}

void destroy_tree(Node* v)
{
    if(v == NULL)
        return;
    destroy_tree(v->left);
    destroy_tree(v->right);
    free(v);
}

Node* tree_root = NULL;

int copy_regular(char* from, char* dest)
{
    int s, t;
    char* buf = malloc(4096);
    int sz;
    if(buf == NULL)
    {
        return E_MALLOC;
    }
    s = open(from, O_RDONLY);
    if(s == -1)
    {
        perror("Regular file read failed");
        free(buf);
        return E_READ;
    }
    t = open(dest, O_WRONLY | O_CREAT);
    if(t == -1)
    {
        perror("Regular file write failed");
        free(buf);
        close(s);
        return E_WRITE;
    }
    while((sz = read(s, buf, 4096)) != 0)
    {
        int written = 0;
        if(sz == -1)
        {
            free(buf);
            perror("Regular file read failed");
            return E_READ;
        }
        while(written != sz)
        {
            int add;
            add = write(t, buf + written, sz - written);
            if(add == -1)
            {
                free(buf);
                close(s);
                close(t);
                perror("Regular file write failed");
                return E_WRITE;
            }
            written += add;
        }
    }
    free(buf);
    close(s);
    close(t);
    return E_OK;
}

int copy_symlink(char* from, char* dest)
{
    char buf[PATH_MAX];
    if(readlink(from, buf, PATH_MAX * sizeof(char)) == -1)
    {
        perror("Symbolic link read failed");
        return E_READ;
    }
    if(symlink(buf, dest) == -1)
    {
        perror("Symbolic link create failed");
        return E_WRITE;
    }
    return E_OK;
}

int copy(char* from, char* dest);

int copy_dir(char* from, char* dest)
{
    DIR* s;
    struct stat from_s, dest_s;
    struct dirent* e;
    int dest_l = strlen(dest);
    int from_l = strlen(from);
    Node* v;
    s = opendir(from);
    if(s == NULL)
    {
        perror("Directory open failed");
        return E_READ;
    }
    if(lstat(from, &from_s) == -1)
    {
        closedir(s);
        perror("Directory stat failed");
        return E_LSTAT;
    }
    if(mkdir(dest, from_s.st_mode) == -1)
    {
        closedir(s);
        perror("Directory create failed");
        return E_WRITE;
    }
    if(lstat(dest, &dest_s) == -1)
    {
        closedir(s);
        perror("Directory stat failed");
        return E_LSTAT;
    }
    v = malloc(sizeof(Node));
    if(v == NULL)
    {
        return E_MALLOC;
    }
    init_node(v, dest_s.st_ino);
    insert(&tree_root, v);
    while((e = readdir(s)) != NULL)
    {
        int err;
        if(strcmp(".", e->d_name) == 0 || strcmp("..", e->d_name) == 0)
            continue;
        from[from_l] = '/';
        dest[dest_l] = '/';
        strcpy(from + from_l + 1, e->d_name);
        strcpy(dest + dest_l + 1, e->d_name);
        if((err = copy(from, dest)))
        {
            closedir(s);
            return err;
        }
        from[from_l] = '\0';
        dest[dest_l] = '\0';
    }
    closedir(s);
    return E_OK;
}

int copy_fifo(char* from, char* dest)
{
    if(mkfifo(dest, 0777) == -1)
    {
        perror("Create FIFO failed");
        return E_WRITE;
    }
    return E_OK;
}



void copy_mode(const char* source, const char* dest)
{
    struct stat st;
    lstat(source, &st);
    chmod(dest, st.st_mode);
}

int copy(char* from, char* dest)
{
    struct stat s, t;
    int err;
    if(lstat(from, &s) == -1)
    {
        puts(from);
        puts(dest);
        perror("Lstat call failed");
        return E_LSTAT;
    }
    if(find(tree_root, s.st_ino))
    {
        return E_OK;
    }
    if(S_ISREG(s.st_mode))
        err = copy_regular(from, dest);
    else if(S_ISDIR(s.st_mode))
        err = copy_dir(from, dest);
    else if(S_ISLNK(s.st_mode))
        err = copy_symlink(from, dest);
    else if(S_ISFIFO(s.st_mode))
        err = copy_fifo(from, dest);
    else
        return E_OK;
    if(err)
    {
        return err;
    }
    copy_mode(from, dest);
    if(lstat(dest, &t) == -1)
    {
        perror("New file stat failed");
        return E_LSTAT;
    }
    return E_OK;
}




int main(int argc, char const *argv[])
{
    char from[PATH_MAX + 256], dest[PATH_MAX + 256];
    int from_l, dest_l;
    if(argc != 3)
    {
        fprintf(stderr, "\tIncorrect number of argumets: %d, expected 3\n", argc);
        return E_ARGS;
    }
    strcpy(from, argv[1]);
    strcpy(dest, argv[2]);
    from_l = strlen(from);
    dest_l = strlen(dest);
    if(dest_l != 0 && dest[dest_l - 1] == '/')
    {
        int i = from_l;
        while(i >= 0 && from[i] != '/')
            --i;
        ++i;
        strcpy(dest + dest_l, from + i);
    }
    copy(from, dest);
    destroy_tree(tree_root);
    return 0;
}