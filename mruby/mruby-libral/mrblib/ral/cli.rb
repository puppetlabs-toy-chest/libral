module Ral
  class ProviderError < RuntimeError; end

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
      elsif action == "list" && prov.respond_to?(:list)
        dump({ resources: prov.list(ctx).flatten })
      elsif action == "find" && prov.respond_to?(:find)
        inp=parse_stdin
        dump({ resource: prov.find(ctx, inp["resource"]["name"]) })
      elsif action == "update" && prov.respond_to?(:update)
        inp=parse_stdin
        # This should be part of the calling convention. Instead, we look
        # things up ourselves
        res = inp["resource"]
        pair = {
          is: prov.find(ctx, res["name"]),
          should: res.dup
        }
        begin
          prov.update(ctx, [pair], inp["ral"]["noop"])
          changes = {}
          res.each do |k,v|
            if v != pair[:is][k]
              changes[k] = { is: v, was: pair[:is][k] }
            end
          end
          result = { changes: changes }
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
