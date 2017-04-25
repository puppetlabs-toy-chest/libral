# Represents a change to an individual attribute. The name of the attribute
# is stored in +attr+, the attribute's value before the change was made is
# stored in +was+, and the attribute's value after the change is stored in
# +is+
class Libral::Change
  attr_reader :attr, :is, :was

  def initialize(attr, is, was)
    @attr = attr
    @is = is
    @was = was
  end
end
