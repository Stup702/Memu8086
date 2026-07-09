const createMemu8086 = require('./out/memu8086_core.js');
const path = require('path');
createMemu8086({
    locateFile: function(pathName) {
        if (pathName.endsWith('.wasm')) return path.join(__dirname, 'wasm/build/memu8086_core.wasm');
        return pathName;
    }
}).then(Module => {
    console.log("Loaded!");
    const emu = new Module.WasmEmulator();
    console.log("Emu instantiated!", emu.get_errors ? "Has get_errors" : "No get_errors");
}).catch(e => {
    console.error("Crash:", e);
});
