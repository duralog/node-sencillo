{
  "targets": [
    {
      "target_name": "gitteh",
      "sources": [ "src/binding.cc" ],
      "include_dirs": [ 'deps/libgit2/include' ],

      "libraries": [
        "<(module_root_dir)/deps/libgit2/build/libgit2.a"
      ],

      # Enable exceptions, required by V8U (see TooTallNate/node-gyp#17)
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
