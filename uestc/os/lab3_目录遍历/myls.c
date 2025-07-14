#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// 函数声明：用于打印单个文件的属性 (此函数与上一版完全相同)
void print_file_info(const char *filename, struct stat *info);

int main(int argc, char *argv[]) {
    // (1) 检查命令行参数数量
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *path = argv[1];
    struct stat path_stat;

    // 验证路径是否存在
    if (stat(path, &path_stat) == -1) {
        perror(path); // 如果路径不存在，stat会设置errno，perror会打印相应错误
        exit(EXIT_FAILURE);
    }

    // (2) 检查参数是否为一个目录
    if (!S_ISDIR(path_stat.st_mode)) { // 
        fprintf(stderr, "Error: '%s' is not a directory.\n", path);
        exit(EXIT_FAILURE);
    }

    // (3) 参数有效，开始遍历指定目录
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir error");
        exit(EXIT_FAILURE);
    }

    printf("Listing contents of: %s\n", path);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        struct stat file_stat;
        char fullpath[1024];
        // 构建完整的文件路径
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &file_stat) == -1) {
            perror(entry->d_name);
            continue;
        }
        
        print_file_info(entry->d_name, &file_stat);
    }

    closedir(dir);

    return 0;
}

/**
 * @brief 打印单个文件的详细信息，模仿"ls -l"的输出格式 (此函数无需修改)
 * @param filename 文件名
 * @param info 指向该文件stat结构的指针
 */
void print_file_info(const char *filename, struct stat *info) {
    char perms[11];

    // 文件类型
    if (S_ISDIR(info->st_mode)) perms[0] = 'd'; // 
    else if (S_ISLNK(info->st_mode)) perms[0] = 'l'; // [cite: 593]
    else if (S_ISCHR(info->st_mode)) perms[0] = 'c'; // [cite: 589]
    else if (S_ISBLK(info->st_mode)) perms[0] = 'b'; // [cite: 590]
    else if (S_ISFIFO(info->st_mode)) perms[0] = 'p'; // [cite: 591]
    else if (S_ISSOCK(info->st_mode)) perms[0] = 's'; // [cite: 592]
    else perms[0] = '-'; // 普通文件 [cite: 584]

    // 文件权限
    perms[1] = (info->st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (info->st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (info->st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (info->st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (info->st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (info->st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (info->st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (info->st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (info->st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    printf("%s ", perms);
    printf("%2ld ", (long)info->st_nlink); // 硬链接数 [cite: 534]

    struct passwd *pw = getpwuid(info->st_uid); // [cite: 622, 625]
    if (pw != NULL) printf("%-8s ", pw->pw_name);
    else printf("%-8d ", info->st_uid);

    struct group *gr = getgrgid(info->st_gid); // [cite: 636, 639]
    if (gr != NULL) printf("%-8s ", gr->gr_name);
    else printf("%-8d ", info->st_gid);

    printf("%8ld ", (long)info->st_size); // 文件大小 [cite: 537]
    
    char* time_str = ctime(&info->st_mtime); // 最后修改时间 [cite: 539]
    printf("%.12s ", time_str + 4);

    printf("%s\n", filename);
}