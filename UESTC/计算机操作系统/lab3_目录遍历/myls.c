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

void print_file_info(const char *filename, struct stat *info);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *path = argv[1];
    struct stat path_stat;

    if (stat(path, &path_stat) == -1) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(path_stat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory.\n", path);
        exit(EXIT_FAILURE);
    }

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

void print_file_info(const char *filename, struct stat *info) {
    char perms[11];

    if (S_ISDIR(info->st_mode)) perms[0] = 'd';
    else if (S_ISLNK(info->st_mode)) perms[0] = 'l';
    else if (S_ISCHR(info->st_mode)) perms[0] = 'c';
    else if (S_ISBLK(info->st_mode)) perms[0] = 'b';
    else if (S_ISFIFO(info->st_mode)) perms[0] = 'p';
    else if (S_ISSOCK(info->st_mode)) perms[0] = 's';
    else perms[0] = '-';

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
    printf("%2ld ", (long)info->st_nlink);

    struct passwd *pw = getpwuid(info->st_uid);
    if (pw != NULL) printf("%-8s ", pw->pw_name);
    else printf("%-8d ", info->st_uid);

    struct group *gr = getgrgid(info->st_gid);
    if (gr != NULL) printf("%-8s ", gr->gr_name);
    else printf("%-8d ", info->st_gid);

    printf("%8ld ", (long)info->st_size);
    
    char* time_str = ctime(&info->st_mtime);
    printf("%.12s ", time_str + 4);

    printf("%s\n", filename);
}