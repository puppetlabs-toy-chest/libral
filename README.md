# Libral

[![Build Status](https://travis-ci.org/puppetlabs/libral.svg?branch=master)](https://travis-ci.org/puppetlabs/libral)

Libral is a systems management library that makes it possible to query and
modify system resources (files, packages, services, etc.) through a
desired-state API. Its goals are to:

* Provide a management API for UNIX-like systems to query and modify
  system resources
* Make managing new kinds of resources very simple; in particular, there is
  no need to write native code to add new providers and they can be
  arbitrary scripts that adhere to one of a number of simple calling
  conventions
* Express both current state and desired state in a simple, unified form
* Guarantee that all changes are done idempotently by enforcing desired
  state only when changes are needed
* Have a very small default footprint that enables use of `libral` in
  resource-constrained environments such as devices or containers
* Be versatile enough to serve as the basis of more extensive configuration
  management systems, such as
  [Puppet](https://github.com/puppetlabs/puppet/), without being directly
  dependent on any one of them.

## Getting in touch

* Mailing list: [libral](https://groups.google.com/group/libral)
* IRC: `irc.freenode.net:#libral`

## Building and installation

If you just want to quickly try out `libral`, you can download a
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz)
([GPG signature](http://download.augeas.net/libral/ralsh-latest.tgz.sig))
that should work on any Linux machine that has glibc 2.12 or later. (It
might actually work with glibc 2.8 or later.) If you succeed in that, please
let [us](mailto:libral@googlegroups.com) know.

We have reports of the precompiled binaries working on 64-bit Fedora 21-25,
RHEL 5-7 (and related distributions like CentOS), Debian 6-9,
Ubuntu 10.04-16.10, and SLES 11-12. The binaries will not work on
32-bit and pre-glibc 2.8 systems: RHEL 4, SLES 10, ...

In case you do need to build from source, which is not required for
provider development, only if you want to work on the core libral library,
[this document](HACKING.md) contains instructions on building `libral`.


### Docker

You can also try out `libral` in the context of a Docker container. The
Dockerfile in the repository allows for building an image quickly based
on the above mentioned precompiled tarball.

```
docker build -t puppet/libral .
```

Running this can then be done with Docker, for instance the following
invocation will launch `ralsh` in the context of the container.

```
docker run --rm -t puppet/libral
```

This is intended for exploring the CLI and experimenting with providers.


## Usage

After you build `libral`, or after you download and unpack the
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz),
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

    # Do the same against a remote system to which you have ssh access
    ralsh -t ahost
    ralsh -t ahost package sudo
```

The default output from `ralsh` is meant for human consumption and looks a
lot like Puppet. It is also possible to have `ralsh` produce
[JSON output](doc/ralsh-json-output.md) by passing the `--json` flag.

Many of the providers that `libral` knows about are separate
scripts. `ralsh` searches them in the following order. In each case, the
providers must be executable scripts in a subdirectory `providers` in the
mentioned directory ending in `.prov`:

* if the environment variable `RALSH_DATA_DIR` is set, look in the
  directory this variable is set to
* if the `--include` option to `ralsh` is given, look in that directory
  (the option can be given multiple times)
* default to a directory determined at build time, by default
  `/usr/share/libral/data`

### Running inside a container

The
[precompiled tarball](http://download.augeas.net/libral/ralsh-latest.tgz)
contains a statically linked `ralsh` and all supporting files. After
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
that is of course labor intensive and should only be done for good
reason. It is much simpler, and recommended, that new providers first be
implemented as external providers. External providers are nothing more than
scripts or other executables that follow one of `libral`'s calling
conventions. The different calling conventions trade off implementation
complexity for expressive power.

The following calling conventions are available. If you are just getting
started with `libral`, you should write your first providers using the
`simple` or `json` calling convention:

* [simple](doc/invoke-simple.md)
* [json](doc/invoke-json.md): input/output via JSON
* `json_batch` (planned,maybe): input/output via JSON, can operate on multiple resources at once
* [native](doc/invoke-native.md)

For all of these, you will also want to read up on the
[metadata](doc/metadata.md) that each provider needs to produce to describe
itself.

To start a new provider, follow these steps:

1. Decide on some working directory `DIR`, run `mkdir $DIR/providers`, and
2. Create a file `$DIR/providers/myprovider.prov` and make it executable
3. Make sure that running `myprovider.prov ral_action=describe` returns
   valid [provider metadata](doc/metadata.md), especially a valid type. For
   now it doesn't matter what the type is, let's call it `MYTYPE`
4. Run `ralsh -I $DIR $MYTYPE`. This will ask the provider to list all
   instances of the type using the `list` action. Get that working
5. Run `ralsh -I $DIR $MYTYPE NAME`. This will ask the provider to find a
   resource with name `NAME` using the `find` action. Get that working,
   too.
6. Run `ralsh -I $DIR $MYTYPE NAME ATTR=VALUE...`. This will ask the
   provider to update the resource `NAME` using the `update` action.
7. You're done; your provider is ready for a pull request here ;)

## Todo list

- [X] finish mount provider
- [X] add a shell provider
- [X] event reporting on update
- [X] simple type system and checking for provider attributes
- [X] produce help/details about providers
- [X] add a [json calling convention](doc/invoke-json.md) and use it in a provider
- more core providers
  - [ ] cron
  - [X] file
  - [X] group (groupXXX)
  - [X] host
  - [ ] package (besides dnf and yum)
  - [ ] service (besides systemd, upstart and sysv)
  - [X] user (userXXX)
- even more core providers
  - [ ] interface
  - [ ] k5login
  - [ ] mailalias
  - [ ] selboolean
  - [ ] selmodule
  - [ ] sshkey
  - [X] ssh-authorized-key
  - [ ] vlan
  - [ ] yumrepo
- [ ] add a remote provider (using an HTTP API)
- [ ] adapt providers to multiple OS (maybe using mount)
- [X] add support for running providers over ssh (see [this PR for details](https://github.com/puppetlabs/libral/pull/67))
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
- Would it be better to make providers responsible for event generation
  rather than doing that in the framework ?
