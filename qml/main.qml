import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
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
        fractalType: typeSelector.currentIndex
        juliaC: Qt.vector2d(parseFloat(realInput.text), parseFloat(imagInput.text))

        viewportScale: resSlider.value
        aaSamples: aaSelector.currentIndex + 1

        // mouse interaction layer
        MouseArea {
            id: internalMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            hoverEnabled: true

            property point lastPos: Qt.point(0, 0)
            property real cursorX: 0.0
            property real cursorY: 0.0

            onPressed: (mouse) => {
                lastPos = Qt.point(mouse.x, mouse.y)
                sidebar.forceActiveFocus()
            }

            onPositionChanged: (mouse) => {
                if (width <= 0 || height <= 0) return;

                let pctX = mouse.x / width
                let pctY = mouse.y / height

                let screenAspect = width / height
                let scaleX = 1.0
                let scaleY = 1.0

                if (width >= height) {
                    scaleX = 1.0
                    scaleY = 1.0 / screenAspect
                } else {
                    scaleX = screenAspect
                    scaleY = 1.0
                }

                let currentX = engine.zoomCenter.x + (pctX * 2.0 - 1.0) * (scaleX * engine.zoomLevel)
                let currentY = engine.zoomCenter.y + (pctY * 2.0 - 1.0) * (scaleY * engine.zoomLevel)

                cursorX = currentX
                cursorY = currentY

                if (mouse.buttons & Qt.LeftButton) {
                    var dx = mouse.x - lastPos.x
                    var dy = mouse.y - lastPos.y
                    engine.panCamera(dx, dy, parent.width, parent.height)
                    lastPos = Qt.point(mouse.x, mouse.y)
                }
            }

            onWheel: (wheel) => {
                var zoomFactor = 1.15
                if (wheel.angleDelta.y > 0) {
                    engine.zoomLevel /= zoomFactor
                } else {
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
        focus: true

        ScrollView {
            id: sidebarScroll
            anchors.fill: parent
            anchors.bottomMargin: 58
            clip: true

            leftPadding: 16
            rightPadding: 16
            topPadding: 20
            bottomPadding: 20

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: sidebarScroll.availableWidth
                spacing: 18

                Text {
                    text: "fraktalized"
                    color: "#7d00ff"
                    font.pixelSize: 20
                    font.bold: true
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }

                ComboBox {
                    id: typeSelector
                    model: ["mandelbrot set", "julia set", "burning ship set"]
                    Layout.fillWidth: true

                    function applyDefaultView() {
                        switch (currentIndex) {
                            case 0: // mandelbrot
                                engine.zoomLevel = 2.0
                                engine.zoomCenter = Qt.vector2d(-0.5, 0.0)
                                break;
                            case 1: // julia
                                engine.zoomLevel = 2.0
                                engine.zoomCenter = Qt.vector2d(0.0, 0.0)
                                break;
                            case 2: // burning ship
                                engine.zoomLevel = 0.1
                                engine.zoomCenter = Qt.vector2d(-1.755, -0.03)
                                break;
                        }
                    }

                    onCurrentIndexChanged: {
                        applyDefaultView()
                    }
                }

                // iteration controls container
                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    RowLayout {
                        Layout.fillWidth: true
                        height: 24
                        spacing: 6

                        Text {
                            text: "iterations / detail"
                            color: "#8a90a6"
                            font.pixelSize: 13
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            id: menuButton
                            width: 24
                            height: 24
                            flat: true
                            checkable: true
                            background: Item {}

                            contentItem: Text {
                                text: menuButton.checked ? "∨" : ">"
                                font.pixelSize: 14
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: menuButton.hovered ? "#7d00ff" : "#8a90a6"
                            }
                        }
                    }

                    // settings container
                    ColumnLayout {
                        id: iterationsSettings
                        Layout.fillWidth: true
                        spacing: 8
                        clip: true

                        visible: menuButton.checked || heightAnimation.running

                        Layout.preferredHeight: menuButton.checked ? targetHeight : 0
                        property real targetHeight: 55

                        Behavior on Layout.preferredHeight {
                            NumberAnimation {
                                id: heightAnimation
                                duration: 150
                                easing.type: Easing.InOutQuad
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            // min value
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3
                                Text { text: "min value:"; color: "#aaaaaa"; font.pixelSize: 10 }
                                TextField {
                                    id: minLimitInput
                                    Layout.fillWidth: true
                                    height: 28
                                    text: "10"
                                    placeholderText: "0"
                                    placeholderTextColor: "#444444"
                                    selectByMouse: true
                                    color: "#ffffff"
                                    font.pixelSize: 12
                                    verticalAlignment: TextInput.AlignVCenter
                                    leftPadding: 8
                                    background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }
                                    onTextChanged: {
                                        var val = parseInt(text)
                                        if (!isNaN(val) && val >= 0) maxIterationsSlider.from = val
                                    }
                                }
                            }

                            // max value
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3
                                Text { text: "max value:"; color: "#aaaaaa"; font.pixelSize: 10 }
                                TextField {
                                    id: maxLimitInput
                                    Layout.fillWidth: true
                                    height: 28
                                    text: "500"
                                    placeholderText: "0"
                                    placeholderTextColor: "#444444"
                                    selectByMouse: true
                                    color: "#ffffff"
                                    font.pixelSize: 12
                                    verticalAlignment: TextInput.AlignVCenter
                                    leftPadding: 8
                                    background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }
                                    onTextChanged: {
                                        var val = parseInt(text)
                                        if (!isNaN(val)) maxIterationsSlider.to = val
                                    }
                                }
                            }
                        }
                    }

                    // warn box
                    Rectangle {
                        id: warnBox
                        Layout.fillWidth: true
                        clip: true
                        color: "#2a0808"
                        border.color: "#ff3333"
                        border.width: 1
                        radius: 6

                        property bool isDismissed: false
                        property bool shouldShow: (Math.round(maxIterationsSlider.value) > 2500) && !isDismissed

                        visible: shouldShow || warnAnimation.running
                        Layout. preferredHeight: shouldShow ? 60 : 0

                        Behavior on height { NumberAnimation { id: warnAnimation; duration: 150 } }

                        Column {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.right: okButton.left
                            anchors.margins: 8
                            spacing: 4

                            Text { text: "warning"; color: "#ff3333"; font.pixelSize: 10; font.bold: true }
                            Text {
                                text: "values over 2500 iterations can cause problems on low-end hardware"
                                color: "#ffaaaa"; font.pixelSize: 9
                                wrapMode: Text.Wrap
                                width: parent.width - 10
                            }
                        }

                        Button {
                            id: okButton
                            width: 36; height: 20
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            anchors.margins: 6
                            background: Rectangle {
                                color: okButton.hovered ? "#ff3333" : "#4a1212"
                                radius: 4; border.color: "#ff3333"; border.width: 1
                            }
                            contentItem: Text {
                                text: "ok"; color: okButton.hovered ? "#2a0808" : "#ff3333"
                                font.pixelSize: 10; font.bold: true
                                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: warnBox.isDismissed = true
                        }
                    }

                    // slider section
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Slider {
                            id: maxIterationsSlider
                            Layout.fillWidth: true
                            from: 10; to: 500; value: 100
                            onPressedChanged: if (pressed) forceActiveFocus()
                        }

                        TextField {
                            id: currentValueInput
                            Layout.fillWidth: true
                            height: 24
                            leftPadding: 6
                            color: activeFocus ? "#ffffff" : "#8a90a6"
                            font.pixelSize: 10
                            font.family: "Monospace"
                            selectByMouse: true
                            verticalAlignment: TextInput.AlignVCenter

                            property bool isOutOfBounds: false

                            Binding on text {
                                when: !currentValueInput.activeFocus && !currentValueInput.isOutOfBounds
                                value: "current: " + Math.round(maxIterationsSlider.value)
                            }

                            validator: IntValidator { bottom: 0; top: 2147483647 }

                            background: Rectangle {
                                color: parent.activeFocus ? "#221133" : "transparent"
                                border.color: parent.activeFocus ? "#7d00ff" : "transparent"
                                border.width: 1; radius: 4
                            }

                            Timer {
                                id: resetTimer; interval: 1200
                                onTriggered: currentValueInput.isOutOfBounds = false
                            }

                            onEditingFinished: {
                                var val = parseInt(text)
                                if (!isNaN(val)) {
                                    if ((val > maxIterationsSlider.to) || (val < maxIterationsSlider.from)) {
                                        currentValueInput.isOutOfBounds = true
                                        currentValueInput.text = "outside of currently set range"
                                        resetTimer.restart()
                                    }
                                    val = Math.max(maxIterationsSlider.from, Math.min(val, maxIterationsSlider.to))
                                    maxIterationsSlider.value = val
                                }
                                deselect(); focus = false
                                rootWindow.contentItem.focus = true
                                rootWindow.contentItem.forceActiveFocus()
                            }

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    resetTimer.stop()
                                    currentValueInput.isOutOfBounds = false
                                    text = Math.round(maxIterationsSlider.value).toString()
                                    selectAll()
                                }
                            }
                        }
                    }
                }

                // color controls
                Text { text: "color gradient maps"; color: "#8a90a6"; font.pixelSize: 13; Layout.fillWidth: true }

                ColumnLayout {
                    // gradient stops
                    id: gradientEditorContainer
                    spacing: 12
                    Layout.fillWidth: true

                    function updateStopPosition(index, newPos) {
                        stops[index].pos = Math.max(0.0, Math.min(1.0, newPos));
                        updateEngineGradient();
                    }

                    property var stops: [
                        { pos: 0.0, color: "#000000" },
                        { pos: 0.35, color: "#ff3300" },
                        { pos: 0.75, color: "#ffcc00" },
                        { pos: 1.0, color: "#ffffff" }
                    ]

                    // compile and stream values to c++
                    function updateEngineGradient() {
                        let sortedStops = [...stops].sort((a, b) => a.pos - b.pos);

                        let positions = [];
                        let colors = [];
                        for (let i = 0; i < sortedStops.length; i++) {
                            positions.push(sortedStops[i].pos);
                            colors.push(sortedStops[i].color);
                        }

                        engine.gradientPositions = positions;
                        engine.gradientColors = colors;
                    }

                    Component.onCompleted: updateEngineGradient()

                    // gradient preview
                    Rectangle {
                        id: gradientTrack
                        height: 24
                        Layout.fillWidth: true
                        radius: 4
                        border.color: "#444444"

                        // visualize stops
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: gradientEditorContainer.stops[0].pos; color: gradientEditorContainer.stops[0].color }
                            GradientStop { position: gradientEditorContainer.stops[1].pos; color: gradientEditorContainer.stops[1].color }
                            GradientStop { position: gradientEditorContainer.stops[2].pos; color: gradientEditorContainer.stops[2].color }
                            GradientStop { position: gradientEditorContainer.stops[3].pos; color: gradientEditorContainer.stops[3].color }
                        }
                    }

                    // interactive handles
                    Item {
                        id: handleTrack
                        Layout.fillWidth: true
                        height: 24

                        Repeater {
                            model: gradientEditorContainer.stops

                            // each color stop anchor
                            Rectangle {
                                id: handleAnchor
                                width: 10
                                height: 24
                                radius: 2
                                color: modelData.color
                                border.color: dragArea.pressed ? "#7d00ff" : "#ffffff"
                                border.width: 1.5
                                anchors.verticalCenter: parent.verticalCenter

                                // map the 0 - 1 position to width coords
                                x: modelData.pos * (handleTrack.width - width)

                                MouseArea {
                                    id: dragArea
                                    anchors.fill: parent
                                    drag.target: parent
                                    drag.axis: Drag.XAxis
                                    drag.minimumX: 0
                                    drag.maximumX: handleTrack.width - handleAnchor.width

                                    // update data map when dragging
                                    onPositionChanged: {
                                        if (pressed) {
                                            let maxTrackWidth = handleTrack.width - handleAnchor.width
                                            if (maxTrackWidth <= 0) return;

                                            let newPos = handleAnchor.x / maxTrackWidth
                                            gradientEditorContainer.updateStopPosition(index, newPos)

                                            gradientTrack.gradient.stops[index].position = Math.max(0.0, Math.min(1.0, newPos))
                                        }
                                    }

                                    // double click an anchor to change its color
                                    onDoubleClicked: {
                                        colorPicker.targetIndex = index
                                        colorPicker.selectedColor = modelData.color
                                        colorPicker.open()
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        text: "double click anchors to change colors"
                        color: "#555a6b"
                        font.pixelSize: 10
                        font.italic: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // color picker popup
                ColorDialog {
                    id: colorPicker
                    title: "choose stop color"
                    property int targetIndex: 0

                    onAccepted: {
                        gradientEditorContainer.stops[targetIndex].color = colorPicker.selectedColor.toString()

                        gradientEditorContainer.stopsChanged()

                        gradientEditorContainer.updateEngineGradient()
                    }
                }

                // julia constant
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    visible: typeSelector.currentIndex === 1

                    Text { text: "julia constant (c):"; color: "#8a90a6"; font.pixelSize: 12; font.family: "Monospace" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Text { text: "real (x):"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: realInput
                                Layout.fillWidth: true
                                text: "-0.7"
                                placeholderText: "0.0"
                                placeholderTextColor: "#444444"
                                selectByMouse: true
                                color: "#ffffff"
                                background: Rectangle { color: "#222222"; radius: 4 }
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Text { text: "imag (y):"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: imagInput
                                Layout.fillWidth: true
                                text: "0.27015"
                                placeholderText: "0.0"
                                placeholderTextColor: "#444444"
                                selectByMouse: true
                                color: "#ffffff"
                                background: Rectangle { color: "#222222"; radius: 4 }
                            }
                        }
                    }
                }

                // export panel
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true

                    Text { text: "export high-res png:"; color: "#8a90a6"; font.pixelSize: 12; font.family: "Monospace" }

                    // width
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Text { text: "width (px):"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: exportWidthInput
                                Layout.fillWidth: true
                                height: 28
                                text: "3840"
                                placeholderText: "3480"
                                placeholderTextColor: "#444444"
                                selectByMouse: true; color: "#ffffff"
                                font.pixelSize: 12
                                font.family: "Monospace"
                                leftPadding: 8
                                background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }
                                validator: IntValidator { bottom: 1; top: 30720 }
                            }
                        }

                        // height
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Text { text: "height (px):"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: exportHeightInput
                                Layout.fillWidth: true
                                height: 28
                                text: "2160"
                                placeholderText: "2160"
                                placeholderTextColor: "#444444"
                                selectByMouse: true
                                color: "#ffffff"
                                font.pixelSize: 12
                                font.family: "Monospace"
                                leftPadding: 8
                                background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }
                                validator: IntValidator { bottom: 1; top: 30720 }
                            }
                        }
                    }

                    // render button
                    Button {
                        id: exportButton; text: "render image to file"
                        Layout.fillWidth: true
                        height: 32
                        background: Rectangle {
                            color: exportButton.hovered ? "#4d0099" : "#2a0055"
                            border.color: exportButton.hovered ? "#7d00ff" : "#5500aa"
                            border.width: 1
                            radius: 6
                        }

                        contentItem: Text {
                            text: exportButton.text
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.family: "Monospace"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            var w = parseInt(exportWidthInput.text)
                            var h = parseInt(exportHeightInput.text)
                            if (!isNaN(w) && !isNaN(h) && w > 0 && h > 0) {
                                var timestamp = Qt.formatDateTime(new Date(), "yyyyMMdd_hhmmss")
                                engine.renderToFile("render_" + timestamp + ".png", w, h)
                            }
                        }
                    }
                }
            }
        }

        // reset view button (anchored to sidebar bottom layout)
        Button {
            id: resetButton; text: "reset view"
            width: 110; height: 30
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 25
            background: Rectangle {
                color: resetButton.hovered ? "#331155" : "#1a0d26"
                border.color: resetButton.hovered ? "#7d00ff" : "#443355"
                border.width: 1; radius: 6
            }
            contentItem: Text {
                text: resetButton.text
                color: resetButton.hovered ? "#ffffff" : "#8a90a6"
                font.pixelSize: 12
                font.family: "Monospace"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                typeSelector.applyDefaultView()

                maxIterationsSlider.value = 100
                sidebar.forceActiveFocus()
            }
        }
    }

    // settings panel
    Rectangle {
        id: settingsPanel
        width: 260
        height: 155
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        color: "#d00d0e15"
        radius: 12
        z: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            Text {
                text: "performance settings:"
                color: "#7d00ff"
                font.pixelSize: 14
                font.bold: true
                font.family: "Monospace"
                Layout.fillWidth: true
            }

            // viewport res controls
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "viewport resolution: " + Math.round(resSlider.value * 100) + "%"
                    color: "#8a90a6"
                    font.pixelSize: 11
                }

                Slider {
                    id: resSlider
                    Layout.fillWidth: true
                    from: 0.25
                    to: 1.5
                    value: 1.0
                    onPressedChanged: if (pressed) sidebar.forceActiveFocus()
                }
            }

            // anti aliasing dropdown
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: "anti-aliasing"
                    color: "#8a90a6"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignVCenter
                }

                ComboBox {
                    id: aaSelector
                    model: ["off (1x)", "low ssaa (2x)", "high ssaa (3x)"]
                    currentIndex: 0
                    Layout.fillWidth: true
                }
            }
        }
    }

    // coord panel
    Rectangle {
        id: coordPanel
        width: 220
        height: 42
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        color: "#d00d0e15"
        radius: 12
        z: 1

        ColumnLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.margins: 10
            spacing: 4

            // center coord
            RowLayout {
                Layout.fillWidth: true

                Text { text: "ctr "; color: "#8a90a6"; font.pixelSize: 10; font.bold: true; font.family: "Monospace" }

                Text {
                    text: "x: " + engine.zoomCenter.x.toFixed(6) + " | y: " + engine.zoomCenter.y.toFixed(6)
                    color: "#aaaaaa"
                    font.pixelSize: 10
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }

            // cursor coord
            RowLayout {
                Layout.fillWidth: true

                Text { text: "cur "; color: "#8a90a6"; font.pixelSize: 10; font.bold: true; font.family: "Monospace"}

                Text {
                    text: "x: " + internalMouseArea.cursorX.toFixed(6) + " | y: " + internalMouseArea.cursorY.toFixed(6)
                    color: "#aaaaaa"
                    font.pixelSize: 10
                    font.family: "Monospace"
                    Layout.fillWidth: true
                }
            }
        }
    }
}