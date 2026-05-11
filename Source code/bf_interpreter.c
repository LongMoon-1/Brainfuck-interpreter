#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define chdir _chdir
    #define GETCWD _getcwd
#else
    #include <unistd.h>
    #define GETCWD getcwd
#endif

#define MEMORY_SIZE 65536
#define PATH_MAX_LEN 1024

// BF 指令
#define BF_INC_PTR    '>'
#define BF_DEC_PTR    '<'
#define BF_INC_VAL    '+'
#define BF_DEC_VAL    '-'
#define BF_OUTPUT     '.'
#define BF_INPUT      ','
#define BF_LOOP_START '['
#define BF_LOOP_END   ']'

// 设置控制台为 UTF-8
void set_console_utf8() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    #endif
}

// 显示帮助
void show_help() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    BF 语言解释器 - 帮助                      ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  BF指令说明：                                                ║\n");
    printf("║    >   指针右移一位                                          ║\n");
    printf("║    <   指针左移一位                                          ║\n");
    printf("║    +   当前指针指向的值 +1                                   ║\n");
    printf("║    -   当前指针指向的值 -1                                   ║\n");
    printf("║    .   输出当前指针指向的值（作为ASCII字符）                 ║\n");
    printf("║    ,   从输入读取一个字符，存入当前指针位置                  ║\n");
    printf("║    [   如果当前值为0，跳转到对应的 ] 之后                    ║\n");
    printf("║    ]   如果当前值不为0，跳回对应的 [ 之后                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  内置命令：                                                  ║\n");
    printf("║    help       - 显示此帮助                                   ║\n");
    printf("║    clear      - 清屏                                         ║\n");
    printf("║    exit       - 退出解释器                                   ║\n");
    printf("║    cd 路径    - 切换目录                                     ║\n");
    printf("║    pwd        - 显示当前目录                                 ║\n");
    printf("║    dir / ls   - 列出当前目录文件                             ║\n");
    printf("║    bf 文件.bf - 执行指定的 .bf 文件                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// 清屏
void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// 列出目录
void list_files() {
    #ifdef _WIN32
        system("dir");
    #else
        system("ls -la");
    #endif
}

// 转换路径
void normalize_path(const char *src, char *dst, int dst_size) {
    char temp[PATH_MAX_LEN];
    int i;
    
    strncpy(temp, src, PATH_MAX_LEN - 1);
    temp[PATH_MAX_LEN - 1] = '\0';
    
    // 将正斜杠替换为反斜杠
    for (i = 0; temp[i]; i++) {
        if (temp[i] == '/') temp[i] = '\\';
    }
    
    // 处理 MSYS2 风格 /d/xxx -> D:\xxx
    if (strlen(temp) >= 3 && temp[0] == '\\' && isalpha((unsigned char)temp[1]) && temp[2] == '\\') {
        char drive = toupper((unsigned char)temp[1]);
        snprintf(dst, dst_size, "%c:%s", drive, temp + 2);
    } else {
        strncpy(dst, temp, dst_size - 1);
        dst[dst_size - 1] = '\0';
    }
}

// 切换目录
int change_dir(const char *path) {
    char norm_path[PATH_MAX_LEN];
    char final_path[PATH_MAX_LEN];
    int i = 0, j = 0;
    
    if (path == NULL || strlen(path) == 0) {
        printf("用法：cd <路径>\n");
        return -1;
    }
    
    normalize_path(path, norm_path, sizeof(norm_path));
    
    // 去除引号
    while (norm_path[i]) {
        if (norm_path[i] != '\"' && norm_path[i] != '\'') {
            final_path[j++] = norm_path[i];
        }
        i++;
    }
    final_path[j] = '\0';
    
    if (chdir(final_path) != 0) {
        printf("无法切换到目录：%s\n", path);
        return -1;
    }
    
    char cwd[PATH_MAX_LEN];
    if (GETCWD(cwd, sizeof(cwd)) != NULL) {
        printf("当前目录：%s\n", cwd);
    }
    return 0;
}

// 显示当前目录
void show_pwd() {
    char cwd[PATH_MAX_LEN];
    if (GETCWD(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        printf("无法获取当前目录\n");
    }
}

// 执行 BF 代码
void run_bf(const char *code, int interactive) {
    unsigned char memory[MEMORY_SIZE] = {0};
    unsigned char *ptr = memory;
    const char *pc = code;
    int loop_stack[MEMORY_SIZE];
    int stack_top = -1;
    
    while (*pc) {
        switch (*pc) {
            case BF_INC_PTR:
                ptr++;
                if (ptr >= memory + MEMORY_SIZE) {
                    printf("错误：内存越界\n");
                    return;
                }
                break;
            case BF_DEC_PTR:
                ptr--;
                if (ptr < memory) {
                    printf("错误：内存越界\n");
                    return;
                }
                break;
            case BF_INC_VAL:
                (*ptr)++;
                break;
            case BF_DEC_VAL:
                (*ptr)--;
                break;
            case BF_OUTPUT:
                putchar(*ptr);
                fflush(stdout);
                break;
            case BF_INPUT:
                *ptr = getchar();
                break;
            case BF_LOOP_START:
                if (*ptr == 0) {
                    int depth = 1;
                    while (depth > 0) {
                        pc++;
                        if (*pc == BF_LOOP_START) depth++;
                        if (*pc == BF_LOOP_END) depth--;
                        if (*pc == '\0') {
                            printf("错误：未找到匹配的 ']'\n");
                            return;
                        }
                    }
                } else {
                    loop_stack[++stack_top] = (int)(pc - code);
                }
                break;
            case BF_LOOP_END:
                if (*ptr != 0) {
                    if (stack_top < 0) {
                        printf("错误：未找到匹配的 '['\n");
                        return;
                    }
                    pc = code + loop_stack[stack_top];
                } else {
                    stack_top--;
                }
                break;
            default:
                break;
        }
        pc++;
    }
    if (!interactive) {
        printf("\n");
    }
}

// 从文件加载 BF 代码
char* load_bf_from_file(const char *filename) {
    char norm_path[PATH_MAX_LEN];
    FILE *file;
    long file_size;
    char *content;
    char *bf_code;
    int idx = 0, i;
    
    normalize_path(filename, norm_path, sizeof(norm_path));
    
    file = fopen(norm_path, "r");
    if (!file) {
        printf("错误：无法打开文件 '%s'\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    content = (char*)malloc(file_size + 1);
    fread(content, 1, file_size, file);
    content[file_size] = '\0';
    fclose(file);
    
    bf_code = (char*)malloc(file_size + 1);
    for (i = 0; i < file_size; i++) {
        char c = content[i];
        if (c == '>' || c == '<' || c == '+' || c == '-' || 
            c == '.' || c == ',' || c == '[' || c == ']') {
            bf_code[idx++] = c;
        }
    }
    bf_code[idx] = '\0';
    
    free(content);
    return bf_code;
}

// 执行 BF 文件
void run_bf_file(const char *filename) {
    printf("\n正在加载文件：%s\n", filename);
    
    char *bf_code = load_bf_from_file(filename);
    if (!bf_code) {
        return;
    }
    
    printf("执行结果：\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    run_bf(bf_code, 0);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("执行完毕！\n\n");
    
    free(bf_code);
}

// 交互模式
void interactive_mode() {
    char input[4096];
    char cmd[64];
    char arg[4064];
    char cwd[PATH_MAX_LEN];
    
    printf("\n╔════════════════════════════════════╗\n");
    printf("║      欢迎使用 BF 语言解释器！       ║\n");
    printf("║    输入 help 查看内置命令           ║\n");
    printf("║    输入 exit 退出程序               ║\n");
    printf("╚════════════════════════════════════╝\n");
    
    if (GETCWD(cwd, sizeof(cwd)) != NULL) {
        printf("\n当前目录：%s\n", cwd);
    }
    printf("\n");
    
    while (1) {
        printf("bf> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        if (input[0] == '\0') {
            continue;
        }
        
        sscanf(input, "%63s %4063[^\n]", cmd, arg);
        
        if (strcmp(cmd, "exit") == 0) {
            printf("再见！\n");
            break;
        }
        else if (strcmp(cmd, "help") == 0) {
            show_help();
        }
        else if (strcmp(cmd, "clear") == 0) {
            clear_screen();
        }
        else if (strcmp(cmd, "cd") == 0) {
            change_dir(arg);
        }
        else if (strcmp(cmd, "pwd") == 0) {
            show_pwd();
        }
        else if (strcmp(cmd, "dir") == 0 || strcmp(cmd, "ls") == 0) {
            list_files();
        }
        else if (strcmp(cmd, "bf") == 0) {
            if (strlen(arg) == 0) {
                printf("用法：bf <文件名.bf>\n");
            } else {
                run_bf_file(arg);
            }
        }
        else {
            run_bf(input, 1);
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]) {
    set_console_utf8();
    
    if (argc == 1) {
        interactive_mode();
    } else {
        run_bf_file(argv[1]);
    }
    
    return 0;
}