// --- Configuration ---
const MQTT_BROKER = 'wss://broker.emqx.io:8084/mqtt';
let client = null;
let secretKey = localStorage.getItem('wol_secret_key') || '';

// --- Parse URL Parameters ---
const urlParams = new URLSearchParams(window.location.search);
const keyFromUrl = urlParams.get('key');
if (keyFromUrl) {
    secretKey = keyFromUrl;
    localStorage.setItem('wol_secret_key', secretKey);
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
    statusText.style.color = "var(--text-dim)";

    client = mqtt.connect(MQTT_BROKER);

    client.on('connect', () => {
        statusText.innerText = "Đã kết nối Remote";
        statusText.style.color = "#4ade80";
        client.subscribe(topicRes);
        sendCmd({ cmd: 'sync' });
    });

    client.on('message', (topic, message) => {
        try {
            const data = JSON.parse(message.toString());
            if (data.type === 'list') {
                renderDevices(data.devices);
                document.getElementById('uptime').innerText = data.uptime || '--';
                document.getElementById('rssi').innerText = (data.rssi || '--') + " dBm";
                document.getElementById('heap').innerText = (data.heap || '--') + " KB";
            }
        } catch (e) {
            console.error("Lỗi parse JSON:", e);
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
        deviceList.innerHTML = '<div class="empty-state">Chưa có thiết bị nào trong danh sách</div>';
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
                <button class="wake-btn">Đánh thức</button>
                <button class="delete-btn">🗑</button>
            </div>
        `;
        
        // Gán sự kiện
        card.querySelector('.wake-btn').onclick = (e) => {
            wakeDevice(dev.mac, dev.name, e.target);
        };
        card.querySelector('.delete-btn').onclick = () => {
            deleteDevice(dev.name);
        };
        
        deviceList.appendChild(card);
    });
}

function wakeDevice(mac, name, btn) {
    sendCmd({ cmd: 'wol', mac: mac, name: name });
    const originalText = btn.innerText;
    btn.innerText = "🚀 Đang gửi...";
    btn.disabled = true;
    setTimeout(() => {
        btn.innerText = originalText;
        btn.disabled = false;
    }, 2000);
}

function deleteDevice(name) {
    if (confirm(`Bạn có chắc chắn muốn xóa thiết bị "${name}"?`)) {
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
    const val = secretInput.value.trim();
    if (val) {
        secretKey = val;
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
    const nameInput = document.getElementById('pc-name');
    const macInput = document.getElementById('pc-mac');
    const name = nameInput.value.trim();
    const mac = macInput.value.trim();
    
    if (name && mac) {
        sendCmd({ cmd: 'add', name: name, mac: mac });
        document.getElementById('modal').style.display = 'none';
        nameInput.value = '';
        macInput.value = '';
    } else {
        alert("Vui lòng điền đầy đủ Tên và MAC!");
    }
};

// Đóng modal khi click ra ngoài
window.onclick = (event) => {
    if (event.target == document.getElementById('modal')) {
        document.getElementById('modal').style.display = 'none';
    }
    if (event.target == document.getElementById('settings-modal')) {
        document.getElementById('settings-modal').style.display = 'none';
    }
};

// Khởi tạo
connectMQTT();
