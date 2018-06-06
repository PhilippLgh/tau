const fs = require('fs')

function getTime(){
  return new Date().toISOString().
  replace(/T/, ' ').      // replace T with a space
  replace(/\..+/, '')     // delete the dot and everything after
}

function log(str) {
  console.log(str)
  // make use of node core libs
  fs.appendFileSync(__dirname + '/out.log', `${getTime()}: ${str} \n`)
}

log("works")

console.log("node v", process.version)

