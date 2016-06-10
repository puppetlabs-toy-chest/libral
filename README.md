# Libral

Libral is a sketch of an attempt to try and build a native provider
platform/framework. The goals of this are

* Roughly match the functionality of the `puppet resource` command
* Serve as the basis for Puppet's provider system
* Avoid being Puppet specific as much as possible to make it useful for
  other uses
* Reduce the overhead of writing providers as much as possible, Ansible
  seems to have struck a chord in that area

## Required packages

You will need to install [Boost](http://boost.org) for program_options and
to use many of the libraries in
[Leatherman](https://github.com/puppetlabs/leatherman). You will also need
[Augeas](http://augeas.net/)

## Todo list

- [X] finish mount provider
- [ ] add a shell provider
- [ ] add a remote provider (using an HTTP API)
- [ ] adapt providers to multiple OS (maybe using mount)
- [ ] more core providers
- [ ] noop mode
- [ ] event reporting

## Some language

Naming things is hard; here's the terms libral uses:

* _Type_: the abstract description of an entity we need to manage; this is
  the external interface through which entities are managed. I am not
  entirely convinced this belongs in libral at all
* _Provider_: something that knows how to manage a certain kind of thing
  with very specific means; for example something that knows how to manage
  users with the `user*` commands, or how to manage mounts on Linux
* _Resource_: an instance of something that a provider manages. This is
  closely tied both to what is being managed and how it is being
  managed. The important thing is that resources expose a desired-state
  interface and therefore abstract away the details of how changes are made

### Open questions
- Do we need types at all at this level ?
- Can we get away without any provider selection logic ? There are two
  reasons why provider selection is necessary:
  * adjust to system differences. Could we push those to compile time ?
  * manage different things that are somewhat similar, like system packages
    and gems, or local users and LDAP users ? This would push this
    modelling decision into a layer above libral.
- Wouldn't it be better to make providers responsible for `noop` mode
  rather than making it part of the framework ?
- Would it be better to make providers responsible for event generation
  rather than doing that in the framework ?

## Provider lifecycle

The intent is that the lifecycle for providers will be roughly something
like this:

```cpp
    some_provider prov();

    if (prov.suitable()) {
      // initialize provider internals
      prov.prepare();

      // Loop over all resources
      for (auto rsrc : prov.instances()) {
        auto should = get_new_attrs_from_somewhere();
        rsrc.update(should);
      }

      // or do something to a specific resource
      auto rsrc = prov.find(some_name);
      rsrc.destroy();

      // or create a new one
      auto rsrc = prov.create("new_one");
      auto should = get_new_attrs_from_somewhere_else();
      rsrc.update(should);
      rsrc.flush()

      // make sure all changes have been written to disk
      prov.flush();

      // not sure yet if we need an explicit 'close' call
      prov.close()
    }
```

* Rather than returning just a `bool`, at some point `suitable()` will need
  to return more details about what the provider does and whether it can be
  used
* At some point, we'll need a notion of a context that tells providers
  about system details (like facts) and some settings; would be cool to use
  that to change the idea of where the root of the FS is, for example.

## External provider

Providers can be scripts; a script called `sc` needs to support the
following invocations:

* `sc describe` - describe the script, including whether it is suitable
* `sc list` - list all instances
* `sc find NAME` (optional) - find the instance named `NAME`
* `sc destroy NAME` - destroy the instance with name `NAME`
* `sc update NAME A1=V1 ... AN=VN` - update (or create) resource
* `sc flush`

### Output from describe

The output from describe must be a list of key/value pairs, with a starting
line that is `# simple`:

    # simple
    suitable: 1
    type: great_type
    attributes: attr1,attr2,attr3

The output can contain the following keys:

* `suitable`: anything other than one of `1`, `yes`, or `true` means
              `false` and the provider is ignored.
* `attributes`:  a comma-separated list of permissible attributes
                 (do we need it ?)
* `type`: defaults to the name of the script

### Output from list

List must produce the following:

    # simple
    name: first_name
    attr1: value1
    ...
    attrN: valueN
    name: second_name
    attr1: value1
    ...
    attrN: valueN
    name: third_name
    ...

### Output from find

    # simple
    name: the_name
    attr1: value1
    ...
    attrN: valueN

### Output from destroy

success/failure

### Output from update

Events ?

### Output from flush

success/failure

### Error reporting

Yo, dawg
