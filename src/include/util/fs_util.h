//
// Created by wtsclwq on 23-3-31.
//

#pragma once

#include <iostream>
#include <string>
#include <vector>
namespace wtsclwq {

class FsUtil {
  public:
    static void ListAllFile(std::vector<std::string>& files,
                            const std::string& path, const std::string& subfix);

    static auto Mkdir(const std::string& dirname) -> bool;

    static auto IsRunningPidfile(const std::string& pidfile) -> bool;

    static auto Rm(const std::string& path) -> bool;

    static auto Mv(const std::string& from, const std::string& to) -> bool;

    static auto Realpath(const std::string& path, std::string& rpath) -> bool;

    static auto Symlink(const std::string& frm, const std::string& to) -> bool;

    static auto Unlink(const std::string& filename, bool exist = false) -> bool;

    static auto Dirname(const std::string& filename) -> std::string;

    static auto Basename(const std::string& filename) -> std::string;

    static auto OpenForRead(std::ifstream& ifs, const std::string& filename,
                            std::ios_base::openmode mode) -> bool;

    static auto OpenForWrite(std::ofstream& ofs, const std::string& filename,
                             std::ios_base::openmode mode) -> bool;
};

}  // namespace wtsclwq
