# Writing native providers

The C++ provider interface is defined in
`lib/inc/libral/provider.hpp`. Note that the native interface is not
considered stable just yet; if you contribute a provider to the project, we
will make sure it is updated as we make changes to the provider API.

## Provider interface

The general interface for a provider is defined by the class
`libral::provider`. Specific providers need to implement three methods:

* `get`: retrieves some (or all) resources that the provider can manage and
  reports their current state
* `set`: enforces desired state for a number of resources
* `describe`: generates metadata about the provider and prepares the
  provider for use

Note that `libral` does not use exceptions. Instead, it uses the
`result<T>` class which either returns a `T` value on success or an `error`
object on failure.

```cpp

   // value is how libral represents attribute values
   // the real definition of a resource is a little more complicated than
   // this, but functions pretty much like a std::map
   using resource = std::map<std::string, value>

   // update bundles an is and a should state for a resource. It has some
   // additional convenience methods
   struct update {
     update(resource& is, resource& should);

     resource is;
     resource should;
   };

   class provider {
     result<std::vector<resource>>
     get(context &ctx,
         const& std::vector<std::string> names,
         const& std::map<std::string, value> config = { }) = 0;

     result<void>
     set(context &ctx,
         const &std::vector<update> updates) = 0;

     result<prov::spec> describe(environment &env) { };
   };
```

## Provider lifecycle

The lifecycle for providers written in C++ follows the outline below. This
applies only to C++ providers - external providers follow a much simpler
lifecycle which is described in the specification of the various calling
convention. The code below is not code you write, it's an approximation of
what `libral` does to achieve various purposes, and is only here to
illustrate how your provider will be used.

```cpp
    // The context and environment objects provide helpers and should be
    // the sole APIs that providers use to call back into libral
    libral::context ctx;
    libral::environment env;

    // Because reporting errors from constructors is so awkward, provider
    // constructors should do as little work as possible. Anything that
    // could fail should be done in the describe() method so that it can be
    // reported back properly
    some_provider prov();

    // A call to describe() must return metadata about the provider and
    // also initialize any provider internals. In particular, it must
    // indicate whether the provider is suitable or not. That means that
    // describe must check whether all its dependencies are met (e.g., that
    // all commands the provider wants to use are available)
    auto spec = prov.describe().ok();

    if (spec.suitable()) {
      // List all resources
      auto all = prov.get(ctx, { });

      // Find one resource
      auto one = prov.get(ctx, { "name" });

      // Find a few resources
      auto some = prov.get(ctx, { "name1", "name2", "name3" });

      // Find a resource with additional provider configuration
      auto one_conf = prov.get(ctx, { "name" }, { "target", "/etc/my_fstab" });

      // do something to a specific resource
      auto is = prov.get(ctx, { "some_name" })->front();
      auto should = prov.create(is.name());
      should["ensure"] = "absent";

      prov.set(ctx, { update(is, should) });

      // create a new one
      // get must return a resource with 'ensure' == 'absent' if the
      // resource does not exist yet, but could be created
      auto is = prov.get(ctx, { "some_name" })->front();
      auto should = prov.create(is.name());
      should["ensure"] = "present";
      should[...] = ...; // set more attributes

      prov.set(ctx, { update(is, should) });
    }
```
