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
            files = a.match("/augeas//error").map do |err|
              a.get("#{err}/path").sub("/files", "")
            end
            result = error("failed to save: invalid file format: #{files.join(",")}")
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
