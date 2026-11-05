// Exercise the real GUI sender without starting Electron or opening a serial port.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname, '../../UI/renderer.js'), 'utf8');
const sender = source.match(/function sendConfig\(\)\s*\{[\s\S]*?^\}/m);
assert(sender, 'GUI sender must exist');
const size = source.match(/(?:const|var|let) CONFIG_SIZE\s*=\s*(\d+)/);
assert(size, 'GUI packet size must exist');
let packet;
const fields = {};
for (const id of ['fixedRPM', 'patternSelect', 'rpmSelect', 'rpmSweepMin',
  'rpmSweepMax', 'rpmSweepSpeed', 'compressionEnable', 'compressionMode',
  'compressionRPM', 'compressionOffset', 'compressionDynamic']) {
  fields[id] = { value: '0', checked: false };
}
fields.fixedRPM.value = '3000';
fields.compressionRPM.value = '400';
const context = vm.createContext({ Buffer, CONFIG_SIZE: Number(size[1]),
  document: { getElementById: id => fields[id] },
  port: { write: data => { packet = Buffer.from(data); } },
  console: { log() {} } });
vm.runInContext(sender[0], context);
for (const offset of [0, 255, 256, 359]) {
  for (const enabled of [false, true]) {
    fields.compressionOffset.value = String(offset);
    fields.compressionDynamic.checked = enabled;
    vm.runInContext('sendConfig()', context);
    assert.equal(packet.length, 18);
    assert.equal(packet[0], 0x63);
    assert.equal(packet.readUInt16LE(15), offset);
    assert.equal(packet[17], Number(enabled));
  }
}
console.log('GUI dynamic amplitude and compression offset packet checks passed.');
