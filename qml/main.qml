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

        fractalType: typeSelector.currentIndex

        juliaC: Qt.vector2d(parseFloat(realInput.text), parseFloat(imagInput.text))

        // mouse interaction layer
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton

            // track drag gestures
            property point lastPos: Qt.point(0, 0)

            onPressed: (mouse) => {
                lastPos = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: (mouse) => {
                if (mouse.buttons & Qt.LeftButton) {
                    // calc pixel differences
                    var dx = mouse.x - lastPos.x
                    var dy = mouse.y - lastPos.y

                    engine.panCamera(dx, dy, parent.width, parent.height)

                    lastPos = Qt.point(mouse.x, mouse.y)
                }
            }

            // scroll wheel logic
            onWheel: (wheel) => {
                var zoomFactor = 1.15
                if (wheel.angleDelta.y > 0) {
                    // zoom in
                    engine.zoomLevel /= zoomFactor
                } else {
                    // zoom out
                    engine.zoomLevel *= zoomFactor
                }

                wheel.accepted = true
            }
        }
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
                text: "fraktalized"
                color: "#7d00ff"
                font.pixelSize: 18
                font.bold: true
                font.family: "Monospace"
            }

            // set changer
            ComboBox {
                id: typeSelector
                width: parent.width
                model: ["mandelbrot set", "julia set"]

                // reset zoom on switch
                onCurrentIndexChanged: {
                    engine.zoomLevel = 2.0
                    engine.zoomCenter = Qt.vector2d(currentIndex === 0 ? -0.5 : 0.0, 0.0)
                }
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
                to: 500
                value: 100

                onPressedChanged: if (pressed) forceActiveFocus()
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
                Slider { id: rSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "green:"; color: "#55ff55"; font.pixelSize: 11 }
                Slider { id: gSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "blue:"; color: "#5555ff"; font.pixelSize: 11 }
                Slider { id: bSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            // julia set constant value setter
            Column {
                width: parent.width
                spacing: 8
                // only show when julia set
                visible: typeSelector.currentIndex === 1

                Text {
                    text: "julia constant (c):"
                    color: "#7d00ff"
                    font.pixelSize: 12; font.family: "Monospace"
                }

                Row {
                    width: parent.width
                    spacing: 10

                    // real value
                    Column {
                        width: (parent.width - 10) / 2
                        spacing: 3
                        Text { text: "real (x):"; color: "#aaaaaa"; font.pixelSize: 10 }
                        TextField {
                            id: realInput
                            width: parent.width
                            text: "-0.7"
                            placeholderText: "0.0"
                            selectByMouse: true
                            color: "#ffffff"
                            background: Rectangle { color: "#222222"; radius: 4}
                        }
                    }

                    // imaginary value
                    Column {
                        width: (parent.width - 10) / 2
                        spacing: 3
                        Text { text: "imag (y):"; color: "#aaaaaa"; font.pixelSize: 10 }
                        TextField {
                            id: imagInput
                            width: parent.width
                            text: "0.27015"
                            placeholderText: "0.0"
                            selectByMouse: true
                            color: "#ffffff"
                            background: Rectangle { color: "#222222"; radius: 4 }
                        }
                    }
                }
            }
        }
    }
}