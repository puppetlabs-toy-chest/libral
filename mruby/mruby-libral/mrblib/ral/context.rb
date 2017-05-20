module Ral
  class Context
    def initialize
      @derive=false
      @changes={}
    end

    # Logs the string +msg+ on +$stderr+ where libral will take it and log
    # it to its log.
    def log(msg)
      $stderr.puts msg
    end

    # Instructs libral to derive changes by diffing +is+ and +should+
    # resources for any resource where changes are not logged explicitly
    def derive_changes!
      @derive=true
    end

    # Log a change to the resource +upd.name+. If +attr+ is given, only a
    # change for that attribute is logged, otherwise a change for every
    # attribute mentioned in +upd.should+ where the value differs from
    # +upd.is+ is logged. If +should+ is given, it is used as the should
    # value rather than +upd.should[attr]+
    def change(upd, attr=nil, should=nil)
      if attr
        attrs = [ attr ]
      else
        attrs = upd.should.keys
      end
      @changes[upd.name] ||= {}
      @changes[upd.name]["name"] = upd.name
      attrs.each do |a|
        should ||= upd.should[a]
        if should != upd.is[a]
          @changes[upd.name][a] = { was: upd.is[a], is: should }
        end
      end
    end

    def result
      { derive: @derive, changes: @changes.values }
    end

    # Runs the given block with a new Augeas instance. The +opts+ are a
    # hash which can contain the following entries:
    # * <tt>:lens</tt> - the name of the lens to use
    # * <tt>:incl</tt> - a list of glob patterns for the files to transform
    # * <tt>:excl</tt> - a list of the glob patterns to remove from the list that matches <tt>:INCL</tt>
    #   <tt>:save</tt> - if +true+, save Augeas tree after running the block
    def with_augeas(opts, &block)
      Augeas.open(nil, nil, Augeas::NO_MODL_AUTOLOAD) do |a|
        a.transform(opts)
        a.load
        result = yield(a)
        if opts[:save]
          unless a.save
            out = []
            a.match("/augeas//error").each do |err|
              t = a.tree(err)
              # Get rid of /augeas/files prefix and /error suffix
              filename = "/" + err.split("/")[3..-2].join("/")
              line = t["line"].value
              if line
                out << "Error in #{filename}:#{line}.#{t["char"].value} #{t.value}"
              elsif t["path"].value
                out << "Error in #{filename} at node #{t["path"].value} (#{t.value})"
                # out += a.dump(t["path"].value)
              else
                out << "Error in #{filename} (#{t.value})"
              end

              if msg = t["message"] && t["message"].value
                out << msg
              end
            end
            result = error("failed to save: invalid file format:\n#{out.join("\n")}")
          end
        end
        result
      end
    end

    # Raise an error and abort further processing of the current
    # action. This will ultimately report an error to libral and tell it
    # that the action failed.
    def error(msg)
      raise ProviderError, msg
    end
  end
end
