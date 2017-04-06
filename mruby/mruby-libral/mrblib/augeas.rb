# Monkey patching for fun and profit.
#
# FIXME: We add some things to mruby-augeas. They need to eventually be
# merged upstream
class Augeas
  def dump(path)
    ret = []
    if v = get(path)
      ret << "#{path} = #{v}"
    else
      ret << path
    end
    ret << match("#{path}/*").map do |p|
      dump(p)
    end
    ret
  end

end
