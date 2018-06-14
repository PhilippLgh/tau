var fs = require('fs')
var path = require('path')

/*
var pathFile = path.join(__dirname, 'path.txt')

if (fs.existsSync(pathFile)) {
  module.exports = path.join(__dirname, fs.readFileSync(pathFile, 'utf-8'))
} else {
  throw new Error('Tau failed to install correctly, please delete node_modules/tauon and try installing again')
}
*/

module.exports = "dist/tau.exe"
