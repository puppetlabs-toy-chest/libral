class Augeas

  # Simple way to build a tree. Each instance of Builder represents one
  # node in the Augeas tree. Creating a Builder instance creates a node in
  # the tree.
  #
  # As a simple example, the following will replace or add a line to /etc/hosts:
  #
  #   root = aug.build("/scratch/01")
  #   root.child("ipaddr", "10.0.0.1")
  #   root.child("canonical", "srv.example.com")
  #   root.child("alias", "srv")
  #   root.child("alias", "server.example.com")
  #   # If the entry already exists, move to the first path and overwrite
  #   # what's there. If it does not exist, use the second path to create
  #   # it.
  #   root.mv("/files/etc/hosts/*[ipaddr = '10.0.0.1']",
  #           "/files/etc/hosts/01")
  #
  class Builder
    def initialize(aug, path, val=nil)
      @aug = aug
      @aug.set("#{path}[last()+1]", val)
      # We need to match here to canonicalize the path
      @path = @aug.match("#{path}[last()]")[0]
    end

    # Append a new child node with label +lbl+ and value +val+ to the
    # children of this node
    def child(lbl, val=nil)
      Builder.new(@aug, "#{@path}/#{lbl}", val)
    end

    # Move the node connected to this Builder instance to +target+. If
    # +create+ is given, it is used as the target of the move in case the
    # node +target+ does not exist.
    def mv(target, create = nil)
      if create
        if @aug.match(target).size == 0
          aug.mv(@path, create)
        else
          aug.mv(@path, target)
        end
      else
        @aug.mv(@path, target)
      end
    end
  end

  # Build a new subtree with root +root+. Note that the node +root+ will be
  # created; if a node with the same path as +root+ already exists, we
  # still create a second one by calling +set+ on +#{root}[last()+1]+
  def build(root, val=nil)
    Builder.new(self, root, val)
  end

end
