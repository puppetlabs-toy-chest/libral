# Libral

Libral is a systems management library that makes it possible to query and
modify system resources (files, packages, services, etc.) through a
desired-state API. It's goals are:

* Provide a management API for UNIX-like systems to query and modify
  system resources
* Make managing new kinds of resources very simple; in particular, there is
  no need to write native code to add new providers and they can be
  arbitrary scripts that adhere to one of a number of simple calling
  conventions
* Express both current state and desired state in a simple, unified form
* Guarantee that all changes are done idempotently, by enforcing desired
  state only when changes are needed
* Have a very small default footprint enabling use of `libral` in
  resource-constrained environments such as devices or containers
* Be versatile enough to serve as the basis of more extensive configuration
  management systems, such as
  [Puppet](https://github.com/puppetlabs/puppet/) without being directly
  tied to any one of them.

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

After you built `libral` or after you downloaded and unpacked the
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz)
you can try things out by running `ralsh`:

```bash
    # If you downloaded the precompiled tarball
    alias ralsh=$TARBALL_LOCATION/ral/bin/ralsh

    # Only if you built libral from source
    export RALSH_DATA_DIR=$LIBRAL_CHECKOUT/data
    alias ralsh=$LIBRAL_CHECKOUT/bin/ralsh

    # list available types
    ralsh
    # list all instances of a type
    ralsh mount
    # list a specific instance
    ralsh service crond
    # make a change for the better
    ralsh service crond ensure=stopped
```

### Running inside a container

The
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz)
contains a statically linked `ralsh` and all supporting files in it. After
unpacking the tarball, you can copy it into a container and run it like
this:

```bash
    CONTAINER=<some_container>
    docker cp $TARBALL_LOCATION/ral $CONTAINER:/tmp
    docker exec $CONTAINER /bin/sh -c /tmp/ral/bin/ralsh
    docker exec $CONTAINER /bin/sh -c '/tmp/ral/bin/ralsh user root'
```

## Writing providers

What resources `libral` can manage is determined by what providers are
available. Some providers are built in and implemented in C++, but doing
that is of course labor-intensive and should only be done for good
reason. It is much simpler, and recommended, that new providers first be
implemented as external providers. External providers are nothing more than
scripts or other executables that follow one of `libral`'s calling
conventions. The different calling conventions trade off implementation
complexity for expressive power.

The following calling conventions are available. If you are just getting
started with `libral`, you should write your first providers using the
`simple` calling convention. For now, all providers need to be placed into
`data/providers/` and be executable files whose name ends in `.prov`.

* [simple](doc/invoke-simple.md)
* `json` (planned): input/output via JSON
* `json_batch` (planned,maybe): input/output via JSON, can operate on multiple resources at once
* [native](doc/invoke-native.md)

For all of these, you will also want to read up on
[how to specify resource attributes](doc/attributes.md)

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
