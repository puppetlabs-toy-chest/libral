# Libral

Libral is a sketch of an attempt to try and build a native provider
platform/framework. The goals of this are

* Roughly match the functionality of the `puppet resource` command
* Serve as the basis for Puppet's provider system
* Avoid being Puppet specific as much as possible to make it useful for
  other uses
* Make writing providers very easy

## Getting in touch

* Mailing list: [libral](https://groups.google.com/group/libral)
* IRC: `irc.freenode.net:#libral`

## Building and installation

Please see [this document](HACKING.md) for instructions on building `libral`
from source.

If you just want to quickly try out `libral`, you can download a
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz)
([GPG signature](http://download.augeas.net/libral/ralsh-latest.tgz.sig))
that should work on any Linux machine that has glibc 2.12 or later. (It
might actually work with glibc 2.8 or later) If you succeed in that, please
let [us](mailto:libral@googlegroups.com) know.

## Usage

After you built `libral` you can try things out by running `ralsh`:

```bash
    export RALSH_DATA_DIR=$LIBRAL_CHECKOUT/data
    # list available types
    ./bin/ralsh
    # list all instances of a type
    ./bin/ralsh mount
    # list a specific instance
    ./bin/ralsh service crond
    # make a change for the better
    ./bin/ralsh service crond ensure=stopped
```

### Running inside a container

The script `examples/dpack` produces a directory `build/dpack` that has a
statically linked `ralsh` plus all supporting files in it. You can copy
`build/dpack` into a container and then run it with commands like the
following:

```bash
   CONTAINER=<some_container>
   docker cp build/dpack $CONTAINER:/tmp
   docker exec $CONTAINER /bin/sh -c "RALSH_DATA_DIR=/tmp/dpack/data /tmp/dpack/bin/ralsh"
```

## Todo list

- [X] finish mount provider
- [X] add a shell provider
- [X] event reporting on update
- [X] simple type system and checking for provider attributes
- [X] produce help/details about providers
- [ ] add a json calling convention and use it in a provider
- more core providers
  - [ ] cron
  - [X] file
  - [ ] group
  - [ ] host
  - [ ] package
  - [ ] service (besides systemd)
  - [X] user (userXXX)
- even more core providers
  - [ ] interface
  - [ ] k5login
  - [ ] mailalias
  - [ ] selboolean
  - [ ] selmodule
  - [ ] sshkey
  - [ ] ssh-authorized-key
  - [ ] vlan
  - [ ] yumrepo
- [ ] add a remote provider (using an HTTP API)
- [ ] adapt providers to multiple OS (maybe using mount)
- [ ] noop mode
- [ ] expand the type system to cover as much of Puppet 4 as is reasonable

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

**FIXME**: we need some conventions around some special resource
properties; especially, namevar should always be 'name' and the primary key
for all resources from this provider, and 'ensure' should have a special
meaning (or should it?)

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

## External providers

What resources `libral` can manage is determined by what providers are
available. Some providers are built in and implemented in C++, but doing
that is of course labor-intensive and should only be done for good
reason. It is much simpler, and recommended, that new providers first be
implemented as external providers. External providers are nothing more than
scripts or other executables that follow one of `libral`'s calling
conventions. The different calling conventions trade off implementation
complexity for expressive power.

The following calling conventions are available. If you are just getting
started with `libral`, you should write your first providers using hte
`simple` calling convention.

* [simple](doc/invoke-simple.md)
* `augeas` (planned,maybe): when you mostly need to twiddle entries in a file,
and maybe run a command
* `json` (planned): input/output via JSON
* `json_batch` (planned,maybe): input/output via JSON, can operate on multiple resources at once

## Provider lifecycle

The lifecycle for providers written in C++ follows the outline below. This
applies only to C++ providers - external providers follow a much simpler
lifecycle which is described in the section above for each calling
convention.

```cpp
    some_provider prov();

    // A call to suitable() must also initialize any provider internals.
    // Once suitable() returns true, the provider must be ready to use.
    if (prov.suitable()) {
      // Called by libral once when it sets up a provider
      auto metadata = prov.describe();

      // Loop over all resources
      for (auto rsrc : prov.instances()) {
        auto should = get_new_attrs_from_somewhere(rsrc.name());
        rsrc.update(should);
      }

      // or do something to a specific resource
      auto rsrc = prov.find(some_name);
      attr_map should = { { "ensure", "absent" } };
      rsrc.update(should);

      // or create a new one
      auto rsrc = prov.create("new_one");
      auto should = get_new_attrs_from_somewhere_else();
      rsrc.update(should);

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
