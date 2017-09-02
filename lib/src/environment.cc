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

  result<std::shared_ptr<augeas::handle>>
  environment::augeas(const std::vector<std::pair<std::string, std::string>>& xfms) {
    return _ral->target()->augeas(data_dirs(), xfms);
  }

  result<prov::spec>
  environment::parse_spec(const std::string& name,
                          const std::string& desc,
                          boost::optional<bool> suitable) {

    auto spec = prov::spec::read(*this, name, desc);
    err_ret(spec);

    if (suitable)
      spec.ok().suitable(*suitable);
    return spec;
  }

  result<prov::spec>
  environment::parse_spec(const std::string& name, const YAML::Node &node) {
    return prov::spec::read(*this, name, node);
  }
}
