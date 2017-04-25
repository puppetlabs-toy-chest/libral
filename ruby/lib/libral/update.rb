# Combines the 'is' and 'should' state of a resource and records any
# changes that +Provider.set+ might have had to make to enforce the
# +should+ state.
#
# @!attribute [r] is
#   @return [Libral::Resource] the state of the resource before the update
# @!attribute [r] should
#   @return [Libral::Resource] the desired state of the resource provided by the user
# @!attribute [r] changes
#   @return [Array<Change>] the changes that were performed to enforce the +should+ state
# @!attribute [r] name
#   @return [String] the name of the underlying resource, same as +is.name+
#     and +should.name+
class Libral::Update
  attr_reader :is, :should, :changes, :name

  def initialize(is, should, changes)
    puts "update for #{is.name}"
    @is = is
    @should = should
    @name = @is.name
    @changes = changes
  end

  # Returns a combination of the 'is' and 'should' state as a +Resource+:
  # for attributes for which we have a 'should' value, that is used. For
  # all other attributes, the +is+ value is used.
  # @return [Libral::Resource] an overlay of the +should+ and +is+ resource
  def resource
    attrs = @is.attrs.merge(@should.attrs)
    @changes.each { |c| attrs[c.attr] = c.is }
    Resource.new(@is.name, attrs)
  end
end
