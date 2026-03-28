
#include "FileManager.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>

//#include "InferenceLogger.h"

using std::vector;
using std::string;
using std::shared_ptr;
using std::cerr;
using std::endl;
using std::sort;
using IO::FileManager;
//using log::InferenceLogger;

bool FileManager::IsRegularFile(string const &path_to_judge) {
    struct stat path_stat;
    stat(path_to_judge.c_str(), &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool FileManager::IsDirectory(string const &path_to_judge) {
    struct stat path_stat;
    stat(path_to_judge.c_str(), &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/******************************************************************************
 * Checks to see if a directory exists. Note: This method only checks the
 * existence of the full path AND if path leaf is a dir.
 *
 * @return  >0 if dir exists AND is a dir,
 *           0 if dir does not exist OR exists but not a dir,
 *          <0 if an error occurred (errno is also set)
 *****************************************************************************/
int FileManager::DirExist(const std::string &path_to_judge) {
    struct stat info;
    int statRC = stat(path_to_judge.c_str(), &info );
    if( statRC != 0 ) {
        if (errno == ENOENT)  { return 0; } // something along the path does not exist
        if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
        return -1;
    }
    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}

/**
 *   @return: 0: Successful
 *            -1: Failed
 */
int FileManager::CreateDirectory(std::string const &path_to_judge) {
    if (DirExist(path_to_judge) > 0) return 0;
    return mkdir(path_to_judge.c_str(), S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);
}

void FileManager::dfs(string const &path_to_search, vector<string> &files_path_vector) {
    shared_ptr<DIR> directory_ptr(opendir(path_to_search.c_str()), [](DIR *dir) { return dir && closedir(dir); });
    struct dirent *dirent_ptr;
    if (!directory_ptr) {
        //SPDLOG_LOGGER_ERROR(InferenceLogger::GetLogger(),
        //                    "Error opening : {}. Reason: {}",
        //                    path_to_search,
        //                    strerror(errno));
        exit(1);
    }
    while ((dirent_ptr = readdir(directory_ptr.get())) != nullptr) {
        // get all the files in a directory, ignoring ".." and "."
        if (strcmp(dirent_ptr->d_name, ".") != 0 && strcmp(dirent_ptr->d_name, "..") != 0) {
            string curr_path = PathJoin(path_to_search, string(dirent_ptr->d_name));
            if (IsRegularFile(curr_path)) {
                files_path_vector.push_back(PathJoin(path_to_search, string(dirent_ptr->d_name)));
            } else {
                dfs(curr_path, files_path_vector);
            }
        }
    }
}

string FileManager::PathJoin(string const &path1, string const &path2) {
    string tmp = path1;
    if (path1[path1.size() - 1] != PATH_DELIMITER) {
        tmp += PATH_DELIMITER;
        return (tmp + path2);
    } else
        return (path1 + path2);
}

vector<string> FileManager::GetFilesInDirectory(string const &path_to_search) {
    vector<string> files_path_vector;
    dfs(path_to_search, files_path_vector);
    sort(files_path_vector.begin(), files_path_vector.end());
    return files_path_vector;
}