module Ral
  # A wrapper around updating a single resource. The +is+ and +should+
  # attributes contain the current state of the resource and the desired
  # state.
  class Update
    # @!attribute r name
    #   @return [String] the name of the resource
    # @!attribute r is
    #   @return [Hash] the current 'is' state of the resource
    # @!attribute r should
    #   @return [Hash] the desired 'should' state of the resource
    attr_reader :name, :is, :should
    def initialize(name, is, should)
      @name = name
      @is = is
      @should = should
    end

    # Returns the value for resource attribute +key+. If it is set in the
    # +should+ resource, that value is returned, if not the +is+ value is
    # returned
    def [](key)
      @should[key] || @is[key]
    end

    # Returns +true+ if the +is+ and +should+ value for attribute +attr+
    # differ and therefore needs to be corrected.
    def changed?(attr)
      @is[attr] != @should[attr]
    end
  end
end
