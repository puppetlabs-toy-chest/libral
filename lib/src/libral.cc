#include <libral/version.h>
#include <libral/libral.hpp>

#include <leatherman/logging/logging.hpp>

namespace libral {

  using namespace std;

  string version()
  {
    //LOG_DEBUG("libral version is %1%", LIBRAL_VERSION_WITH_COMMIT);
    return LIBRAL_VERSION_WITH_COMMIT;
  }
}  // libral
