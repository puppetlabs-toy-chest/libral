import json
import sys

try:
    import dnf
    from dnf import cli, const, exceptions, subject, util
    HAS_DNF = True
except ImportError:
    HAS_DNF = False

METADATA="""
---
provider:
  type: package
  invoke: json
  actions: [set, get]
  suitable:
    commands: [dnf]
  attributes:
    name:
      desc: the name of the package
    ensure:
      desc: what state the package should be in
      # Our type system is not strong enough to be more precise. ensure is
      # either one of enum[present,installed,absent,purged,held,latest] or
      # a version number (so string might be as tightly as this can be
      # typed anyway
      type: string
    platform:
      desc: the platform (architecture) for which the package was built
      kind: r
"""

#
# Utility methods
#

def dnf_base():
    """Return a fully configured dnf Base object."""
    base = dnf.Base()
    # Configure base
    conf = base.conf
    conf.debuglevel = 0
    conf.assumeyes = True
    conf.read()

    base.read_all_repos()
    base.fill_sack(load_system_repo='auto')
    return base


def pkg_as_json(pkg):
    """Return a dictionary of information for the package."""
    result = {
        'name': pkg.name,
        'ensure': pkg.evr,
        'platform': pkg.arch}
    return result

def find_installed(base, name):
    q=base.sack.query()
    pkgs=q.installed().filter(name=name).latest()
    if len(pkgs) == 0:
        return None
    else:
        return pkgs[0]

#
# Provider actions
#
def do_describe():
    suitable=HAS_DNF
    print(METADATA.format(suitable=str(suitable).lower()))

def do_get(inp):
    names=inp["names"]
    base=dnf_base()
    q=base.sack.query()
    if len(names) > 0:
        q = q.filter(name=names)

    present = [pkg_as_json(pkg) for pkg in q.installed().latest()]
    present_names = [ pkg["name"] for pkg in present ]

    result = { "resources": present }

    if len(names) > 0:
        # We need to build absent packages as a dict to avoid duplicates caused
        # by differences in arch
        # FIXME: strictly speaking, we also need to do that for present
        absent = {}
        for pkg in q.available().latest():
            if pkg.name not in present_names:
                absent[pkg.name] = { "name": pkg.name, "ensure": "absent" }
        result["resources"] += list(absent.values())

    return result

def do_set(inp):
    if not util.am_i_root():
        return {
            "error": {
                "message": "only the root user can make package changes",
                "kind": "failed"
            } }
    try:
        upds=inp["updates"]
        noop=inp["ral"]["noop"]
        base=dnf_base()
        was = {}

        for upd in upds:
            name=upd["name"]
            ensure=upd["should"]["ensure"]

            is_pkg=find_installed(base, name)
            if is_pkg:
                was[name]=is_pkg.evr
            else:
                was[name]="absent"

            if ensure == "present" or ensure == "installed":
              if not is_pkg:
                  base.install(name)
            elif ensure == "absent" or ensure == "purged":
              if is_pkg:
                  base.remove(name)
            elif ensure == "held":
              # No idea what this is supposed to do
              return { "error": { "message": "not implemented",
                                  "kind": "failed" }}
            elif ensure == "latest":
              if is_pkg:
                  base.upgrade(name)
              else:
                  base.install(name)
            else:
              # ensure must be a version number
              target=("%s-%s" % (name, ensure))
              if is_pkg:
                  base.upgrade(target)
              else:
                  base.install(target)

        # Run the transaction
        base.resolve()
        if not noop:
            base.download_packages(base.transaction.install_set)
            base.do_transaction()

        # Generate changes
        result = { "changes": [], "derive": False }
        names = [ upd["name"] for upd in upds ]
        pkgs=[p for p in base.transaction.install_set if p.name in names]
        for pkg in pkgs:
            change={
                "name": pkg.name,
                "ensure": {  "is": pkgs[0].evr, "was": was[pkg.name] } }
            result["changes"].append(change)

        pkgs=[p for p in base.transaction.remove_set if p.name in names]
        for pkg in pkgs:
            change={
                "name": pkg.name,
                "ensure": { "is": "absent", "was": was[p.name] } }
            result["changes"].append(change)

        return result
    except exceptions.Error as e:
        return { "error": { "message": str(e), "kind": "failed" } }
    finally:
        base.close()

def die(msg):
    print(msg)
    sys.exit(1)

def parse_stdin():
    inp=json.load(sys.stdin)
    # Checking isn't strictly necessary as libral will make sure that all
    # these things are set up right, but it helps when monkeying around
    # from the command line
    if not isinstance(inp, dict):
        die("input must be a dict")
    return inp

def dump(obj):
    print(json.dumps(obj))

def main():
    if len(sys.argv) < 2:
        print("usage: dnf.prov ral_action=<action>")
        sys.exit(1)
    action=sys.argv[1].split("=")[-1]
    if action == "describe":
        do_describe()
    elif action == "get":
        dump(do_get(parse_stdin()))
    elif action == "set":
        dump(do_set(parse_stdin()))
    else:
        print("unsupported action: '%s'" % action)
        sys.exit(1)

if __name__ == '__main__':
    main()
