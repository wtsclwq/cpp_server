#pragma once

#include <memory>
#include <string>

namespace wtsclwq {
class ConfigVarBase {
   public:
    using ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase(const ConfigVarBase &) = default;
    ConfigVarBase(ConfigVarBase &&) = delete;
    auto operator=(const ConfigVarBase &) -> ConfigVarBase & = default;
    auto operator=(ConfigVarBase &&) -> ConfigVarBase & = delete;

    ConfigVarBase(std::string name, std::string description);
    virtual ~ConfigVarBase() = default;

    virtual auto GetName() -> std::string;
    virtual void SetName(std::string name);
    virtual auto GetDescription() -> std::string;
    virtual void SetDescription(std::string description);

    virtual auto ToString() -> std::string = 0;
    virtual auto FromString(std::string val) -> bool = 0;
    virtual auto GetTypeName() const -> std::string = 0;

   private:
    std::string m_name;
    std::string m_description;
};

}  // namespace wtsclwq