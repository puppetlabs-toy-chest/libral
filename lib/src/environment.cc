#include <libral/environment.hpp>

#include <libral/ral.hpp>

#include <leatherman/execution/execution.hpp>

namespace exe = leatherman::execution;

namespace libral {

  namespace aug = libral::augeas;

  const std::vector<std::string>&
  environment::data_dirs() const { return _ral->data_dirs(); }

  libral::command::uptr
  environment::command(const std::string& cmd) {
    auto abs_cmd = exe::which(cmd);
    if (abs_cmd.empty()) {
      return std::unique_ptr<libral::command>();
    } else {
      auto raw = new libral::command(abs_cmd);
      return std::unique_ptr<libral::command>(raw);
    }
  }

  libral::command::uptr
  environment::script(const std::string& cmd) {
    return command(cmd);
  }

  result<std::shared_ptr<augeas::handle>>
  environment::augeas(const std::vector<std::pair<std::string, std::string>>& xfms) {

    std::stringstream buf;
    bool first=true;

    for (auto dir : data_dirs()) {
      if (!first)
        buf << ":";
      first=false;
      buf << dir << "/lenses";
    }

    auto aug = aug::handle::make(buf.str(), AUG_NO_MODL_AUTOLOAD);
    for (auto& xfm : xfms) {
      err_ret( aug->include(xfm.first, xfm.second) );
    }
    err_ret( aug->load() );

    return aug;
  }

  result<prov::spec>
  environment::parse_spec(const std::string& name,
                          const std::string& desc,
                          boost::optional<bool> suitable) {

    auto spec = prov::spec::read(name, desc);
    err_ret(spec);

    if (suitable)
      spec.ok().suitable(*suitable);
    return spec;
  }

  result<prov::spec>
  environment::parse_spec(const std::string& name, const YAML::Node &node) {
    return prov::spec::read(name, node);
  }
}
