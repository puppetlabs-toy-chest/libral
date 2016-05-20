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

  boost::optional<std::unique_ptr<type>> ral::find_type(const std::string& name) {
    auto types_vec = types();
    auto type_it = std::find_if(types_vec.begin(), types_vec.end(),
                                [name](std::unique_ptr<type> &t)
                                { return name == t->name(); });
    if (type_it == types_vec.end()) {
      return boost::none;
    } else {
      return std::move(*type_it);
    }
  }
}
