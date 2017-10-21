#include <libral/target.hpp>

#include <libral/target/base.hpp>
#include <libral/target/local.hpp>
#include <libral/target/ssh.hpp>

namespace libral {
  namespace target {
  std::shared_ptr<base> make_local() {
    return std::shared_ptr<local>(new local());
  }

  std::shared_ptr<base> make_ssh(const std::string& target) {
    return std::shared_ptr<ssh>(new ssh(target));
  }
  }
}
