class Resource
  attr_reader :name, :attrs

  def initialize(name, attrs)
    @name = name
    @attrs = attrs
  end

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

  def delete(*attrs)
    ret = self.dup
    attrs.each { |a| ret.attrs.delete(a) }
    ret
  end
end

class Change
  attr_reader :attr, :is, :was

  def initialize(attr, is, was)
    @attr = attr
    @is = is
    @was = was
  end
end

class Update
  attr_reader :is, :should, :changes, :name

  def initialize(is, should, changes)
    @is = is
    @should = should
    @name = @is["name"]
    @changes = changes
  end

  def resource
    attrs = @is.attrs.merge(@should.attrs)
    @changes.each { |c| attrs[c.attr] = c.is }
    Resource.new(@is.name, attrs)
  end
end

class RalSuite < MTest::Unit::TestCase
  def provider
    @@provider
  end

  def set(*args)
    @@provider.set(args)
  end

  def get(*args)
    @@provider.get(args)
  end

  def assert_ensure(ens, name, rsrcs)
    rsrcs = Array(rsrcs)
    res = rsrcs.find { |res| res.name == name }
    assert !res.nil?, "resource #{name} should exist , but is missing"
    assert_equal ens, res["ensure"], "resource #{res.name} has wrong ensure value"
  end

  def assert_absent(name, rsrcs)
    assert_ensure "absent", name, rsrcs
  end

  def assert_present(name, rsrcs)
    assert_ensure "present", name, rsrcs
  end

  def self.provider(name, opts={})
    @@provider = Provider.new(name)
    unless opts[:root] == false
      puts "add hook"
      add_setup_hook Proc.new { |test| test.skip "need to be root" }
    end
  end

  def resource(name, attrs={})
    Resource.new(name, attrs)
  end
end
