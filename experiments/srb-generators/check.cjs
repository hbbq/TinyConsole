// Terrain-only reachability experiment. No dependencies. node check.cjs
const fs = require('fs');
const path = require('path');
const assert = require('assert/strict');
function hash8(x, seed) {
    x = (x + seed * 31) & 65535;
    x ^= x >> 7;
    x = (x * 13) & 65535;
    return (x ^ (x >> 5)) & 255;
}
function column(x, level, s1, s2, s3) {
    if (x === 255) return 255;
    if (x >= 239) return 0xbc;
    if (x < 16) return x >= 6 && x <= 7 ? 0x90 : 0x80;
    if (x >= 232) return 0x80;
    const tier = Math.min(3, Math.floor((level - 1) / 5));
    const intensity = 5 + tier + (x >> 6), density = Math.min(8, intensity);
    const cell = x >> 4, pos = x & 15;
    const a = hash8(cell, s1), b = hash8(cell, s2);
    let col = 0x80;
    if (level % 5 === 0) {
        const length = 7 - ((hash8(x >> 3, s2) & 7) < density);
        col = (x & 7) < length ? 1 << (5 + (hash8(x >> 3, s1) & 1)) : 0;
    } else if (!(level & 1)) {
        if ((a & 7) < density && pos >= 4 && pos < 5 + ((a >> 7) || intensity > 8)) col = 0;
        if ((b & 7) < density && pos >= 9 && pos <= 10)
            col = (b & 128) || intensity > 9 ? 0xe0 : 0xc0;
        if (pos <= 1) col |= 0x10;
    } else {
        if ((a & 7) < density) {
            let pattern = b & 3;
            if (intensity > 8 && pattern === 2) pattern = 3;
            if ((pattern === 0 || pattern === 3) && pos >= 4 && pos <= 5) col = 0;
            if ((pattern === 1 || pattern === 3) && pos >= 9 && pos <= 10) col = 0xc0;
            if (pattern === 2 && pos >= 4 && pos <= 6) col |= 0x10;
        }
        if (pos <= 1) col |= 0x10;
    }
    if (col && (hash8(x, s3) & 63) < 2 + tier + (x >> 6)) col |= 1;
    return col;
}
// State is world x, unsigned 4-bit-fraction y, signed velocity, tick modulo 4.
// Actual order: jump, gravity (+2, capped at +8), horizontal movement.
// Reject falls instead of accepting the game's life-loss recovery platform.
function step(cols, state, jump, dir) {
    let [x, y, v, t] = state;
    const solid = (xx, yy) => yy >= 0 && yy < 8 && !!(cols[xx] & (1 << yy));
    if (jump && solid(x, (y >> 4) + 1)) v = -16;
    v = Math.min(v + 2, 8);
    if (v > 0 && solid(x, (y >> 4) + 1)) { v = 0; y = (y >> 4) << 4; }
    if (v < 0 && solid(x, (y + v) >> 4)) { v = 0; y = (y >> 4) << 4; }
    y = Math.max(0, y + v);
    if (y >= 128) return null;
    if (t === 0 && dir && x + dir >= 4 && !solid(x + dir, y >> 4)) x += dir;
    return [x, y, v, (t + 1) & 3];
}
function key(s) { return (((s[0] * 128 + s[1]) * 25 + s[2] + 16) * 4 + s[3]); }
// Search monotonically rightward with arbitrary waits/jump timing. A found
// route suffices; a failure is conservative (leftward solutions not searched).
function route(cols, start = [4, 0, 0, 0], goal = 244, witness = false, landingY = null) {
    const queue = [start], seen = new Set([key(start)]), parents = witness ? [-1] : null;
    const actions = witness ? [0] : null;
    for (let i = 0; i < queue.length; ++i) {
        const s = queue[i];
        if (s[0] >= goal && (landingY === null || (s[1] === landingY && s[2] === 0))) {
            if (!witness) return true;
            const result = [];
            for (let j = i; parents[j] !== -1; j = parents[j]) result.push(actions[j]);
            return result.reverse();
        }
        for (const action of [2, 3, 0, 1]) {
            if (s[3] && (action & 2)) continue;
            const next = step(cols, s, !!(action & 1), action & 2 ? 1 : 0);
            if (!next || next[0] > goal) continue;
            const k = key(next);
            if (seen.has(k)) continue;
            seen.add(k); queue.push(next);
            if (witness) { parents.push(i); actions.push(action); }
        }
    }
    return false;
}
function bounds() {
    const results = {};
    for (let width = 1; width <= 6; ++width) {
        const cols = Array(64).fill(0x80);
        cols.fill(0, 16, 16 + width);
        results['gap' + width] = route(cols, [4,96,0,0], 32);
    }
    for (let height = 1; height <= 5; ++height) {
        const cols = Array(64).fill(0x80);
        cols.fill((256 - (1 << (7 - height))) & 255,16);
        results['step' + height] = route(cols, [4,96,0,0], 24);
    }
    for (let row = 2; row <= 5; ++row) {
        const cols = Array(64).fill(0x80);
        cols.fill(0, 16);
        for (let x = 16; x < 64; ++x) cols[x] = 1 << row;
        results['platformRow' + row] = route(cols, [4,96,0,0], 24);
    }
    console.log('Isolated bounds:', JSON.stringify(results));
    return results;
}
function transitions() {
    const variants = [];
    // Superset of every independent/pattern cell: optional 1/2-wide hole,
    // optional 1/2-high step, optional raised platform at either approved zone.
    for (const gap of [0,1,2]) for (const height of [0,1,2]) for (const platform of [0,1,2,3]) {
        const cell = Array(16).fill(128);
        cell.fill(0,4,4+gap);
        if (height) cell.fill(height===1 ? 192:224,9,11);
        if (platform & 1) for (let p=0; p<2; ++p) cell[p] |= 16;
        if (platform & 2) for (let p=4; p<7; ++p) cell[p] |= 16;
        // The pattern platform never coexists with its hole or step.
        if ((platform & 2) && (gap || height)) continue;
        variants.push(cell);
    }
    let checks = 0;
    for (const a of variants) for (const b of variants) for (let t=0;t<4;++t) {
        const cols = [...Array(16).fill(128), ...a, ...b, ...Array(16).fill(128)];
        assert.ok(route(cols,[18,96,0,t],34,false,96), `ground transition ${checks}`);
        checks++;
    }
    for (const h1 of [5,6]) for (const h2 of [5,6]) for (const gap1 of [1,2]) for (const gap2 of [1,2]) for(let t=0;t<4;++t) {
        const cols=Array(64).fill(0);
        cols.fill(1<<h1,16,24-gap1); cols.fill(1<<h2,24,32-gap2);
        assert.ok(route(cols,[18,(h1-1)*16,0,t],26,false,(h2-1)*16),`bottomless transition ${checks}`);
        checks++;
    }
    for (const cell of variants) for (let t=0;t<4;++t) {
        const start = Array(16).fill(128); start[6]=start[7]=0x90;
        const entry = [...start,...cell,...Array(16).fill(128)];
        assert.ok(route(entry,[4,0,0,t],18,false,96),'ground entry');
        const ending = Array(256).fill(128);
        ending.splice(224,8,...cell.slice(0,8));
        ending.fill(0xbc,239,255);
        assert.ok(route(ending,[226,96,0,t],244),'ground ending');
        checks+=2;
    }
    for (const h of [5,6]) for (const gap of [1,2]) for(let t=0;t<4;++t) {
        const entry=Array(48).fill(128);
        entry[6]=entry[7]=0x90;
        entry.fill(0,16,32); entry.fill(1<<h,16,24-gap);
        assert.ok(route(entry,[4,0,0,t],18,false,(h-1)*16),'bottomless entry');
        const ending=Array(256).fill(128);
        ending.fill(0,224,232); ending.fill(1<<h,224,232-gap);
        ending.fill(0xbc,239,255);
        assert.ok(route(ending,[226,(h-1)*16,0,t],244),'bottomless ending');
        checks+=2;
    }
    console.log(`PASS ${checks} adjacent-cell/phase checks with settled landing checkpoints`);
    return checks;
}
if (require.main === module) {
    const output = path.resolve(__dirname, '../../.pio/srb-experiment');
    fs.mkdirSync(output, {recursive:true});
    const results = bounds(), transitionChecks = transitions(), cases = [];
    const levels = [2,3,5,6,7,10,11,12,15,16,17,20,254,255];
    const count = Number(process.argv[2] || 32);
    for (const level of levels) {
        for (let seed = 0; seed < count; ++seed) {
            const seeds = [seed & 255, (seed * 73 + 19) & 255, (seed * 151 + 97) & 255];
            const raw = Array.from({length:256}, (_,x) => column(x, level, ...seeds));
            for (let x = 255; x >= 0; --x) assert.equal(column(x, level, ...seeds), raw[x]);
            const start = Array(16).fill(0x80); start[6]=start[7]=0x90;
            assert.deepEqual(raw.slice(0,16), start);
            assert.equal(raw[238], 0x80); assert.equal(raw[255], 255);
            assert.deepEqual(raw.slice(239,255), Array(16).fill(0xbc));
            let plain = 0;
            for (const col of raw) {
                plain = (col & 0xfc) === 0x80 ? plain + 1 : 0;
                assert.ok(plain < 16, `Featureless run level=${level} seeds=${seeds}`);
            }
            const cols = raw.map(c => c & 0xfc);
            const found = route(cols, [4,0,0,0], 244, seed === 0);
            assert.ok(found, `Unreachable level=${level} seeds=${seeds}`);
            if (seed === 0) cases.push({level, seeds, raw, actions:found});
        }
        console.log(`PASS level ${level}: ${count} seed triples`);
    }
    fs.writeFileSync(path.join(output,'witnesses.json'), JSON.stringify(cases));
    fs.writeFileSync(path.join(output,'results.json'), JSON.stringify({bounds:results,transitionChecks,levels,count,total:levels.length*count},null,2));
    console.log(`PASS ${levels.length * count} complete routes; witnesses saved for AVR replay.`);
}
module.exports = {column, step, route, bounds};
