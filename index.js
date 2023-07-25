const setClipboardListener = require('./build/Debug/clipboard-event-windows.node')
console.log(setClipboardListener);
let count = 0

setClipboardListener(() => {
    console.log('update');
    // setClipboardListener(() => {
    //     console.log('foo')
    // })
});

// setClipboardListener();


globalThis.setClipboardListener = setClipboardListener;
