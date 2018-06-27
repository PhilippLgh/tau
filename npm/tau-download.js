const fs = require('fs')
const path = require('path')
const http = require('http')
const https = require('https')

function download(url, w, progress = () => {}) {
  return new Promise((resolve, reject) => {
    let protocol = /^https:/.exec(url) ? https : http;

    progress(0);

    protocol
      .get(url, res1 => {

        protocol = /^https:/.exec(res1.headers.location) ? https : http;

        protocol
          .get(res1.headers.location, res2 => {
            const total = parseInt(res2.headers['content-length'], 10);
            let completed = 0;
            res2.pipe(w);
            res2.on('data', data => {
              completed += data.length;
              progress(completed / total);
            });
            res2.on('progress', progress);
            res2.on('error', reject);
            res2.on('end', () => resolve(w.path));
          })
          .on('error', reject);
      })
      .on('error', reject);
  });
}

function downloadTau(config){

  const author = 'philipplgh'
  const repoName = 'tau'
  const binaryName = 'tau'

  // `${binaryName}-${suffix}.zip` :
  // tau-v0.0.1-win32-x64.zip
  const platform = process.platform
  const arch = 'x64'
  const version = '0.0.1'

  const type = `${platform}-${arch}`
  const suffix = `v${version}-${type}`

  let filename = `${binaryName}-${suffix}.zip`
  
  const baseURL = `https://github.com/${author}/${repoName}/releases/download`
  const middleURL = version
  const suffixURL = filename

  let url = `${baseURL}/v${version}/${filename}`

  const outputdir = __dirname
  const destf = path.join(outputdir, filename);
  const dest = fs.createWriteStream(destf);

  let pp = 0
  function onProgress(p) {
    let pn = Math.floor((p * 100))
    if(pn > pp) {
      pp = pn
      console.log(`downloading tau..  ${pn}%`)
    }
  }

  return download(url, dest, onProgress)
}

module.exports = downloadTau