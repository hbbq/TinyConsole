// Prepare an isolated PlatformIO project; never edits production sources/map.
const fs = require('fs');
const path = require('path');
const root = path.resolve(__dirname, '../..');
const output = path.join(root, '.pio/srb-experiment/build');
fs.mkdirSync(output, {recursive:true});
fs.cpSync(path.join(root,'src'), path.join(output,'src'), {recursive:true});
fs.copyFileSync(path.join(root,'platformio.ini'),path.join(output,'platformio.ini'));
let source = fs.readFileSync(path.join(root,'src/apps/SrbApp.cpp'),'utf8');
const hash = source.slice(source.indexOf('uint8_t hash8('), source.indexOf('\n}',source.indexOf('uint8_t hash8('))+2);
const start = source.indexOf('// Level columns and their offsets');
const end = source.indexOf('void SrbApp::startLevel()');
const fixed = source.slice(source.indexOf('            if (x >= sizeof(level1)'),source.indexOf('\n        }',source.indexOf('            if (x >= sizeof(level1)')));
const replacement = `${hash}\n\n${fs.readFileSync(path.join(__dirname,'generator.h'),'utf8')}\n
uint8_t SrbApp::getLevelColumn(uint8_t x) {
    if (console.state[LEVEL] == 1) {
${fixed}
    }
    return proceduralColumn(x, console.state[LEVEL], console.state[SEED1],
                            console.state[SEED2], console.state[SEED3]);
}

`;
source = source.slice(0,start) + replacement + source.slice(end);
source = source.replace('constexpr uint8_t LEVEL_COUNT = 2;', 'constexpr uint8_t LEVEL_COUNT = 16;');
fs.writeFileSync(path.join(output,'src/apps/SrbApp.cpp'),source);
console.log(output);
