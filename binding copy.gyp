{
  "targets": [
    {
      "target_name": "hwAddon",
      "sources": ["src/hwAddon.cpp"],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "conditions": [
        ["OS=='win'", {
          "defines": ["NAPI_CPP_EXCEPTIONS"]
        }]
      ]
    }
  ]
}
