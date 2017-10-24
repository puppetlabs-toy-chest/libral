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

Providers can implement the following actions.

* `describe` - describe the provider by outputting YAML metadata
* `get` - list one or more resources
* `set` - update (or create or delete) one or more resources

You can run a provider from the command line with the following
invocations. The JSON files mentioned in the example must conform to the
inputs described for each action:

```bash
    > script.prov ral_action=describe
    # dumps YAML metadata

    > script.prov ral_action=get < get.json
    # lists resources

    > script.prov ral_action=set < set.json
    # update resources to the state given on stdin
```

### The describe action

The `describe` action is used to retrieve metadata about the provider. If
the provider `script.prov` is accompanied by a file `script.yaml`
containing the metadata, the `describe` action will never be invoked. If no
such YAML file exists, the provider must support the `describe` action.

When the describe action is invoked, `stdin` is empty and the provider
should not try to read from it.

The output from describe must be valid YAML as described in the
[provider metadata specification](metadata.md). The metadata must set
`provider.invoke` to `json`. Note that even though all other actions must
return JSON, the describe action must return a YAML document.

### The get action

When the `get` action is invoked, `stdin` contains a JSON object giving the
names of the resources that should be listed:

```json
    {
      "names": [ "name1", "name2", ..., "nameN" ]
    }
```

The `get` action must return at least the resources mentioned in `names`,
but may return more (or even all) resources:

```json
    {
      "resources": [
        { "name" : "name1", "attr1": "value11", "..." "attrM": "value1M" },
        { "name" : "name2", "attr1": "value21", "..." "attrM": "value2M" },
        "..."
        { "name" : "nameN", "attr1": "valueN1", "..." "attrM": "valueNM" },
      ]
    }
```

**FIXME**: what happens when one of the names does not correspond to a
valid resource, i.e., one that can not even be created ?

### The set action

When the `set` action is invoked, `stdin` contains the following JSON:
```json
    {
      "updates": [
        {
          "name": "the_name",
          "is" : {
            "attr1": "value1",
            "..."
            "attrN": "valueN"
          },
          "should" : {
            "attr1": "value1",
            "..."
            "attrN": "valueN"
          },
        },
        ...,
      ],
      "ral": {
        "noop": true|false
      }
    }
```

The `updates` array contains the `is` and the `should` state for each
resource that should be updated, both of which use the same format for the
resource as the `get` action. The `should` entry only lists the attributes
that need to actually be changed, and the provider will only be called if
there is at least one attribute that needs to be changed. Any attribute not
mentioned in `should` should be taken from the `is` resource if a full
resource needs to be constructed. The `is` resource reflects the latest
state that a previous `get` reported.

If the entry `ral.noop` is set to `true`, the provider should act as if it
were making changes without actually modifying anything. In particular, it
should produce the same output as setting `ral.noop` to `false` would.

The update action should produce the following output:
```json
    {
      "changes": [
        {
          "name": "the_name",
          "attr1" : { "is": "<is_value1>", "was": "<was_value1>" },
          "..."
          "attrN" : { "is": "<is_valueN>", "was": "<was_valueN>" },
        },
        ...
      ],
      "derive": true|false
    }
```

The `changes` array list all resources that were changed, which may be more
than the ones that were passed in to `set`. If the optional `derive` flag
is set to `true`, changes for resources that are not mentioned in the
`changes` array are derived from the differences in their `is` and `should`
entries.

If `ral.noop` was `true` in the input, no changes should be made to the
system, but the output should reflect the changes that _would_ have been
made.

If the resource does not exist, and can not be created, the entry in the
changes array should be

```json
    {
      "changes" : [
        {
          "name": "foobar",
          "error" : {
            "message" : "the resource named 'foobar' could not be created",
            "kind": "unknown"
          }
        }
      ]
    }
```

In general, `set` should try and perform as many updates as possible, and
note any resources that can not be updated with an error as described
above. If the overall `set` action can not be performed, for example,
because the user does not have permission to make any changes, the response
should simply be:

```json
    {
      "error": {
        "message": "user does not have permission to make changes",
        "kind": "forbidden"
      }
    }
```

### Error reporting

Errors are indicated by having an `error` key in the response. If the
`error` key is set at the toplevel, the rest of the output is disregarded,
and it is assumed that the action failed. If the `resources` or `changes`
arrays returned by `get` and `set` contain entries that have the `error`
key set, it is assumed that there was a problem with the corresponding
resource. In either case, the `error` entry must look like

```json
    {
      "error": {
        "message": "<human-readable message about what happened>",
        "kind": "<error_kind>"
      }
    }
```

The error kind must be one of the following:
* `unknown`: the resource targetted in a `get` or `set` action does not
  exist and can not be created. (If it could be created, the resource
  should carry an attribute `ensure` with value `absent`)
* `forbidden`: the current user does not have permission to carry out the
  action for this resource
* `failed`: a general failure to perform the action

Even if the provider encounters an error, it still needs to exit with
status code 0. Exiting with any other status code will be taken as an
indication that the provider encountered a fatal error and all of its
output will be disregarded.
