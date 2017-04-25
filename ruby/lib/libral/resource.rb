# An individual thing that we manage. Each resource has a +name+, which
# identifies it uniquely among all the resources that a provider manages,
# and a set of attributes.
#
# @!attribute [r] name
#   @return [String] the resource name
# @!attribute [r] attrs
#   @return [Hash] the attributes of the resource
class Libral::Resource
  attr_reader :name, :attrs

  def initialize(name, attrs)
    @name = name
    @attrs = attrs
  end

  # Returns the value of the attribute +key+
  def [](key)
    @attrs[key]
  end

  def to_s
    attrs = @attrs.map { |k,v| "#{k}: #{v}" }.join(", ")
    "{ #{@name}:: #{attrs} }"
  end

  def inspect
    to_s
  end

  def dump(prov, indent)
    puts "#{indent}#{prov} { #{@name} :"
    @attrs.each do |k,v|
      puts "#{indent}  #{k} => #{v}"
    end
    puts "#{indent}}"
  end

  def ==(other)
    name == other.name && attrs == other.attrs
  end

  # Returns a duplicate of this resource without the attributes +attrs+
  # set.
  #
  # @param names [Array<String>] names of the attributes to omit
  # @return [Libral::Resource] a new resource without +attrs+ set
  def delete(*attrs)
    ret = self.dup
    attrs.each { |a| ret.attrs.delete(a) }
    ret
  end
end
