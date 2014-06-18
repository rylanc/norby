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
