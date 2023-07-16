{
    "targets": [
        {
            "target_name": "clipboard-event-windows",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": [
                "src/Api.cc",
                "src/ChildThread.cc",
                "src/ClipboardEvent.cc",
            ],
            'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
        }
    ]
}
