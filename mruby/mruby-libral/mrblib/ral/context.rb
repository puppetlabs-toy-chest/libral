module Ral
  class Context
    def log(msg)
      $stderr.puts msg
    end

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

    def error(msg)
      raise ProviderError, msg
    end
  end
end
