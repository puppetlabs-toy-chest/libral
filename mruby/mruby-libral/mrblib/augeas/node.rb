class Augeas

  # Node provides a read-only view of an Augeas tree node giving its label,
  # value, and a list of children (which must be an array of Node objects)
  class Node
    attr_reader :label, :value, :children

    def initialize(path, value = nil, children = nil)
      @path = path
      @label = path.split("/").last.split("[")[0] if path
      @value = value
      @children = children || []
    end

    # Return the first child with the given label
    def [](lbl)
      @children.find { |c| c.label == lbl } || Node.new(nil)
    end

    # Turn the tree rooted at this Node into a string, using the same
    # notation { LABEL [= VALUE] [CHILDREN] } that Augeas uses when
    # printing trees
    def to_s
      cs = children.map { |c| c.to_s }.join(" ")
      cs = " #{cs}" unless cs.empty?
      if value
        " { #{label} = #{value}#{cs} }"
      else
        " { #{label}#{cs} }"
      end
    end
  end

  # Return a Node containing the subtree rooted at +path+. For this to work
  # properly, +path+ must match a single node
  def tree(path)
    children = match("#{path}/*").map { |p| tree(p) }
    Node.new(path, get(path), children)
  end
end
