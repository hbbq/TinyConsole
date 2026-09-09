// Copy production into an isolated PlatformIO project for AVR replay/builds.
const fs = require('fs');
const path = require('path');
const root = path.resolve(__dirname, '../..');
const output = path.join(root, '.pio/srb-experiment/build');
fs.mkdirSync(output, {recursive:true});
fs.cpSync(path.join(root,'src'), path.join(output,'src'), {recursive:true});
fs.copyFileSync(path.join(root,'platformio.ini'),path.join(output,'platformio.ini'));
console.log(output);
