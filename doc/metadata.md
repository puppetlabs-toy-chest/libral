# Provider metadata

Each provider must be able to describe itself. Descriptions are written in
YAML following the format defined in this document. A complete example can
be found [here](../examples/providers/metadata.yaml)

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
* `invoke`: the calling convention the provider uses. Must be either
  [simple](invoke-simple.md) or [json](invoke-json.md)
* `actions`: an array listing the actions this provider supports; possible
  entries in the array are `list`, `find` and `update`
* `suitable`: either `true` or `false` indicating whether the provider can
be used on the current system

In addition to this data, a provider also needs to describe its attributes
as defined in [this document](attributes.md).

<!--
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


 The `invocation_method` must be one of the supported calling
conventions, either [simple](invoke-simple.md) or [json](invoke-json.md).
-->
