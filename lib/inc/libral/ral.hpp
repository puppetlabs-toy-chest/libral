#pragma once

#include <vector>
#include <memory>
#include <boost/optional.hpp>

#include "type.hpp"

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
    std::string _data_dir;
  };
}
