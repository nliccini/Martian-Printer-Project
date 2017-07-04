const btSerial = new(require('bluetooth-serial-port')).BluetoothSerialPort();
const _ = require('lodash');
const async = require('async');
const keypress = require('keypress');

const motions = {
    'up': 'w',
    'right': 'd',
    'down': 's',
    'left': 'a',
    'space': ' ',
    'return': 'f',
    'q': ',',
    'w': '.',
    'z': 'z',
    'p': 'p',
    'o': 'o',
    'i': 'i',
    'u': 'u',
    'e': 'z',
    'r': 'x'
};

keypress(process.stdin);

btSerial.on('found', (address, name) => {
    console.log(`I Found a Device: ${address} - ${name}`);

    btSerial.findSerialPortChannel(address, (channel) => {
        if (address !== '20-16-06-30-69-09') {
            return;
        }

        console.log(`For address: ${address}, trying this channel: ${channel}`);

        btSerial.connect(address, channel, () => {

            console.log(`connected to address: ${address} on channel: ${channel}`);

            process.stdin.on('keypress', (ch, key) => {
                //console.log('got "keypress"', key);

                const {
                    name
                } = key;

                if (name === 'c' && key.ctrl) {
                    process.stdin.pause();

                    btSerial.write(new Buffer('s', 'utf-8'), (err, bytesWritten) => {
                        btSerial.close();
                        console.log('Done!');
                    });
                }

                const command = motions[name];

                if (!command) {
                    return;
                }

                btSerial.write(new Buffer(command, 'utf-8'), (err, bytesWritten) => {
                    if (err) {
                        console.log(err);
                    } else {
                        console.log(`Successfully Wrote: ${bytesWritten} Bytes over Bluetooth`);
                    }
                });

            });

            process.stdin.setRawMode(true);
            process.stdin.resume();

            btSerial.on('data', (buffer) => {
                console.log(buffer.toString('utf-8'));
            });

        }, () => {
            console.log('cannot connect');
        });

    }, () => {
        console.log('found nothing');
    });
});

btSerial.inquire();
