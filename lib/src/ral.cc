#include <libral/ral.hpp>
namespace libral {
  ral::ral(void) { }

  std::vector<std::unique_ptr<type>> ral::types(void) {
    // @todo lutter 2016-05-10:
    //   Need more magic here: need to find and register all types
    std::vector<std::unique_ptr<type>> result;
    result.push_back(std::unique_ptr<type>(new mount_type()));
    result.push_back(std::unique_ptr<type>(new type("user")));
    return result;
  }
}
