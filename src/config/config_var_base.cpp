/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-07 22:46:11
 * @LastEditTime: 2023-03-11 00:50:30
 */
#include "../include/config/config_var_base.h"

#include <algorithm>
#include <cctype>

namespace wtsclwq {

ConfigVarBase::ConfigVarBase(std::string name, std::string description)
    : m_name(std::move(name)), m_description(std::move(description)) {}

auto ConfigVarBase::GetName() -> std::string { return m_name; }

void ConfigVarBase::SetName(std::string name) { m_name = std::move(name); }

auto ConfigVarBase::GetDescription() -> std::string { return m_description; }

void ConfigVarBase::SetDescription(std::string description) {
    m_description = std::move(description);
}

}  // namespace wtsclwq