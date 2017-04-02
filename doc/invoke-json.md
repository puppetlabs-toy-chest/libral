# The `json` calling convention

A script that follows the `json` calling convention must, at a minimum, be
able to describe itself. To actually be useful, it should also support
additional actions such as listing resources, finding them, and updating
them.

When a `json` script is invoked, it receives one command line argument, the
action that should be performed. Any other input will be on `stdin` as a
JSON object. The action will be passed in in the form
`ral_action=<action>`, where `<action>` can be one of `describe`, `list`,
`find`, or `update`.

### stdio

When the script is invoked, file descriptos are set up in the following
ways:

* `stdin`: JSON-encoded arguments for the action (except for `describe`,
  see below)
* `stdout`: will be read by `libral`; all output described here should go
there
* `stderr`: any output on stderr will be logged. The log level can be
specified by prefixing the line with `LEVEL:`. Possible levels are `debug`,
`info`, `warn`, and `error`; the log level defaults to `warn`

### Environment

**FIXME**: what environment variables will be set ? What gets scrubbed ?
(All but PATH and HOME ?)

**FIXME**: explain how data types are mapped from libral's type system to
  JSON

## Actions

Providers can implement the following actions. Only `describe` must be
supported.

* `describe` - describe the provider by outputting YAML metadata
* `list` - list all resources
* `find` - find a specific resource
* `update` - update (or create or delete) a resource

You can run a provider from the command line with the following
invocations. The JSON files mentioned in the example must conform to the
inputs described for each action:

```bash
    > script.prov ral_action=describe
    # dumps YAML metadata

    > script.prov ral_action=list < list.json
    # lists all resources

    > script.prov ral_action=find < find.json
    # list the resource named FOO

    > script.prov ral_action=update < update.json
    # update the resource to the state given on stdin
```

### The describe action

When the describe action is invoked, `stdin` is empty and the provider
should not try to read from it.

The output from describe must be valid YAML as described in the
[provider metadata specification](metadata.md). The metadata must set
`provider.invoke` to `json`. Note that even though all other actions must
return JSON, the describe action must return a YAML document.

### The find action

When the `find` action is invoked, `stdin` contains the following JSON:

```json
    {
      "resource" : {
        "name" : "the_name"
      }
    }
```

The `find` action must produce the following output:

```json
    {
      "resource" : {
        "name"   : "the_name",
        "attr1"  : "value1",
        "..."
        "attrN"  : "valueN"
      }
    }
```

### The list action

When the `list` action is invoked, `stdin` contains an empty JSON object
`{}`.

The `list` action must produce the following output

```json
    {
      "resources": [
        { "name" : "name1", "attr1": "value1", "..." "attrN": "valueN" },
        { "name" : "name1", "attr1": "value1", "..." "attrN": "valueN" },
        "..."
        { "name" : "name1", "attr1": "value1", "..." "attrN": "valueN" },
      ]
    }
```

Note that the format for each resource in the `resources` array is the same
as what `find` must return for its single `resource` key.

### The update action

When the `update` action is invoked, `stdin` contains the following JSON:
```json
    {
      "resource": {
        "name": "the_name",
        "attr1": "value1",
        "..."
        "attrN": "valueN"
      },
      "ral": {
        "noop": true|false
      }
    }
```

The `resource` entry contains the name of the resource that should be
updated, together with all the attributes whose state should be
enforced. The provider must make sure that it only updates attributes
mentioned in the `resource` object, but not ones that are not mentioned.

If the entry `ral.noop` is set to `true`, the provider should act as if it
were making changes without actually modifying anything. In particular, it
should produce the same output as setting `ral.noop` to `false` would.

The update action should produce the following output:
```json
    {
      "changes": {
        "attr1" : { "is": "<is_value1>", "was": "<was_value1>" },
        "..."
        "attrN" : { "is": "<is_valueN>", "was": "<was_valueN>" },
      }
    }
```

The `changes` hash should only mention attributes that were actually changed.

If `ral.noop` was `true` in the input, no changes should be made to the
system, but the output should contain all changes that _would_ have been
made.

If the resource does not exist, and can not be created, the output should
be

```json
    {
      "error" : {
        "message" : "the resource named 'foobar' could not be found",
        "kind": "unknown"
      }
    }
```

### Error reporting

If the output contains an `error` key, the rest of the output is
disregarded, and it is assumed that the action failed. An error response
looks like

```json
    {
      "error": {
        "message": "<human-readable message about what happened>",
        "kind": "<error_kind>"
      }
      "... anything else is ignored ..."
    }
```

The error kind must be one of the following:
* `unknown`: the resource targetted in a `find` or `update` action does not
  exist.
* `failed`: a general failure to perform the action

Even if the providers encounters an error, it still needs to exit with status
code 0. Exiting with any other status code will be taken as an indication
that the provider encountered a fatal error.
