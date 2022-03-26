import QtQuick 2.0
import Qt.labs.platform 1.1
import QuickDownload 1.0

Item {
    Download {
        id: download_python

        url: "https://www.python.org/ftp/python/3.10.3/python-3.10.3-embed-amd64.zip"
        property string this_url: url
        property var url_parts: this_url.split('/')
        destination: StandardPaths.writableLocation(StandardPaths.TempLocation) + "/" + url_parts[url_parts.length - 1]

        overwrite: true
        running: true

        followRedirects: true
        onRedirected: console.log('Redirected',url,'->',redirectUrl)

        onStarted: console.log('Started download',url)
        onError: console.error(errorString)
        onProgressChanged: console.log(url,'progress:',progress)
        onFinished: console.info(url,'done')
    }

    Download {
        id: download_get_pip

        url: "https://bootstrap.pypa.io/get-pip.py"
        property string this_url: url
        property var url_parts: this_url.split('/')
        destination: StandardPaths.writableLocation(StandardPaths.TempLocation) + "/" + url_parts[url_parts.length - 1]

        overwrite: true
        running: true

        followRedirects: true
        onRedirected: console.log('Redirected',url,'->',redirectUrl)

        onStarted: console.log('Started download',url)
        onError: console.error(errorString)
        onProgressChanged: console.log(url,'progress:',progress)
        onFinished: console.info(url,'done')
    }

}
