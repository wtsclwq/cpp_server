//
// Created by wtsclwq on 23-3-31.
//

#include "../include/util/fs_util.h"

#include <dirent.h>
#include <sys/stat.h>

#include <csignal>
#include <cstring>
#include <fstream>

namespace wtsclwq {

void FsUtil::ListAllFile(std::vector<std::string>& files,  // NOLINT
                         const std::string& path, const std::string& subfix) {
    if (access(path.c_str(), 0) != 0) {
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            if ((strcmp(dp->d_name, ".") == 0) ||
                (strcmp(dp->d_name, "..") == 0)) {
                continue;
            }
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        } else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);  // NOLINT
            } else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) ==
                    subfix) {
                    files.push_back(path + "/" + filename);  // NOLINT
                }
            }
        }
    }
    closedir(dir);
}

static auto __Lstat(const char* file, struct stat* st) -> int {
    struct stat lst {};
    int ret = lstat(file, &lst);
    if (st != nullptr) {
        *st = lst;
    }
    return ret;
}

static auto __Mkdir(const char* dirname) -> int {  // NOLINT
    if (access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

auto FsUtil::Mkdir(const std::string& dirname) -> bool {  // NOLINT
    if (__Lstat(dirname.c_str(), nullptr) == 0) {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do {
        for (; ptr != nullptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if (__Mkdir(path) != 0) {
                break;
            }
        }
        if (ptr != nullptr) {
            break;
        }
        if (__Mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;
    } while (0);
    free(path);
    return false;
}

auto FsUtil::IsRunningPidfile(const std::string& pidfile) -> bool {
    if (__Lstat(pidfile.c_str(), nullptr) != 0) {
        return false;
    }
    std::ifstream ifs(pidfile);
    std::string line;
    if (!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if (line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());  // NOLINT
    if (pid <= 1) {
        return false;
    }
    if (kill(pid, 0) != 0) {
        return false;
    }
    return true;
}

auto FsUtil::Unlink(const std::string& filename, bool exist) -> bool {
    if (!exist && (__Lstat(filename.c_str(), nullptr) != 0)) {
        return true;
    }
    return ::unlink(filename.c_str()) == 0;
}

auto FsUtil::Rm(const std::string& path) -> bool {  // NOLINT
    struct stat st {};
    if (lstat(path.c_str(), &st) != 0) {
        return true;
    }
    if ((st.st_mode & S_IFDIR) == 0U) {
        return Unlink(path);
    }

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }

    bool ret = true;
    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        if ((strcmp(dp->d_name, ".") == 0) || (strcmp(dp->d_name, "..") == 0)) {
            continue;
        }
        std::string dirname = path + "/" + dp->d_name;
        ret = Rm(dirname);
    }
    closedir(dir);
    if (::rmdir(path.c_str()) != 0) {
        ret = false;
    }
    return ret;
}

auto FsUtil::Mv(const std::string& from, const std::string& to) -> bool {
    if (!Rm(to)) {
        return false;
    }
    return rename(from.c_str(), to.c_str()) == 0;
}

auto FsUtil::Realpath(const std::string& path, std::string& rpath) -> bool {
    if (__Lstat(path.c_str(), nullptr) != 0) {
        return false;
    }
    char* ptr = ::realpath(path.c_str(), nullptr);
    if (nullptr == ptr) {
        return false;
    }
    std::string(ptr).swap(rpath);
    free(ptr);
    return true;
}

auto FsUtil::Symlink(const std::string& from, const std::string& to) -> bool {
    if (!Rm(to)) {
        return false;
    }
    return ::symlink(from.c_str(), to.c_str()) == 0;
}

auto FsUtil::Dirname(const std::string& filename) -> std::string {
    if (filename.empty()) {
        return ".";
    }
    auto pos = filename.rfind('/');
    if (pos == 0) {
        return "/";
    }
    if (pos == std::string::npos) {
        return ".";
    }
    return filename.substr(0, pos);
}

auto FsUtil::Basename(const std::string& filename) -> std::string {
    if (filename.empty()) {
        return filename;
    }
    auto pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return filename;
    }
    return filename.substr(pos + 1);
}

auto FsUtil::OpenForRead(std::ifstream& ifs, const std::string& filename,
                         std::ios_base::openmode mode) -> bool {
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}

auto FsUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename,
                          std::ios_base::openmode mode) -> bool {
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open()) {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}
}  // namespace wtsclwq