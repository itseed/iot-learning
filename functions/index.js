const functions = require('firebase-functions');
const request = require('request-promise');
const admin = require('firebase-admin');

// // Create and Deploy Your First Cloud Functions
// // https://firebase.google.com/docs/functions/write-firebase-functions
//

const hardwareId = 'abcd';

var serviceAccount = require("./serviceAccountKey.json");
admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://your-host.firebaseio.com/"
});

const LINE_TOKEN = 'Your Token Key';
const LINE_MESSAGING_API = 'https://api.line.me/v2/bot/message';
const LINE_HEADER = {
    'Content-Type': 'application/json',
    'Authorization': `Bearer ${LINE_TOKEN}`
}

exports.LineBot = functions.region('asia-east2').https.onRequest((request, response) => {
    //response.send("Hello from Firebase!");
    if (request.body.events[0].message.type !== 'text') {
        replyText(request, 'ไม่พบสิ่งที่คุณต้องการค้นหา');
        return;
    }

    const contentText = request.body.events[0].message.text;
    var ref = admin.database().ref('hwStatus/' + hardwareId);
    switch (contentText.toLowerCase()) {
        case "เปิดไฟ": {
            ref.child('switch').set(1);
            replyText(request, 'ระบบเปิดไฟแล้วค่ะ');
            break;
        }
        case "ปิดไฟ": {
            ref.child('switch').set(0);
            replyText(request, 'ระบบปิดไฟแล้วค่ะ');
            break;
        }
        case "สถานะ": {
            admin.database().ref('hwStatus/' + hardwareId).once('value', (snapshot) => {
                var event = snapshot.val();

                var switchState = 'ปิด';
                if (event.switch === 1) {
                    switchState = 'เปิด';
                }

                replyText(request, `อุณหภูมิห้อง ${event.temperature}°C \r\n ค่าความชื้น ${event.humidity}% \r\n สวิตไฟ ${switchState}`);
            });
            
            break;
        }
        default: {
            replyText(request, 'ไม่พบสิ่งที่คุณต้องการค้นหา');
            break;
        }
    }
});

const replyText = (req, msg) => {
    return request({
        method: `POST`,
        uri: `${LINE_MESSAGING_API}/reply`,
        headers: LINE_HEADER,
        body: JSON.stringify({
            replyToken: req.body.events[0].replyToken,
            messages: [
                {
                    type: `text`,
                    text: msg
                }
            ]
        })
    });
};
