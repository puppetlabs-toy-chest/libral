# Writing native providers

The C++ provider interface is defined in
`lib/inc/libral/provider.hpp`. Note that the native interface is not
considered stable just yet; but if you contribute a provider to the
project, we will make sure it is updated as we make changes to the provider
API.

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
  about system details (like facts) and some settings
