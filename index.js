const setClipboardListener = require('./build/Debug/clipboard-event-windows.node')
console.log(setClipboardListener);
let count = 0

setClipboardListener(() => {
    console.log('update');
    // setClipboardListener();
    // setClipboardListener(() => {
    //     console.log('foo')
    // })
});

globalThis.setClipboardListener = setClipboardListener;

setInterval(() => {

}, 1000);