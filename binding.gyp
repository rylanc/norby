{
  "targets": [
    {
      "target_name": "ruby_bridge",
      "sources": [
        "src/ruby.cpp",
        "src/common.cpp",
        "src/RubyObject.cpp"
      ],
      "include_dirs": [
        "<!(ruby -e 'puts RbConfig::CONFIG[\"rubyhdrdir\"]')",
        "<!(ruby -e 'puts RbConfig::CONFIG[\"rubyhdrdir\"] + \"/\" + RbConfig::CONFIG[\"arch\"]')"
      ],
      "library_dirs": [
        "<!(ruby -e 'puts RbConfig::CONFIG[\"libdir\"]')"
      ],
      "libraries": [
        "<!(ruby -e 'puts RbConfig::CONFIG[\"LIBRUBYARG\"]')",
        "-ldl"
      ]
    }
  ]
}
