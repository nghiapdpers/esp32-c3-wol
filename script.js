// --- Configuration ---
const MQTT_BROKER = 'wss://broker.emqx.io:8084/mqtt'; // Kết nối qua WebSockets Secure
let client = null;
let secretKey = localStorage.getItem('wol_secret_key') || '';

// --- Parse URL Parameters ---
const urlParams = new URLSearchParams(window.location.search);
const keyFromUrl = urlParams.get('key');
if (keyFromUrl) {
    secretKey = keyFromUrl;
    localStorage.setItem('wol_secret_key', secretKey);
    // Xóa param trên URL cho đẹp sau khi đã lưu
    window.history.replaceState({}, document.title, window.location.pathname);
}

let topicCmd = '';
let topicRes = '';

// --- DOM Elements ---
const deviceList = document.getElementById('device-list');
const statusText = document.getElementById('status-text');
const settingsModal = document.getElementById('settings-modal');
const secretInput = document.getElementById('secret-key');

// --- MQTT Connection ---
function connectMQTT() {
    if (!secretKey) {
        showSettings();
        return;
    }

    topicCmd = `esp32_c3_wol/${secretKey}/cmd`;
    topicRes = `esp32_c3_wol/${secretKey}/res`;

    if (client) client.end();

    statusText.innerText = "Đang kết nối MQTT...";

    client = mqtt.connect(MQTT_BROKER);

    client.on('connect', () => {
        statusText.innerText = "Đã kết nối Remote";
        statusText.style.color = "#4ade80";
        client.subscribe(topicRes);
        // Gửi lệnh sync ngay khi kết nối
        sendCmd({ cmd: 'sync' });
    });

    client.on('message', (topic, message) => {
        const data = JSON.parse(message.toString());
        if (data.type === 'list') {
            renderDevices(data.devices);
            document.getElementById('uptime').innerText = data.uptime;
            document.getElementById('rssi').innerText = data.rssi + " dBm";
        }
    });

    client.on('error', (err) => {
        statusText.innerText = "Lỗi kết nối MQTT";
        statusText.style.color = "#f87171";
    });
}

function sendCmd(obj) {
    if (client && client.connected) {
        client.publish(topicCmd, JSON.stringify(obj));
    }
}

// --- UI Logic ---
function renderDevices(devices) {
    deviceList.innerHTML = '';
    if (devices.length === 0) {
        deviceList.innerHTML = '<div class="empty-state">Chưa có thiết bị nào</div>';
        return;
    }

    devices.forEach(dev => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
            <div class="device-info">
                <h3>${dev.name}</h3>
                <p>${dev.mac}</p>
            </div>
            <div class="device-actions">
                <button class="wake-btn" onclick="wakeDevice('${dev.mac}', '${dev.name}')">Wake</button>
                <button class="delete-btn" onclick="deleteDevice('${dev.name}')">🗑</button>
            </div>
        `;
        deviceList.appendChild(card);
    });
}

function wakeDevice(mac, name) {
    sendCmd({ cmd: 'wol', mac: mac, name: name });
    // Feedback ngay lập tức
    const btn = event.target;
    btn.innerText = "Sending...";
    btn.disabled = true;
    setTimeout(() => {
        btn.innerText = "Wake";
        btn.disabled = false;
    }, 2000);
}

function deleteDevice(name) {
    if (confirm(`Xóa thiết bị ${name}?`)) {
        sendCmd({ cmd: 'delete', name: name });
    }
}

function showSettings() {
    settingsModal.style.display = 'flex';
    secretInput.value = secretKey;
}

// --- Event Listeners ---
document.getElementById('settings-btn').onclick = showSettings;

document.getElementById('settings-close-btn').onclick = () => {
    secretKey = secretInput.value.trim();
    if (secretKey) {
        localStorage.setItem('wol_secret_key', secretKey);
        settingsModal.style.display = 'none';
        connectMQTT();
    } else {
        alert("Vui lòng nhập Secret Key!");
    }
};

document.getElementById('add-btn').onclick = () => {
    document.getElementById('modal').style.display = 'flex';
};

document.getElementById('cancel-btn').onclick = () => {
    document.getElementById('modal').style.display = 'none';
};

document.getElementById('save-btn').onclick = () => {
    const name = document.getElementById('pc-name').value;
    const mac = document.getElementById('pc-mac').value;
    if (name && mac) {
        sendCmd({ cmd: 'add', name: name, mac: mac });
        document.getElementById('modal').style.display = 'none';
        // Reset form
        document.getElementById('pc-name').value = '';
        document.getElementById('pc-mac').value = '';
    }
};

// Khởi tạo
connectMQTT();
