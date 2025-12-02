const path = require('path');
const fs = require('fs');
const crypto = require('crypto');
const tls = require('tls');
const ota = require('./ota');
const filenameOrURL = process.argv[2];
//const modelId = process.argv[3];
//const baseURL = 'https://github.com/pvvx/ATC_MiThermometer/raw/master/zigbee_ota';
const baseURL = 'https://github.com/pvvx/TlsrSmartSwitch/raw/refs/heads/master/bin';
const caCerts = './cacerts.pem';

const manufacturerNameLookup = {
    0x7B: 'UHome',
    0x1002: 'Tuya',
    0x100B: 'Hue',
    0x1015: 'Develco',
    0x1021: 'Legrand',
    0x1037: 'Jennic',
    0x105E: 'SchneiderElectric',
    0x110C: 'Osram',
    0x1135: 'DresdenElektronik',
    0x1141: 'Telink',
    0x1144: 'Lutron',
    0x115C: 'Danalock',
    0x115F: 'Lumi',
    0x1160: 'Sengled',
    0x1166: 'Innr',
    0x117A: 'Insta',
    0x117C: 'IKEA',
    0x1189: 'Ledvance',
    0x1209: 'Bosch',
    0x1224: 'Namron',
    0x1228: 'Terncy',
    0x1233: 'ThirdReality',
    0x1246: 'Danfoss',
    0x124F: 'Gledopto',
    0x126A: 'EcoDim',
    0x1286: 'Sonoff',
    0x128B: 'NodOn',
    0x1337: 'Datek',
    0x130D: 'thirdreality',
    0x121C: 'Aurora',
    0x1407: '3R',
    0x2794: 'ClimaxTechnology',
    0x6565: 'Slacky-DIY',
    0x6666: 'Sprut.device',
    0xEA60: 'Telink-pvvx',
};

const main = async () => {
    if (!filenameOrURL) {
        throw new Error('Please provide a filename or URL');
    }

    const isURL = filenameOrURL.toLowerCase().startsWith("http");
    const files = [];

    if (isURL) {
        const downloadFile = async (url, path) => {
            const lib = url.toLowerCase().startsWith("https") ? require('https') : require('http');
            const file = fs.createWriteStream(path);

            return new Promise((resolve, reject) => {
                const ca = [...tls.rootCertificates];
                if(fs.existsSync(caCerts)) {
                    ca.push(fs.readFileSync(caCerts));
                }
                const request = lib.get(url, { ca },  function(response) {
                    if (response.statusCode >= 200 && response.statusCode < 300) {
                        response.pipe(file);
                        file.on('finish', function() {
                          file.close(function() {
                              resolve();
                          });
                        });
                    } else if (response.headers.location) {
                        resolve(downloadFile(response.headers.location, path));
                    } else {
                        reject(new Error(response.statusCode + ' ' + response.statusMessage));
                    }
                });
            });
        }

        const file = path.resolve("temp");
        await downloadFile(filenameOrURL, file);
        files.push(file);
    } else {
        const file = path.resolve(filenameOrURL);
        if (fs.lstatSync(file).isFile()) {
            if (!fs.existsSync(file)) {
                throw new Error(`${file} does not exist`);
            }
            files.push(file);
        } else {
            const otaExtension = ['.ota', '.zigbee'];
            const otasInDirectory = fs.readdirSync(file)
                .filter((f) => otaExtension.includes(path.extname(f).toLowerCase()))
                .map((f) => path.join(file, f));
            files.push(...otasInDirectory);
        }
    }

    for (const file of files) {
        const buffer = fs.readFileSync(file);
        const parsed = ota.parseImage(buffer);

        if (!manufacturerNameLookup[parsed.header.manufacturerCode]) {
            throw new Error(`${parsed.header.manufacturerCode} not in manufacturerNameLookup (please add it)`);
        }

        const manufacturerName = manufacturerNameLookup[parsed.header.manufacturerCode];
        const indexJSON = JSON.parse(fs.readFileSync('index.json'));
        //const destination = path.join('images', manufacturerName, path.basename(file));
        const destination = path.basename(file);

        const hash = crypto.createHash('sha512');
        hash.update(buffer);
        const hdString = parsed.header.otaHeaderString;
        const hdmanufacturer = [hdString.split(':')[0]];
        const modelId = hdString.split(':')[1];

        const entry = {
            fileVersion: parsed.header.fileVersion,
            fileSize: parsed.header.totalImageSize,
            manufacturerCode: parsed.header.manufacturerCode,
            imageType: parsed.header.imageType,
            sha512: hash.digest('hex'),
            otaHeaderString: hdString,
            //manufacturerName: manufacturerName,
        };

        if (modelId) {
            entry.modelId = modelId;
            entry.manufacturerName = hdmanufacturer;
        }

        if (isURL) {
            entry.url = filenameOrURL;
        } else {
            const destinationPosix = destination.replace(/\\/g, '/');
            entry.url = `${baseURL}/${escape(destinationPosix)}`;
            //entry.path = destinationPosix;
        }

        const index = indexJSON.findIndex((i) => {
            return i.manufacturerCode === entry.manufacturerCode && i.imageType === entry.imageType && (!i.modelId || i.modelId === entry.modelId)
        });

        if (index !== -1) {
            console.log(`Updated existing entry (${JSON.stringify(entry)})`);
            indexJSON[index] = {...indexJSON[index], ...entry};

            if (entry.path && entry.path !== destination) {
                try {
                    fs.unlinkSync(path.resolve(entry.path));
                } catch (err) {
                    if (err && err.code != 'ENOENT') {
                        console.error("Error in call to fs.unlink", err);
                        throw err;
                    }
                }
            }
        } else {
            console.log(`Added new entry (${JSON.stringify(entry)})`);
            indexJSON.push(entry);
        }

        if (!isURL && file !== path.resolve(destination)) {
            if (!fs.existsSync(path.dirname(destination))) {
                fs.mkdirSync(path.dirname(destination));
            }

            fs.copyFileSync(file, destination);
        }

        fs.writeFileSync('index.json', JSON.stringify(indexJSON, null, '    '));

        if (isURL) {
            fs.unlinkSync(file);
        }
    }
}

main();
