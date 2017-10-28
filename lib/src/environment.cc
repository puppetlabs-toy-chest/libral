#include <libral/environment.hpp>

#include <libral/ral.hpp>
#include <libral/target/base.hpp>

namespace libral {

  namespace aug = libral::augeas;

  const std::vector<std::string>&
  environment::data_dirs() const { return _ral->data_dirs(); }

  libral::command::uptr
  environment::command(const std::string& cmd) {
    return _ral->target()->command(cmd);
  }

  libral::command::uptr
  environment::script(const std::string& cmd) {
    return _ral->target()->script(cmd);
  }

  std::string environment::which(const std::string& cmd) const {
    return _ral->target()->which(cmd);
  }

  bool environment::is_local() const {
    return _ral->is_local();
  }

  result<std::shared_ptr<augeas::handle>>
  environment::augeas(const std::vector<std::pair<std::string, std::string>>& xfms) {
    return _ral->target()->augeas(data_dirs(), xfms);
  }

  result<prov::spec>
  environment::parse_spec(const std::string& name,
                          const std::string& desc,
                          bool suitable) {
    return prov::spec::read(*this, name, desc, suitable);
  }
}
