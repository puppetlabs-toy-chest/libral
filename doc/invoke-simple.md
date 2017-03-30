# The `simple` calling convention

A script that follows the `simple` calling convention must, at a minimum,
be able to describe itself and list all resources of a certain kind. In
addition, it may be able to find a resoure by name, or update a resource.

When a `simple` script is invoked, it receives a number of key/value pairs
as command line arguments, i.e. if the script is called `script.prov`,
`libral` will invoke it as

```bash
script.prov var1=val1 var2=val2 var3=val3
```
### Special variables

A few variables have special meaning. All special variables will have a
name starting with `ral_`:

* `ral_action`: the action the provider should perform. This variable will
  always be passed
* `ral_noop`: whenever this argument is passed, regardless of its value,
the provider should only determine if changes would need to be made without
actually making them.

The other variables that are passed in will have the same name as the
attributes of the resource.

### stdio

When the script is invoked, file descriptos are set up in the following
ways:

* `stdin`: empty
* `stdout`: will be read by `libral`; all output described here should go
there
* `stderr`: any output on stderr will be logged. The log level can be
specified by prefixing the line with `LEVEL:`. Possible levels are `debug`,
`info`, `warn`, and `error`; the log level defaults to `warn`

### Environment

**FIXME**: what environment variables will be set ? What gets scrubbed ?
(All but PATH and HOME ?)

## Actions

Providers can implement the following actions. Only `describe` must be
supported.

* `describe` - describe the provider by outputting YAML metadata
* `list` - list all instances
* `find name=NAME` - find the instance named `NAME`
* `update name=NAME A1=V1 ... AN=VN` - update (or create or delete)
  resource. This action is only called if at least one attribute needs to
  be changed from the value reported by `find name=NAME`, and only
  attributes that have to be changed will be passed to `update`.

You can run a provider from the command line with the following
invocations:

```bash
    > script.prov ral_action=describe
    # dumps YAML metadata

    > script.prov ral_action=list
    # lists all resources

    > script.prov ral_action=find name=foo
    # list the resource named FOO

    > script.prov ral_action=update name=foo ensure=present attr1=val1 attr2=val2
    # update the resource to the state given by ensure, attr1, and attr2
    # and report what had to be changed
```

### Output from describe

The output from describe must be valid YAML. The simplest metadata that
could be returned is

```yaml
---
provider:
  type: service
  invoke: simple
  actions: [list,find,update]
  suitable: true
```

The entries under `provider` have the following meaning:

* `type`: the name of the provider's underlying type
* `invoke`: the calling convention the provider uses. Must be `simple` for
  providers that use the calling convention described in this document
* `actions`: an array listing the actions this provider supports
* `suitable`: either `true` or `false` indicating whether the provider can
be used on the current system

#### Digression on suitable/default (later)

**FIXME**: we really want to make it possible for people to write boolean
expressions for `suitable`. Those expressions should have access to facts,
and some simple notation for discovering some commands, so that people can
write

```yaml
provider:
  suitable:
    - command(which)
    - command(something_else)
    - osfamily == 'redhat'
```

Similarly, we want to make it possible to indicate whether a provider
should be the default by offering a `default` key that can contain similar
expressions, so that one could write
```yaml
provider:
  default:
    - osfamily = 'archlinux'
    - osfamily = 'redhat' and operatingsystemmajrelease = '7'
    - osfamily = 'redhat' and operatingsystem = 'fedora'
    - osfamily = 'suse'
    - bla bla bla
```

Note that for 'suitable' the different clauses are connected with an 'and',
whereas for default they are connected by 'or'.

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

The first line of the output must be the literal string `# simple` with
nothing else on the line.

Any line `name: NAME` in the output is taken to indicate a new resource and
the following lines provide the attributes of that resource until the next
line `name: NAME` is encountered.

When porcessing, each line is first stripped of leading and trailing
whitespace, anything up to the first ':' is considered the name of the
property, and anything that follows up to the end of the line, except for
leading whitespace, is considered the value of the property

### Output from find

    # simple
    name: the_name
    attr1: value1
    ...
    attrN: valueN

Same format as `list`, but just one resource. If the resource does not
exist (and could not possibly be created with something like
`ensure=present`), the output should be

    # simple
    name: the_name
    ral_unknown: true

### Output from update

    # simple
    name: the_name
    attr1: new_value1
    ral_was: old_value1
    attr2: new_value2
    ral_was: old_value2
    ...
    attrN: new_valueN
    ral_was: old_valueN

The output should only contain attributes that actually had to be changed,
and only needs to include attributes for which the value was changed to
something other than the value passed on the command line. For each such
attribute the new value, and the old value prefixed by `ral_was`, need to
be output. For any attribute that had to be changed bceause it was passed
on the command line, and for which nothing is output, `libral` assumes it
was changed exactly from the last value reported by `find` to the exact
value passed on the command line. That means that in the simplest case, the
output from `update` can just be

    # simple

If the variable `ral_noop` was passed to the provider, no changes should be
made to the system, but the output should contain all changes that _would_
have been made.

If the resource does not exist, and can not be created, the output should
be

    # simple
    name: the_name
    ral_unknown: true

### Error reporting

If the output contains any lines of the form

    ral_error: some message
    ...
    ral_eom

`libral` assumes that the provider operation encountered an
error. Everything from the leading `ral_error:` up to a single line
containing `ral_eom` will be used as the error message.

If the output contains a `ral_error:` line, `libral` will disregard
anything else in the output.

Even if the script encounters an error, it still needs to exit with status
code 0. Exiting with any other status code will be taken as an indication
that the provider encountered a fatal error.

## Getting at the command line arguments

When the provider is invoked, care is taken that the arguments are quoted
appropriately. Roughly speaking, the provider is invoked like this:

```bash
script.prov ral_action=something "arg1='value1'" "arg2='value2 spaces'" ...
```

The following list shows how to most easily turn these arguments into
values that the provider script can use, depending on the language in which
the script is implemented:

If the provider is written in Bash, use the following
```bash
eval "$@"
# all arguments are now shell variables
echo "Action $ral_action"
```

If the provider is written in Python, use
```python
import shlex
import sys

args=dict(x.split('=') for x in shlex.split(" ".join(sys.argv[1:])))
print args
```

If the provider is written in Ruby, use
```ruby
require 'shellwords'

args = Hash[Shellwords.shellsplit(ARGV.join(' ')).map { |x| x.split("=") }]
p args
```
