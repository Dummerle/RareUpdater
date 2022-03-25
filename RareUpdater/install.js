function install() {
    console.log("Install")
    window.setTimeout(() =>{
                          root.progress = root.progress + 0.1
                      }, 1000
                      )
    install_python()
    install_deps()
}

function install_python(){
    console.log("Install Python")
}

function install_deps() {
    console.log("Install deps")
}
