#pragma once

#include <vector>

#include <boost/optional.hpp>

#include <libral/command.hpp>
#include <libral/augeas.hpp>
#include <libral/prov/spec.hpp>

namespace libral {
  // Forward declaration to break an include loop
  class ral;

  /**
   * This class represents the environment in which providers operate and
   * contains convenience function for accessing various aspects of the
   * environment. An instance of this class is passed to the provider's
   * describe method. */
  class environment {
  public:
    /**
     * Looks for a command called \p cmd and returns a command object that
     * can be used to run the command.
     *
     * @return \c boost::none if no command \p cmd can be found, or a
     * command object to run this command.
     */
    libral::command::uptr command(const std::string& cmd);

    libral::command::uptr script(const std::string& cmd);

    std::string which(const std::string& cmd) const;

    /**
     * Creates an augeas handle that has the files described by \p xfms
     * loaded.
     */
    result<std::shared_ptr<augeas::handle>>
    augeas(const std::vector<std::pair<std::string, std::string>>& xfms);

    const std::vector<std::string>& data_dirs() const;

    /**
     * Creates provider metadata from the description \p desc, which must
     * be valid YAML and comply with the provider metadata
     * specification. The \p name is used as the default name for the
     * provider, unless the metadata specifies a different name.
     *
     * If \p suitable is not \c boost::none, it takes precedence over a
     * suitable entry that might be in \p desc. If \p suitable is
     * boost::none, then \p desc must contain an entry that indicates
     * whether the provider is suitable or not.
     */
    result<prov::spec> parse_spec(const std::string& name,
                                  const std::string& desc,
                                  boost::optional<bool> suitable = boost::none);

    result<prov::spec>
    parse_spec(const std::string& name, const YAML::Node &node);

  protected:
    friend class ral;

    environment(std::shared_ptr<ral> ral) : _ral(ral) { }
  private:
    std::shared_ptr<ral> _ral;
  };
}
