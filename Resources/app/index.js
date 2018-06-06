const fs = require('fs')
const { app, ipc, BrowserWindow } = require('electron')

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

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win

function createWindow() {
    console.log("create window called")
    // Create the browser window.
    //win = new BrowserWindow({ width: 800, height: 600 })
    win = new BrowserWindow("http://www.google.com", "Window Title Test", 200, 100)


    // and load the index.html of the app.
    win.loadFile('index.html')

    // Open the DevTools.
    //win.webContents.openDevTools()

    // Emitted when the window is closed.
    /* TODO
    win.on('closed', () => {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        win = null
    })
    */
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', () => {
    // On macOS it is common for applications and their menu bar
    // to stay active until the user quits explicitly with Cmd + Q
    if (process.platform !== 'darwin') {
        app.quit()
    }
})

app.on('activate', () => {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (win === null) {
        createWindow()
    }
})

// some more testing:

// global C++ string constant
console.log("global c++ constant:", globalConstantTest) //"I was set in C++"

// global C++ method
let z = addFast(4, 9)
console.log("z = ", z)

// timers = stress testing event loop + kepp script alive
let count = 0
let timer = setInterval(() => {
    count++
    console.log('count ' + count)
    if (count > 5)
        clearInterval(timer)
}, 1000)

setTimeout(() => {
    console.log('done')
}, 10*1000)
