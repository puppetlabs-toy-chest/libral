# Resource attributes

Every provider defines, as part of its metadata, what attributes it has
and, for each of them, their data type and whether that attribute can be
read or written.

There is one attribute that every resource needs to have: `name`, which
must be of type `string`. This attribute can not be changed through
`update`. It serves as a primary key for the resource among all the
resources managed by a specific provider.

## Specifying provider attributes

Provider attributes are specified in YAML (TODO: clarify where that YAML
comes from) in the following way:

```yaml
---
provider:
  name: <provider_name>
  type: <type_name>
  desc: <description of what the provider does>
  attributes:
    name:
      desc: <description of name in the context of this provider>
    <attr1>:
      desc: <description of attr1>
      type: <data type>
      kind: <attr kind>
    ...
    <attrN>:
      desc: <description of attrN>
      type: <data type>
      kind: <attr kind>
```

### The data type
The `<data type>` of the attributes uses a small subset of the [Puppet 4
type system](https://docs.puppet.com/puppet/latest/lang_data_type.html). Currently
supported are the following data types:

* `string`: a string of characters
* `boolean`: either `true` or `false`
* `array[string]`: an array of strings (note that only arrays of strings
  are currently supported)
* `enum[<option>, (<option>, ...)]`: a string that is exactly one of the
  `<option>`

If no `type` is given for an attribute, the type defaults to `string`

### The kind

The `<attr kind>` indicates whether an attribute can be read or
written. The possible values are:

* `r`: the attribute is readonly; such an attribute will be reported back
  when reading a resource via `instances` or `find`, but can not be changed
  with `update`
* `w`: the attribute can only be written; that means that such an attribute
  will not be reported from `instances` or `find`, but can be changed with
  `update`
* `rw`: the attribute can both be read and written

If no `<attr_kind>` is provided, it defaults to `rw`.

*Note*: the idea behind introducing a 'kind' (terrible name) is to ease
some of the confusion between property and parameter, which correspond to
`rw` and `w` kinds, and provide a clean way to also have readonly
attributes

## TODO

* we need a way to indicate mandatory vs optional attrs (for writing)
* we use `type` both for the data type and the provider type
* we will, at some point, need to support more than these types; though it
  does not make sense to support the full Puppet type system as some of it
  only applies to catalogs etc. and there is no notion of that in `libral`
* how strict should checking be ? In some cases, it's preferrable to allow
  passing readonly attributes to `update` (so you can do `find`/change a
  couple attributes/`update`) but it might also lead people astray when
  they try to write a file's `mtime` and nothing happens.
