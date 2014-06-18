{
  "targets": [
    {
      "target_name": "ruby",
      "sources": [
        "ruby.cpp",
        "common.cpp",
        "RubyObject.cpp"
      ],
      "include_dirs": [
        "<!(ruby -e 'puts RbConfig::CONFIG[\"rubyhdrdir\"]')",
        "<!(ruby -e 'puts RbConfig::CONFIG[\"rubyarchhdrdir\"]')"
      ],
      "library_dirs": [
        "<!(ruby -e 'puts RbConfig::CONFIG[\"libdir\"]')"
      ],
      "libraries": [
        "-lruby",
        "-ldl"
      ]
    }
  ]
}
