class SshAuthorizedKey < RalSuite

  USER=ENV["USER"]

  # FIXME: how do we check for validation errors ? E.g.,
  # - absent/present without a user
  # - present without a key
  # we want to perform such a test and have it pass when the test returns
  # an error that matches some pattern

  provider "ssh_authorized_key", :root => false

  def setup
    no_test = resource("test", "ensure" => "absent", "user" => USER )
    set(no_test)
  end

  def test_get_nonexistent_resource
    # FIXME: we need to pass a user or target, but the internal plumbing
    # currently doesn't do anything with that, so we get lucky in that
    # the provider scans lots of files. This should really be an error
    res = get("test")
    # The provider could create a 'test' resource, but it currently doesn't
    # exist so 'get' needs to return it with ensure=absent
    assert_absent "test", res
  end

  def test_create_delete_without_options
    tst = resource "test",
        "ensure" => "present",
        "type" => "ssh-rsa",
        "user" => USER,
        "key" => "YOLO"
    res = set(tst)
    assert_equal 1, res.size
    assert_present "test", res[0].should
    # not true because get fills in attrs like options and target
    # assert_equal [res[0].resource], get(tst)
  end

  def test_create_update_options
    tst = resource "test",
        "ensure" => "present",
        "type" => "ssh-rsa",
        "user" => USER,
        "key" => "YOLO"
    set(tst)

    no_tst = resource("test", { "ensure" => "absent", "user" => USER })
    res = set(no_tst)
    assert_equal 1, res.size

    res = get(tst.name)
    assert_equal 1, res.size
    # FIXME: get can't fill the user or target attribute for absent
    # resources because we don't pass that to get
    assert_equal no_tst.delete("user"), res[0]
  end

  def test_create_with_options
    tst = resource "test",
        "ensure" => "present",
        "options" => ["command=/bin/cmd arg", "no-pty"],
        "type" => "ssh-rsa",
        "key" => "YOLO",
        "user" => USER
    set(tst)

    res = get(tst.name)
    assert_equal 1, res.size
    assert_equal tst["options"], res[0]["options"]
  end
end
