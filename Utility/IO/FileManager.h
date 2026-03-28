#ifndef N900_MODEL_FILEMANAGER_H
#define N900_MODEL_FILEMANAGER_H

#include <vector>
#include <string>

namespace IO {
    class FileManager {
    public:
        static bool IsRegularFile(std::string const &path_to_judge);
        static bool IsDirectory(std::string const &path_to_judge);
        static int DirExist(std::string const &path_to_judge);
        static int CreateDirectory(std::string const &path_to_judge);
        static std::vector<std::string> GetFilesInDirectory(std::string const &path_to_search);
        static std::string PathJoin(std::string const &path1, std::string const &path2);
        FileManager() = default;
        ~FileManager() = default;

    private:
        static void dfs(std::string const &path_to_search, std::vector<std::string> &files_path_vector);

    private:
        static char const PATH_DELIMITER = '/';
    };
}

#endif //N900_MODEL_FILEMANAGER_H
