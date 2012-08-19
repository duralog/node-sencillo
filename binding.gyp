{
  "targets": [
    {
      "target_name": "gitteh",
      "sources": [ "src/binding.cc" ],
      "include_dirs": [ 'deps/libgit2/include',
                        'deps/libgit2/deps/http-parser',
                        'deps/libgit2/deps/zlib',
                        'deps/libgit2/deps/regex' ],

      # Enable exceptions (see TooTallNate/node-gyp#17)
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
