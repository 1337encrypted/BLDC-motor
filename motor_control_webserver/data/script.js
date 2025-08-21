// Motor Control Dashboard JavaScript - HTTP API Version (ESP32 Optimized)
class MotorControlAPI {
    constructor() {
        this.baseUrl = window.location.origin;
        this.isConnected = false;
        this.lastDataUpdate = 0;
        this.updateInterval = null;
        this.connectionCheckInterval = null;
        
        // Cache DOM elements for better performance
        this.domCache = {};
        this.cacheDOMElements();
        
        this.init();
    }
    
    cacheDOMElements() {
        // Cache frequently accessed DOM elements
        this.domCache = {
            statusIndicator: document.getElementById('statusIndicator'),
            statusText: document.getElementById('statusText'),
            commandStatus: document.getElementById('commandStatus'),
            emergencyIndicator: document.getElementById('emergencyIndicator'),
            pwmSlider: document.getElementById('pwmSlider'),
            pwmValue: document.getElementById('pwmValue'),
            lastUpdate: document.getElementById('lastUpdate'),
            toastContainer: document.getElementById('toastContainer'),
            systemVoltage: document.getElementById('systemVoltage'),
            wifiSignal: document.getElementById('wifiSignal'),
            systemUptime: document.getElementById('systemUptime'),
            freeHeap: document.getElementById('freeHeap'),
            motor1Speed: document.getElementById('motor1Speed'),
            motor1Current: document.getElementById('motor1Current'),
            motor2Speed: document.getElementById('motor2Speed'),
            motor2Current: document.getElementById('motor2Current'),
            motor3Speed: document.getElementById('motor3Speed'),
            motor3Current: document.getElementById('motor3Current'),
            motor4Speed: document.getElementById('motor4Speed'),
            motor4Current: document.getElementById('motor4Current'),
            leftDoorBtn: document.getElementById('leftDoorBtn'),
            rightDoorBtn: document.getElementById('rightDoorBtn'),
            leftLightBtn: document.getElementById('leftLightBtn'),
            rightLightBtn: document.getElementById('rightLightBtn')
        };
    }
    
    init() {
        this.setupEventListeners();
        this.updateConnectionStatus(false, 'Connecting...');
        this.startDataPolling();
        this.startConnectionMonitoring();
    }
    
    async fetchWithTimeout(url, options = {}, timeout = 3000) { // Reduced timeout for ESP32
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), timeout);
        
        try {
            const response = await fetch(url, {
                ...options,
                signal: controller.signal
            });
            clearTimeout(timeoutId);
            return response;
        } catch (error) {
            clearTimeout(timeoutId);
            throw error;
        }
    }
    
    async getMotorStatus() {
        try {
            const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/status`);
            if (response.ok) {
                const data = await response.json();
                this.updateDashboard(data);
                this.lastDataUpdate = Date.now();
                this.updateLastUpdateTime();
                this.updateConnectionStatus(true, 'Connected');
                return data;
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            console.error('Error fetching motor status:', error);
            this.updateConnectionStatus(false, 'Connection Error');
            throw error;
        }
    }
    
    async sendMotorCommand(command) {
        try {
            const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/control`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(command)
            });
            
            if (response.ok) {
                const result = await response.json();
                this.updateCommandStatus('success');
                return result;
            } else {
                const errorData = await response.json().catch(() => ({}));
                throw new Error(errorData.error || `HTTP ${response.status}`);
            }
        } catch (error) {
            console.error('Error sending motor command:', error);
            this.updateCommandStatus('error');
            throw error;
        }
    }
    
    updateDashboard(data) {
        // Update motor speeds - optimized for 4 motors
        const motors = ['motor1', 'motor2', 'motor3', 'motor4'];
        motors.forEach((motor, index) => {
            if (data[motor]) {
                const speedEl = this.domCache[`${motor}Speed`];
                const currentEl = this.domCache[`${motor}Current`];
                const indicatorEl = document.getElementById(`${motor}Indicator`);
                
                if (speedEl) {
                    speedEl.innerHTML = `${data[motor].speed || 0} <span class="unit">RPM</span>`;
                }
                if (currentEl) {
                    currentEl.innerHTML = `${(data[motor].current || 0).toFixed(1)} <span class="unit">A</span>`;
                }
                if (indicatorEl) {
                    this.updateMotorIndicator(indicatorEl, data[motor].speed > 0);
                }
            }
        });
        
        // Update PWM control with improved synchronization
        if (data.pwm !== undefined && this.domCache.pwmSlider && this.domCache.pwmValue) {
            // Only update if value actually changed to avoid input interference
            if (parseInt(this.domCache.pwmSlider.value) !== data.pwm) {
                this.domCache.pwmSlider.value = data.pwm;
                this.domCache.pwmValue.textContent = data.pwm;
                console.log('PWM slider synchronized to:', data.pwm);
            }
        }
        
        // Update system info
        if (data.voltage !== undefined && this.domCache.systemVoltage) {
            this.domCache.systemVoltage.textContent = `${data.voltage.toFixed(1)}V`;
        }
        
        if (data.wifi_rssi !== undefined && this.domCache.wifiSignal) {
            this.domCache.wifiSignal.textContent = `${data.wifi_rssi} dBm`;
        }
        
        if (data.uptime !== undefined && this.domCache.systemUptime) {
            this.domCache.systemUptime.textContent = this.formatUptime(data.uptime);
        }
        
        if (data.heap !== undefined && this.domCache.freeHeap) {
            this.domCache.freeHeap.textContent = `${Math.round(data.heap / 1024)} KB`;
        }
        
        // Update control states with enhanced synchronization
        if (data.doors !== undefined) {
            this.updateLeftDoorButton(data.doors.left_locked || false);
            this.updateRightDoorButton(data.doors.right_locked || false);
            
            // Log door state changes for debugging
            console.log('Door states updated:', {
                left: data.doors.left_locked || false,
                right: data.doors.right_locked || false
            });
        }
        
        if (data.lights !== undefined) {
            this.updateLeftLightButton(data.lights.left_on || false);
            this.updateRightLightButton(data.lights.right_on || false);
            
            // Log light state changes for debugging
            console.log('Light states updated:', {
                left: data.lights.left_on || false,
                right: data.lights.right_on || false
            });
        }
    }
    
    updateMotorIndicator(motorStatusElement, isActive) {
        const statusIndicator = motorStatusElement.querySelector('.status-dot');
        if (statusIndicator) {
            if (isActive) {
                statusIndicator.classList.remove('offline');
                statusIndicator.classList.add('online');
            } else {
                statusIndicator.classList.remove('online');
                statusIndicator.classList.add('offline');
            }
        }
    }
    
    updateLeftDoorButton(locked) {
        const btn = this.domCache.leftDoorBtn;
        if (!btn) {
            console.warn('Left door button not found in DOM cache');
            return;
        }
        
        const icon = btn.querySelector('.hardware-icon');
        if (!icon) {
            console.warn('Left door button icon not found');
            return;
        }
        
        // Only update if state actually changed to avoid unnecessary DOM manipulation
        const isCurrentlyActive = btn.classList.contains('active');
        if (isCurrentlyActive === locked) return;
        
        console.log('Updating left door button state:', locked ? 'LOCKED' : 'UNLOCKED');
        
        if (locked) {
            btn.classList.add('active');
            // Lock icon - closed lock
            icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 10 0v4"/>';
        } else {
            btn.classList.remove('active');
            // Unlock icon - open lock
            icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 8.5 0v2.5"/>';
        }
        
        // Add visual feedback animation
        btn.style.transform = 'scale(0.95)';
        setTimeout(() => {
            btn.style.transform = 'scale(1)';
        }, 100);
    }
    
    updateRightDoorButton(locked) {
        const btn = this.domCache.rightDoorBtn;
        if (!btn) {
            console.warn('Right door button not found in DOM cache');
            return;
        }
        
        const icon = btn.querySelector('.hardware-icon');
        if (!icon) {
            console.warn('Right door button icon not found');
            return;
        }
        
        // Only update if state actually changed to avoid unnecessary DOM manipulation
        const isCurrentlyActive = btn.classList.contains('active');
        if (isCurrentlyActive === locked) return;
        
        console.log('Updating right door button state:', locked ? 'LOCKED' : 'UNLOCKED');
        
        if (locked) {
            btn.classList.add('active');
            // Lock icon - closed lock
            icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 10 0v4"/>';
        } else {
            btn.classList.remove('active');
            // Unlock icon - open lock
            icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 8.5 0v2.5"/>';
        }
        
        // Add visual feedback animation
        btn.style.transform = 'scale(0.95)';
        setTimeout(() => {
            btn.style.transform = 'scale(1)';
        }, 100);
    }
    
    updateLeftLightButton(on) {
        const btn = this.domCache.leftLightBtn;
        if (!btn) {
            console.warn('Left light button not found in DOM cache');
            return;
        }
        
        const icon = btn.querySelector('.hardware-icon');
        if (!icon) {
            console.warn('Left light button icon not found');
            return;
        }
        
        // Only update if state actually changed to avoid unnecessary DOM manipulation
        const isCurrentlyActive = btn.classList.contains('active');
        if (isCurrentlyActive === on) return;
        
        console.log('Updating left light button state:', on ? 'ON' : 'OFF');
        
        if (on) {
            btn.classList.add('active');
            // Light On icon - sun
            icon.innerHTML = '<circle cx="12" cy="12" r="4"/><path d="M12 2v2"/><path d="M12 20v2"/><path d="m4.93 4.93 1.41 1.41"/><path d="m17.66 17.66 1.41 1.41"/><path d="M2 12h2"/><path d="M20 12h2"/><path d="m6.34 17.66-1.41 1.41"/><path d="m19.07 4.93-1.41 1.41"/>';
        } else {
            btn.classList.remove('active');
            // Light Off icon - moon
            icon.innerHTML = '<path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z"/>';
        }
        
        // Add visual feedback animation
        btn.style.transform = 'scale(0.95)';
        setTimeout(() => {
            btn.style.transform = 'scale(1)';
        }, 100);
    }
    
    updateRightLightButton(on) {
        const btn = this.domCache.rightLightBtn;
        if (!btn) {
            console.warn('Right light button not found in DOM cache');
            return;
        }
        
        const icon = btn.querySelector('.hardware-icon');
        if (!icon) {
            console.warn('Right light button icon not found');
            return;
        }
        
        // Only update if state actually changed to avoid unnecessary DOM manipulation
        const isCurrentlyActive = btn.classList.contains('active');
        if (isCurrentlyActive === on) return;
        
        console.log('Updating right light button state:', on ? 'ON' : 'OFF');
        
        if (on) {
            btn.classList.add('active');
            // Light On icon - sun
            icon.innerHTML = '<circle cx="12" cy="12" r="4"/><path d="M12 2v2"/><path d="M12 20v2"/><path d="m4.93 4.93 1.41 1.41"/><path d="m17.66 17.66 1.41 1.41"/><path d="M2 12h2"/><path d="M20 12h2"/><path d="m6.34 17.66-1.41 1.41"/><path d="m19.07 4.93-1.41 1.41"/>';
        } else {
            btn.classList.remove('active');
            // Light Off icon - moon
            icon.innerHTML = '<path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z"/>';
        }
        
        // Add visual feedback animation
        btn.style.transform = 'scale(0.95)';
        setTimeout(() => {
            btn.style.transform = 'scale(1)';
        }, 100);
    }
    
    async setPWM(value) {
        try {
            await this.sendMotorCommand({ pwm: parseInt(value) });
        } catch (error) {
            console.error('Failed to set PWM:', error);
        }
    }
    
    async toggleLeftDoor() {
        try {
            // Get current state and toggle immediately
            const btn = this.domCache.leftDoorBtn;
            const isCurrentlyLocked = btn.classList.contains('active');
            const newState = !isCurrentlyLocked;
            
            // Update UI immediately
            this.updateLeftDoorButton(newState);
            
            // Send command to server
            await this.sendMotorCommand({
                doors: {
                    left: "toggle"
                }
            });
        } catch (error) {
            console.error('Failed to toggle left door:', error);
            // Revert on error
            const btn = this.domCache.leftDoorBtn;
            const isCurrentlyLocked = btn.classList.contains('active');
            this.updateLeftDoorButton(!isCurrentlyLocked);
        }
    }
    
    async toggleRightDoor() {
        try {
            // Get current state and toggle immediately
            const btn = this.domCache.rightDoorBtn;
            const isCurrentlyLocked = btn.classList.contains('active');
            const newState = !isCurrentlyLocked;
            
            // Update UI immediately
            this.updateRightDoorButton(newState);
            
            // Send command to server
            await this.sendMotorCommand({
                doors: {
                    right: "toggle"
                }
            });
        } catch (error) {
            console.error('Failed to toggle right door:', error);
            // Revert on error
            const btn = this.domCache.rightDoorBtn;
            const isCurrentlyLocked = btn.classList.contains('active');
            this.updateRightDoorButton(!isCurrentlyLocked);
        }
    }
    
    async toggleLeftLight() {
        try {
            // Get current state and toggle immediately
            const btn = this.domCache.leftLightBtn;
            const isCurrentlyOn = btn.classList.contains('active');
            const newState = !isCurrentlyOn;
            
            // Update UI immediately
            this.updateLeftLightButton(newState);
            
            // Send command to server
            await this.sendMotorCommand({
                lights: {
                    left: "toggle"
                }
            });
        } catch (error) {
            console.error('Failed to toggle left light:', error);
            // Revert on error
            const btn = this.domCache.leftLightBtn;
            const isCurrentlyOn = btn.classList.contains('active');
            this.updateLeftLightButton(!isCurrentlyOn);
        }
    }
    
    async toggleRightLight() {
        try {
            // Get current state and toggle immediately
            const btn = this.domCache.rightLightBtn;
            const isCurrentlyOn = btn.classList.contains('active');
            const newState = !isCurrentlyOn;
            
            // Update UI immediately
            this.updateRightLightButton(newState);
            
            // Send command to server
            await this.sendMotorCommand({
                lights: {
                    right: "toggle"
                }
            });
        } catch (error) {
            console.error('Failed to toggle right light:', error);
            // Revert on error
            const btn = this.domCache.rightLightBtn;
            const isCurrentlyOn = btn.classList.contains('active');
            this.updateRightLightButton(!isCurrentlyOn);
        }
    }
    
    async emergencyStop() {
        try {
            // Update emergency indicator immediately
            this.updateEmergencyIndicator(true);
            
            await this.sendMotorCommand({ pwm: 0 });
            
            // Reset UI to safe state
            if (this.domCache.pwmSlider) this.domCache.pwmSlider.value = 0;
            if (this.domCache.pwmValue) this.domCache.pwmValue.textContent = '0';
            
            // Reset emergency indicator after 3 seconds
            setTimeout(() => {
                this.updateEmergencyIndicator(false);
            }, 3000);
        } catch (error) {
            console.error('Failed to emergency stop:', error);
            // Reset emergency indicator on error
            this.updateEmergencyIndicator(false);
        }
    }
    
    updateEmergencyIndicator(active) {
        const emergencyButton = document.querySelector('.emergency-button');
        if (emergencyButton) {
            if (active) {
                emergencyButton.classList.add('emergency-active');
            } else {
                emergencyButton.classList.remove('emergency-active');
            }
        }
    }
    
    startDataPolling() {
        // Poll for data every 1 second for better responsiveness
        this.updateInterval = setInterval(async () => {
            try {
                await this.getMotorStatus();
            } catch (error) {
                // Error handling is done in getMotorStatus
                console.error('Data polling error:', error);
            }
        }, 1000);
    }
    
    startConnectionMonitoring() {
        // Check connection status every 10 seconds (reduced frequency)
        this.connectionCheckInterval = setInterval(() => {
            if (Date.now() - this.lastDataUpdate > 15000) { // Increased timeout
                this.updateConnectionStatus(false, 'No data received');
            }
        }, 10000);
    }
    
    updateConnectionStatus(connected, statusText) {
        if (this.domCache.statusIndicator) {
            if (connected) {
                this.domCache.statusIndicator.classList.add('connected');
                this.domCache.statusIndicator.classList.remove('disconnected', 'error');
            } else {
                this.domCache.statusIndicator.classList.remove('connected');
                this.domCache.statusIndicator.classList.add('disconnected');
            }
        }
        
        if (this.domCache.statusText) {
            this.domCache.statusText.textContent = statusText;
        }
        
        this.isConnected = connected;
    }
    
    formatUptime(milliseconds) {
        const seconds = Math.floor(milliseconds / 1000);
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;
        
        return `${hours}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }
    
    updateLastUpdateTime() {
        const now = new Date();
        const timeStr = now.toLocaleTimeString();
        if (this.domCache.lastUpdate) {
            this.domCache.lastUpdate.textContent = `Last update: ${timeStr}`;
        }
    }
    
    updateCommandStatus(status, duration = 2000) {
        if (!this.domCache.commandStatus) return;
        
        const statusElement = this.domCache.commandStatus;
        const iconElement = statusElement.querySelector('.status-icon');
        
        // Remove existing classes
        statusElement.classList.remove('success', 'error', 'idle');
        
        // Add new status class
        statusElement.classList.add(status);
        
        // Update icon based on status
        if (status === 'success') {
            // Success icon (checkmark)
            iconElement.innerHTML = '<circle cx="12" cy="12" r="10"/><path d="m9 12 2 2 4-4"/>';
        } else if (status === 'error') {
            // Error icon (X mark)
            iconElement.innerHTML = '<circle cx="12" cy="12" r="10"/><path d="m15 9-6 6"/><path d="m9 9 6 6"/>';
        } else {
            // Idle icon (dash)
            iconElement.innerHTML = '<circle cx="12" cy="12" r="10"/><path d="M8 12h8"/>';
        }
        
        // Reset to idle after duration (longer for errors)
        if (status !== 'idle') {
            const resetDuration = status === 'error' ? 4000 : duration;
            setTimeout(() => {
                this.updateCommandStatus('idle');
            }, resetDuration);
        }
    }
    
    showToast(message, type = 'info') {
        if (!this.domCache.toastContainer) return;
        
        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.textContent = message;
        
        this.domCache.toastContainer.appendChild(toast);
        
        // Trigger animation
        setTimeout(() => toast.classList.add('show'), 10);
        
        // Remove after 5 seconds
        setTimeout(() => {
            toast.classList.remove('show');
            setTimeout(() => {
                if (toast.parentNode) {
                    toast.parentNode.removeChild(toast);
                }
            }, 300);
        }, 5000);
    }
    
    setupEventListeners() {
        // Connection monitoring is handled by startConnectionMonitoring
    }
    
    destroy() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
        }
        if (this.connectionCheckInterval) {
            clearInterval(this.connectionCheckInterval);
        }
        // Clear DOM cache
        this.domCache = {};
    }
}

// Global API instance
let motorAPI;

// Global functions for HTML event handlers - optimized
function updatePWM(value) {
    if (motorAPI && motorAPI.domCache.pwmValue) {
        motorAPI.domCache.pwmValue.textContent = value;
    }
}

function setPWM(value) {
    if (motorAPI) {
        motorAPI.setPWM(value);
    }
}

function setPresetPWM(value) {
    if (motorAPI && motorAPI.domCache.pwmSlider) {
        motorAPI.domCache.pwmSlider.value = value;
        updatePWM(value);
        setPWM(value);
    }
}

function toggleLeftDoor() {
    if (motorAPI) motorAPI.toggleLeftDoor();
}

function toggleRightDoor() {
    if (motorAPI) motorAPI.toggleRightDoor();
}

function toggleLeftLight() {
    if (motorAPI) motorAPI.toggleLeftLight();
}

function toggleRightLight() {
    if (motorAPI) motorAPI.toggleRightLight();
}

function emergencyStop() {
    if (confirm('Are you sure you want to activate emergency stop?')) {
        if (motorAPI) motorAPI.emergencyStop();
    }
}

// Test function for debugging icon toggles
function testIconToggles() {
    if (!motorAPI) return;
    
    console.log('Testing icon toggles...');
    
    // Test door toggles
    motorAPI.updateLeftDoorButton(true);
    motorAPI.updateRightDoorButton(true);
    setTimeout(() => {
        motorAPI.updateLeftDoorButton(false);
        motorAPI.updateRightDoorButton(false);
    }, 2000);
    
    // Test light toggles
    motorAPI.updateLeftLightButton(true);
    motorAPI.updateRightLightButton(true);
    setTimeout(() => {
        motorAPI.updateLeftLightButton(false);
        motorAPI.updateRightLightButton(false);
    }, 4000);
}

// WiFi Mode Management Functions
async function checkWiFiMode() {
    try {
        const response = await fetch('/api/wifi/mode');
        if (response.ok) {
            const wifiInfo = await response.json();
            updateWiFiModeDisplay(wifiInfo);
        }
    } catch (error) {
        console.error('Failed to check WiFi mode:', error);
    }
}

async function switchWiFiMode() {
    if (!confirm('Switch WiFi mode? System will restart and you may need to reconnect.')) {
        return;
    }
    
    try {
        const response = await fetch('/api/wifi/mode');
        if (response.ok) {
            const currentInfo = await response.json();
            const newMode = currentInfo.currentMode === 'AP' ? 'Station' : 'AP';
            
            const switchResponse = await fetch('/api/wifi/switch', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode: newMode })
            });
            
            if (switchResponse.ok) {
                if (motorAPI) motorAPI.showToast('WiFi mode switching... System will restart', 'info');
                // Disable UI during restart
                document.body.style.pointerEvents = 'none';
                document.body.style.opacity = '0.7';
            }
        }
    } catch (error) {
        console.error('Failed to switch WiFi mode:', error);
        if (motorAPI) motorAPI.showToast('Failed to switch WiFi mode', 'error');
    }
}

function updateWiFiModeDisplay(wifiInfo) {
    // Update any existing WiFi mode indicators
    const modeIndicators = document.querySelectorAll('.wifi-mode-indicator');
    modeIndicators.forEach(indicator => {
        indicator.textContent = wifiInfo.modeDescription;
        indicator.className = `wifi-mode-indicator ${wifiInfo.currentMode.toLowerCase()}`;
    });
}

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    console.log('Motor Control Dashboard initializing...');
    motorAPI = new MotorControlAPI();
    
    // Check WiFi mode on startup
    setTimeout(() => {
        checkWiFiMode();
    }, 2000);
});

// Handle window events
window.addEventListener('beforeunload', function() {
    if (motorAPI) {
        motorAPI.destroy();
    }
});

window.addEventListener('online', function() {
    if (motorAPI) motorAPI.showToast('Network connection restored', 'success');
});

window.addEventListener('offline', function() {
    if (motorAPI) motorAPI.showToast('Network connection lost', 'warning');
});