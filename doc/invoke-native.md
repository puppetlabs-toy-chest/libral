# Writing native providers

The C++ provider interface is defined in
`lib/inc/libral/provider.hpp`. Note that the native interface is not
considered stable just yet; but if you contribute a provider to the
project, we will make sure it is updated as we make changes to the provider
API.

## Provider interface

The general interface for a provider is defined by the class
`libral::provider` and has the following important methods:

```cpp

   // value is how libral represents attribute values
   using resource = std::map<std::string, value>

   struct update {
     update(const& resource is, const& resource should);

     resource is;
     resource should;
   };

   class provider {
     result<bool> suitable(runtime &rt);

     result<std::vector<resource>>
     get(runtime &rt,
         const& std::vector<std::string> names,
         const& std::map<std::string, value> config = { }) = 0;

     result<void>
     set(runtime &rt,
         const &std::vector<update> updates) = 0;

     result<void> flush(runtime &rt) { };

     result<void> close(runtime &rt) { };
   };
```

## Provider lifecycle

The lifecycle for providers written in C++ follows the outline below. This
applies only to C++ providers - external providers follow a much simpler
lifecycle which is described in the section above for each calling
convention.

```cpp
    // The runtime object provides helpers for providers that they can use
    // to interact with their environment.
    libral::runtime rt;

    some_provider prov();

    // A call to suitable() must also initialize any provider internals.
    // Once suitable() returns true, the provider must be ready to use.
    // The suitable method most call rt.register(metadata) where metadata
    // is the provider metadata
    if (prov.suitable(rt)) {
      // List all resources
      auto all = prov.get(rt, { });

      // Find one resource
      auto one = prov.get(rt, { "name" });

      // Find a few resources
      auto some = prov.get(rt, { "name1", "name2", "name3" });

      // Find a resource with additional provider configuration
      auto one_conf = prov.get(rt, { "name" }, { "target", "/etc/my_fstab" });

      // do something to a specific resource
      auto rsrc = prov.get(rt, { "some_name" });
      attr_map should = { { "ensure", "absent" } };
      prov.set(rt, { update(rsrc, should) });

      // create a new one
      auto creat = get_new_attrs_from_somewhere_else();
      prov.set(rt, { update(nullptr, creat) });

      // make sure all changes have been written to disk
      prov.flush(rt);

      // not sure yet if we need an explicit 'close' call
      prov.close(rt)
    }
```

## Runtime interface

```cpp

    class runtime {

    };
