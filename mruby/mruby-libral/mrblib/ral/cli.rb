module Ral
  class ProviderError < RuntimeError; end

  class Update
    attr_reader :name, :is, :should
    def initialize(name, is, should)
      @name = name
      @is = is
      @should = should
    end

    def [](key)
      @should[key] || @is[key]
    end

    def changed?(attr)
      @is[attr] != @should[attr]
    end
  end

  class CLI
    def parse_stdin
      JSON.load($stdin)
    end

    def dump(obj)
      puts JSON.dump(obj)
    end

    def run(argv, prov)
      if argv.size == 0
        puts("usage: #{File::basename($0)} ral_action=<action>")
        exit(1)
      end

      action=argv[0].split("=").last

      ctx = Ral::Context.new

      if action == "describe" && prov.respond_to?(:describe)
        prov.describe(ctx)
      elsif action == "get" && prov.respond_to?(:get)
        inp = parse_stdin
        dump({ resources: Array(prov.get(ctx, inp["names"])).flatten })
      elsif action == "set" && prov.respond_to?(:set)
        inp=parse_stdin

        upds = inp["updates"].map do |upd|
          Update.new(upd["name"], upd["is"], upd["should"])
        end
        begin
          result = prov.set(ctx, upds, inp["ral"]["noop"])
        rescue ProviderError => e
          result = { error: { message: e.message } }
        end
        dump(result)
      else
        dump({ error: { message: "Unknown action #{action}" } })
      end
    end

    def self.run(klass)
      new.run(ARGV, klass.new)
    end
  end
end
