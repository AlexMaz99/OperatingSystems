#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

const char format[] = "%Y-%m-%d %H:%M:%S";
int atime = -1, mtime = -1;
int max_depth = 100000;
char asign, msign;

void print_results(char* file_path, struct stat *stat){
    char file_type[64] = "undefined";
    if (S_ISREG(stat -> st_mode)) strcpy(file_type, "file");
    else if (S_ISDIR(stat -> st_mode)) strcpy(file_type, "dir");
    else if (S_ISLNK(stat -> st_mode)) strcpy(file_type, "slink");
    else if (S_ISCHR(stat -> st_mode)) strcpy(file_type, "char dev");
    else if (S_ISBLK(stat -> st_mode)) strcpy(file_type, "block dev");
    else if (S_ISFIFO(stat -> st_mode)) strcpy(file_type, "fifo");
    else if (S_ISSOCK(stat -> st_mode)) strcpy(file_type, "socket");

    struct tm tm_modif_time;
    localtime_r(&stat -> st_mtime, &tm_modif_time);
    char modif_time_str[255];
    strftime(modif_time_str, 255, format, &tm_modif_time);

    struct tm tm_access_time;
    localtime_r(&stat -> st_atime, &tm_access_time);
    char access_time_str[255];
    strftime(access_time_str, 255, format, &tm_access_time);

    printf("Path: %s || type: %s, size: %ld, modification time: %s, access time: %s, nlinks: %ld\n\n",
    file_path, file_type, stat -> st_size, modif_time_str, access_time_str, stat -> st_nlink);
}

void print_results_nftw(const char* file_path, const struct stat *stat){
    char file_type[64] = "undefined";
    if (S_ISREG(stat -> st_mode)) strcpy(file_type, "file");
    else if (S_ISDIR(stat -> st_mode)) strcpy(file_type, "dir");
    else if (S_ISLNK(stat -> st_mode)) strcpy(file_type, "slink");
    else if (S_ISCHR(stat -> st_mode)) strcpy(file_type, "char dev");
    else if (S_ISBLK(stat -> st_mode)) strcpy(file_type, "block dev");
    else if (S_ISFIFO(stat -> st_mode)) strcpy(file_type, "fifo");
    else if (S_ISSOCK(stat -> st_mode)) strcpy(file_type, "socket");

    struct tm tm_modif_time;
    localtime_r(&stat -> st_mtime, &tm_modif_time);
    char modif_time_str[255];
    strftime(modif_time_str, 255, format, &tm_modif_time);

    struct tm tm_access_time;
    localtime_r(&stat -> st_atime, &tm_access_time);
    char access_time_str[255];
    strftime(access_time_str, 255, format, &tm_access_time);

    printf("Path: %s || type: %s, size: %ld, modification time: %s, access time: %s, nlinks: %ld\n\n",
    file_path, file_type, stat -> st_size, modif_time_str, access_time_str, stat -> st_nlink);
}

int check_time(int count, char sign, time_t time_from_file){
    time_t now;
    struct tm *time_info;
    time(&now);
    time_info = localtime(&now);
    time_t current_date = mktime(time_info);

    int diff = difftime(current_date, time_from_file) / 86400; // (24h * 60min * 60s)

    if ((sign == '+' && diff > count) || (sign == '-' && diff < count) || (sign == '=' && diff == count)) 
        return 1;

    return 0;
}

int check_conditions(struct stat* dir_stat){
    if (atime != -1 && check_time(atime, asign, dir_stat -> st_atime) == 0) return 0;
    if (mtime != -1 && check_time(mtime, msign, dir_stat -> st_mtime) == 0) return 0;
    return 1;
}

int check_conditions_nftw(const struct stat* dir_stat){
    if (atime != -1 && check_time(atime, asign, dir_stat -> st_atime) == 0) return 0;
    if (mtime != -1 && check_time(mtime, msign, dir_stat -> st_mtime) == 0) return 0;
    return 1;
}

void find_dir(char *path, int depth){
    if (depth > max_depth) return;
    if (path == NULL) return;
    DIR* dir = opendir(path);
    if (dir == NULL){
        printf("Cannot open directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *file;
    char new_path[256];

    while ((file = readdir(dir)) != NULL){
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);
        
        struct stat buffer;
        if (lstat(new_path, &buffer) < 0){
            printf("Cannot lstat file %s: ", new_path);
            exit(EXIT_FAILURE);
        }
        
        if (S_ISDIR(buffer.st_mode)){
            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0){
                continue;
            }
            find_dir(new_path, depth + 1);
        }
        if (check_conditions(&buffer) == 1){
            print_results(new_path, &buffer);
        }
        
    }
    closedir(dir);
}

static int find_nftw(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if (ftwbuf -> level > max_depth) return 0;
    if (check_conditions_nftw(sb) == 1){
        print_results_nftw(fpath, sb);
    }
    return 0;
}

int main(int argc, char** argv){
    if (argc < 2){
        printf("Wrong number of arguments");
        exit(EXIT_FAILURE);
    }

    char* path;
    path = argv[1];
    int i = 2;
    int is_nftw = 0;

    while(i < argc){
        if (!strcmp(argv[i], "-mtime")){
            if (mtime != -1){
                printf("Too many declarations of mtimt");
                exit(EXIT_FAILURE);
            }
            i++;
            if (argv[i][0] == '+' || argv[i][0] == '-') msign = argv[i][0];
            else msign = '=';
            mtime = abs(atoi(argv[i]));
        }
        else if(!strcmp(argv[i], "-atime")){
            if (atime != -1){
                printf("Too many declarations of atimt");
                exit(EXIT_FAILURE);
            }
            i++;
            if (argv[i][0] == '+' || argv[i][0] == '-') asign = argv[i][0];
            else asign = '=';
            atime = abs(atoi(argv[i]));
        }
        else if (!strcmp(argv[i], "-maxdepth")){
            i++;
            max_depth = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "nftw")) {
            is_nftw = 1;
        }
        i ++;
    }

    if (is_nftw == 1){
        nftw(path, find_nftw, 10, FTW_PHYS);
    }
    else{
        find_dir(path, 1);
    }
    return 0;

}
