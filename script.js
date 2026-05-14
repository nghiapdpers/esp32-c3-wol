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
let topicStatus = '';

// --- State ---
let onlineStatus = {}; // { macWithoutColons: true/false }

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
    topicStatus = `esp32_c3_wol/${secretKey}/status/#`;

    if (client) client.end();

    statusText.innerText = "Đang kết nối MQTT...";
    statusText.style.color = "var(--text-dim)";

    client = mqtt.connect(MQTT_BROKER);

    client.on('connect', () => {
        statusText.innerText = "Đã kết nối Remote";
        statusText.style.color = "#4ade80";
        client.subscribe(topicRes);
        client.subscribe(topicStatus);
        sendCmd({ cmd: 'sync' });
    });

    client.on('message', (topic, message) => {
        const payload = message.toString();
        
        // Xử lý Topic Status (Heartbeat từ Agent)
        const statusMatch = topic.match(/status\/([0-9A-F]+)$/i);
        if (statusMatch) {
            const mac = statusMatch[1].toUpperCase();
            onlineStatus[mac] = (payload === 'online');
            updateDeviceStatusUI(mac);
            return;
        }

        // Xử lý Topic Response (Dữ liệu từ ESP32)
        try {
            const data = JSON.parse(payload);
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

function normalizeMac(mac) {
    return mac.replace(/[:.-]/g, '').toUpperCase();
}

// --- UI Logic ---
function renderDevices(devices) {
    deviceList.innerHTML = '';
    if (devices.length === 0) {
        deviceList.innerHTML = '<div class="empty-state">Chưa có thiết bị nào trong danh sách</div>';
        return;
    }

    devices.forEach(dev => {
        const nMac = normalizeMac(dev.mac);
        const isOnline = onlineStatus[nMac] || false;
        
        const card = document.createElement('div');
        card.className = 'device-card';
        card.setAttribute('data-mac', nMac);
        card.innerHTML = `
            <div class="device-header">
                <div class="status-badge ${isOnline ? 'status-online' : 'status-offline'}">
                    <div class="status-dot"></div>
                    <span>${isOnline ? 'Online' : 'Offline'}</span>
                </div>
            </div>
            <div class="device-info">
                <h3>${dev.name}</h3>
                <p>${dev.mac}</p>
            </div>
            <div class="device-actions">
                <button class="wake-btn" title="Bật máy">🚀 Wake</button>
                <button class="shutdown-btn" title="Tắt máy">🛑 Off</button>
                <button class="delete-btn" title="Xóa">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/><line x1="10" y1="11" x2="10" y2="17"/><line x1="14" y1="11" x2="14" y2="17"/></svg>
                </button>
            </div>
        `;
        
        // Gán sự kiện
        card.querySelector('.wake-btn').onclick = (e) => {
            wakeDevice(dev.mac, dev.name, e.currentTarget);
        };
        card.querySelector('.shutdown-btn').onclick = (e) => {
            shutdownDevice(dev.mac, dev.name, e.currentTarget);
        };
        card.querySelector('.delete-btn').onclick = () => {
            deleteDevice(dev.name);
        };
        
        deviceList.appendChild(card);
    });
}

function updateDeviceStatusUI(mac) {
    const card = document.querySelector(`.device-card[data-mac="${mac}"]`);
    if (card) {
        const isOnline = onlineStatus[mac];
        const badge = card.querySelector('.status-badge');
        badge.className = `status-badge ${isOnline ? 'status-online' : 'status-offline'}`;
        badge.querySelector('span').innerText = isOnline ? 'Online' : 'Offline';
    }
}

function wakeDevice(mac, name, btn) {
    sendCmd({ cmd: 'wol', mac: mac, name: name });
    const originalText = btn.innerText;
    btn.innerText = "🚀 Sent";
    btn.disabled = true;
    setTimeout(() => {
        btn.innerText = originalText;
        btn.disabled = false;
    }, 2000);
}

function shutdownDevice(mac, name, btn) {
    if (confirm(`Gửi lệnh tắt máy đến "${name}"?`)) {
        sendCmd({ cmd: 'shutdown', mac: mac, name: name });
        const originalText = btn.innerText;
        btn.innerText = "🛑 Sent";
        btn.disabled = true;
        setTimeout(() => {
            btn.innerText = originalText;
            btn.disabled = false;
        }, 2000);
    }
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
