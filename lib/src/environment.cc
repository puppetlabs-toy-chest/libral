#include <libral/environment.hpp>

#include <libral/ral.hpp>

namespace libral {

  namespace aug = libral::augeas;

  const std::vector<std::string>&
  environment::data_dirs() const { return _ral->data_dirs(); }

  boost::optional<libral::command>
  environment::command(const std::string& cmd) {
    return command::create(cmd);
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
    err_ret( aug->include("Mount_Fstab.lns", "/etc/fstab") );
    err_ret( aug->include("Mount_Fstab.lns", "/etc/mtab") );
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
