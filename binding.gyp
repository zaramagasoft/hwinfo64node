{
    "targets": [
        {
            "target_name": "hwAddon",
            "sources": ["src/hwAddon.cpp"],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "ExceptionHandling": "1",
                    "AdditionalOptions": ["/EHsc"],
                }
            },
        }
    ]
}