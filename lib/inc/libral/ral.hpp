#pragma once

#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include <libral/result.hpp>
#include <libral/type.hpp>

namespace libral {
  class ral {
  public:
    ral(const std::string& data_dir);
    ral(const ral&);

    std::vector<std::unique_ptr<type>> types(void);
    boost::optional<std::unique_ptr<type>> find_type(const std::string& name);
  private:
    bool add_type(std::vector<std::unique_ptr<type>>& types,
                  const std::string& name, std::shared_ptr<provider> prov);
    result<YAML::Node> parse_metadata(const std::string& path,
                                      const std::string& yaml) const;
    result<std::string> run_describe(const std::string& path) const;
    std::string _data_dir;
  };
}
