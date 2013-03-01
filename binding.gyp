{
  "variables": {
    "node_shared_openssl%": 'true'
  },
  "targets": [
    {
      "target_name": "sencillo",
      "include_dirs": [
        'deps/libgit2/include',
        'deps/v8-convert'
      ],
      "sources": [ "src/binding.cc"
      , "src/common.cc"
      , "src/error.cc"
      , "src/message.cc"
      , "src/object.cc"
      , "src/oid.cc"
      , "src/reference.cc"
      , "src/repository.cc"
      ],

      "libraries": [
        "<(module_root_dir)/deps/libgit2/build/libgit2.a",
        "-lssl"
      ],

      # Enable exceptions, required by V8U (see TooTallNate/node-gyp#17)
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }],
        ['node_shared_openssl=="false"', {
          # so when "node_shared_openssl" is "false", then OpenSSL has been
          # bundled into the node executable. So we need to include the same
          # header files that were used when building node.
          'include_dirs': [
            '<(node_root_dir)/deps/openssl/openssl/include'
          ],
          "conditions" : [
            ["target_arch=='ia32'", {
              "include_dirs": [ "<(node_root_dir)/deps/openssl/config/piii" ]
            }],
            ["target_arch=='x64'", {
              "include_dirs": [ "<(node_root_dir)/deps/openssl/config/k8" ]
            }],
            ["target_arch=='arm'", {
              "include_dirs": [ "<(node_root_dir)/deps/openssl/config/arm" ]
            }]
          ]
        }]
      ]
    }
  ]
}
