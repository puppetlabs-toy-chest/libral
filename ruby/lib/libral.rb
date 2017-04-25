require "libral/version"
require "libral/change"
require "libral/resource"
require "libral/update"

require "libral/libral"

# The module containing bindings to libral and supporting classes
# @example
#   ral = Libral::open
#   prov = ral.provider("user")
#   rs = prov.get.find { |x| x.name == "root" }
#   rs.attrs["comment"] = "The super user"
#   prov.set(rs)
module Libral
end


# FIXME: This documentation should be in libral.cc, but I can't for the
# life of me figure out how to get YARD to extract it from there.
# if true
# @!parse
#   # Creates a new libral connection.
#   # @return [Libral::Ral]
#   def Libral.open; end
#
#   # A provider knows how to manage a certain kind of resources, such as a
#   # user or a service.
#   class Libral::Provider
#     # Gets a list of resources. The returned resources will at least
#     # contain all the resources with names +names+, but might contain more,
#     # or even all resources this provider can manage
#     #
#     # @example
#     #   ral = Libral::open
#     #   prov = ral.provider("user")
#     #   root = prov.get.find { |x| x.name == "root" }
#     #
#     # @param names a list of resource names to retrieve. Each entry in this
#     #        array can either be a +String+ or an +Array<String>+
#     #
#     # @return [Array<Resource>] list of resources
#     def get(*names); end
#
#     # Sets (enforces) the state of resources. Each resource only needs to
#     # have the attributes specified that should actually be
#     # changed. Attributes that are not mentioned in +resources+ are
#     # automatically taken from the current state of the resources.
#     #
#     # @example
#     #   ral = Libral::open
#     #   prov = ral.provider("service")
#     #   rs = Libral::Resource.new("etcd", "ensure" => "running")
#     #   upd = prov.set(rs).first
#     #   # Do things with upd.is, upd.should, or upd.changes
#     #   puts "Made a change" unless upd.changes.empty?
#     #
#     # @param resources [Array<Resource>] an array of all the resources and
#     #   the desired state that should be enforced for them.
#     #
#     # @return [Array<Update>] the updates that were made. Only resources that
#     #   had any changes made to them are mentioned in the return value. The
#     #   return value might mention more resources than just the ones passed
#     #   in.
#     def set(resources); end
#
#     # Returns the fully qualified name of the provider.
#     # @return [String] the name of the provider
#     def name; end
#   end
#
#   # Represents a connection to the libral library. Instances of this class
#   # are produced by +Libral::open+
#   class Libral::Ral
#     # Looks up and returns the provider with the given name.
#     # @return [Libral::Provider] the provider with name +name+
#     def provider(name); end
#   end
# end
