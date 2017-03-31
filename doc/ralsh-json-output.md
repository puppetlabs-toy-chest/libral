# Format of ralsh's JSON output

When you run `ralsh --json`, it will produce JSON output meant for use by
other tools and programs. This page describes what you can find in that
output.

## Provider listing

When you run `ralsh --json`, it produces a list of the providers it knows
about. The output object contains one key `providers` whose content is an
array of objects, each object describing one provider. The description of a
provider has the following fields:

* `name`: the qualified name of the provider
* `type`: the name of the type to which this provider belongs
* `source`: the source for the provider, either `builtin` for C++ providers
  or the path of the provider script for external providers
* `suitable`: a boolean indicating whether the provider can be used on this
  system; curently, this will always be `true` as unsuitable providers are
  not loaded by `ralsh` (though that might change in the future)
* `attributes`: an array of objects describing each of the provider's
  attributes, containing the `name`, `desc`, `type`, and `kind` for the
  attribute as detailed in the [resource attributes](./attributes.md)
  specification

An example of this output looks like
```json
{
  "providers": [
    {
      "name": "service::sysv",
      "type": "service",
      "source": "/usr/share/libral/data/providers/sysv.prov",
      "suitable": true,
      "desc": "",
      "attributes": [
        {
          "kind": "rw",
          "type": "enum[true, false]",
          "desc": "(missing description)",
          "name": "enable"
        },
        {
          "kind": "rw",
          "type": "enum[running, stopped]",
          "desc": "(missing description)",
          "name": "ensure"
        },
        {
          "kind": "rw",
          "type": "string",
          "desc": "(missing description)",
          "name": "name"
        }
      ]
    }
  ]
}
```

## Resource listing

When you run `ralsh --json <PROVIDER>`, ralsh prints a list of all
resources managed by `PROVIDER`. The output object contains one key
`resources` whose content is an array of objects, each object describing
one resource. Those objects have the following fields:

* one field for each attribute, using the attribute's name as the key and
  the attribute's value as the value
* a field `ral` carrying an object with some metainformation; currently
  that object contains the entries `type` with the resource's type, and
  `provider` with the fully-qualified name of the provider.

## Resource find

When you run `ralsh --json <PROVIDER> <NAME>`, ralsh prints the resource
`NAME` managed by `PROVIDER`. The output object contains one key `resource`
whose content is that resource, in the same format as that used when
listing resources.

An example of this output looks like
```json
{
  "resource": {
    "ral": {
      "provider": "service::sysv",
      "type": "service"
    },
    "ensure": "stopped",
    "enable": "false",
    "name": "smartd"
  }
}
```

**NOTE**: it's silly to have a different output format. Maybe this should
just return a `resources` array with a single resource in it.

## Resource update

When you run `ralsh --json <PROVIDER> <NAME> A1=V1 A2=V2`, ralsh prints the
result of ensuring that the resource `NAME` managed by `PROVIDER` is in the
desired state expressed by the `An=Vn` attribute changes. The output object
contains two keys:

* `resource` showing the state after the changes have been made, in the
  same format as is used when finding a resource
* `changes`, an array detailing the changes that had to be made to each
  attribute to bring the resource into that state. Only attributes that had
  to be changed are listed here. Each entry in the array
  is an object with these keys:
  * `attr`: the name of the attribute that was changed
  * `is`: the current state of the attribute, i.e., the state it is in
    after the change
  * `was`: the previous state of the attribute, i.e., the state it was in
    before the change

An example of this output looks like
```json
{
  "changes": [
    {
      "was": "stopped",
      "is": "running",
      "attr": "ensure"
    }
  ],
  "resource": {
    "ral": {
      "provider": "service::sysv",
      "type": "service"
    },
    "ensure": "running",
    "enable": "false",
    "name": "smartd"
  }
}
```
**NOTE**: it is likely that this format will be changed to an array of
   `changes`/`resource` pairs to accomodate providers where enforcing the
   state of one resource actually touches many resources (like recursive
   file operations)
