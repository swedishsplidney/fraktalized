import QtQuick
import QtQuick.Controls
import fraktalizedModule

Window {
    visible: true
    width: 1280
    height: 720
    title: "fraktalized"
    color: "#0d0e15"

    // openGL viewport
    FractalEngine {
        anchors.fill: parent
    }

    // floating panel
    Rectangle {
        id: sidebar
        width: 320
        height: parent.height - 40
        x: 20
        y: 20
        color: "#25000000"
        border.color: "#35ffffff"
        radius: 12

        Column {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 20

            Text {
                text: "fraktalized core"
                color: "#00ffcc"
                font.pixelSize: 18
                font.bold: true
                font.family: "Monospace"
            }

            Text {
                text: "adjust rendering parameters:"
                color: "#8a90a6"
                font.pixelSize: 13
            }

            // test component
            Slider {
                id: maxIterations
                width: parent.width
                from: 10
                to: 1000
                value: 100
            }
        }
    }
}