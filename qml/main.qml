import QtQuick
import QtQuick.Controls
import fraktalizedModule

Window {
    id: rootWindow
    visible: true
    width: 1280
    height: 720
    title: "fraktalized"

    // openGL viewport
    FractalEngine {
        id: engine
        anchors.fill: parent

        maxIterations: Math.round(maxIterationsSlider.value)

        colorTint: Qt.vector3d(rSlider.value, gSlider.value, bSlider.value)
    }

    // floating panel
    Rectangle {
        id: sidebar
        width: 320
        height: parent.height - 40
        x: 20
        y: 20
        color: "#d00d0e15"
        border.color: "#35ffffff"
        radius: 12
        z: 1

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

            // iteration controls

            Text {
                text: "iterations / detail:"
                color: "#8a90a6"
                font.pixelSize: 13
            }

            Slider {
                id: maxIterationsSlider
                width: parent.width
                from: 10
                to: 250
                value: 100
            }

            // color controls
            Text {
                text: "color setup:"
                color: "#8a90a6"
                font.pixelSize: 13
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "red:"; color: "#ff5555"; font.pixelSize: 11 }
                Slider { id: rSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2 }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "green:"; color: "#55ff55"; font.pixelSize: 11 }
                Slider { id: gSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2 }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "blue:"; color: "#5555ff"; font.pixelSize: 11 }
                Slider { id: bSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2 }
            }
        }
    }
}