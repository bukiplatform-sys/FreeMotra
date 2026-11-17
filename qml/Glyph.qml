import QtQuick
Canvas {
    id: glyph
    property string name: "screen"
    property color ink: "#72abff"
    implicitWidth: 24
    implicitHeight: 24
    onNameChanged: requestPaint()
    onInkChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        let c = getContext("2d"); c.reset(); c.scale(width / 24, height / 24)
        c.strokeStyle = ink; c.fillStyle = ink; c.lineWidth = 1.6; c.lineCap = "round"; c.lineJoin = "round"
        function line(x,y,a,b) { c.beginPath(); c.moveTo(x,y); c.lineTo(a,b); c.stroke() }
        if (name === "screen" || name === "connect") {
            c.strokeRect(3,4,18,12); line(12,16,12,21); line(7,21,17,21)
            if (name === "connect") { line(8,10,16,10); line(13,7,16,10); line(13,13,16,10) }
        } else if (name === "shield") {
            c.beginPath(); c.moveTo(12,2); c.lineTo(21,6); c.lineTo(20,14); c.quadraticCurveTo(18,20,12,23); c.quadraticCurveTo(6,20,4,14); c.lineTo(3,6); c.closePath(); c.stroke()
            line(8,12,11,15); line(11,15,17,9)
        } else if (name === "file") {
            c.beginPath(); c.moveTo(5,2); c.lineTo(14,2); c.lineTo(20,8); c.lineTo(20,22); c.lineTo(5,22); c.closePath(); c.stroke()
            line(14,2,14,8); line(14,8,20,8); line(9,14,16,14); line(9,18,14,18)
        } else if (name === "bolt") {
            c.beginPath(); c.moveTo(14,2); c.lineTo(5,14); c.lineTo(11,14); c.lineTo(10,23); c.lineTo(20,10); c.lineTo(13,10); c.closePath(); c.stroke()
        } else if (name === "copy") {
            c.strokeRect(8,7,12,14); c.beginPath(); c.moveTo(5,17); c.lineTo(3,17); c.lineTo(3,2); c.lineTo(15,2); c.lineTo(15,4); c.stroke()
        } else if (name === "refresh") {
            c.beginPath(); c.arc(12,12,8,0.6,5.5); c.stroke(); line(18,4,18,10); line(18,10,12,10)
        } else if (name === "arrow") {
            line(3,12,21,12); line(15,6,21,12); line(21,12,15,18)
        } else if (name === "cloud") {
            c.beginPath(); c.moveTo(6,19); c.bezierCurveTo(-1,19,0,9,7,10); c.bezierCurveTo(8,0,22,2,20,11); c.bezierCurveTo(27,12,24,20,18,19); c.closePath(); c.stroke()
        } else if (name === "heart") {
            c.beginPath(); c.moveTo(12,21); c.bezierCurveTo(-8,8,8,-4,12,6); c.bezierCurveTo(16,-4,32,8,12,21); c.stroke()
        } else if (name === "history") {
            c.beginPath(); c.arc(12,12,9,0,Math.PI*2); c.stroke(); line(12,6,12,12); line(12,12,16,14)
        } else if (name === "settings") {
            c.beginPath(); c.arc(12,12,4,0,Math.PI*2); c.stroke()
            for (let i=0;i<8;i++) { let a=i*Math.PI/4; line(12+8*Math.cos(a),12+8*Math.sin(a),12+11*Math.cos(a),12+11*Math.sin(a)) }
        } else if (name === "lock") {
            c.strokeRect(4,10,16,12); c.beginPath(); c.arc(12,9,5,Math.PI,0); c.stroke(); line(12,15,12,18)
        } else {
            c.beginPath(); c.arc(12,12,9,0,Math.PI*2); c.stroke(); line(12,10,12,17); line(12,6,12,6.3)
        }
    }
}
