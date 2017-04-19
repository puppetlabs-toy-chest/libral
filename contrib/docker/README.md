Files to help with running builds inside Docker containers. They can all be
built with `make NAME`.

Make targets that end in `-build` create containers that will build libral
automatically when they are run. They have the necessary tooling to do that
in them, but libral sources will be downloaded every time they are run.

Make targets that are just a distro nmae like `f25` will create a container
image that has a prebuilt libral in them and can be used for trying it out.
