// Motor Control Dashboard JavaScript - HTTP API Version
class MotorControlAPI {
    constructor() {
        this.baseUrl = window.location.origin;
        this.isConnected = false;
        this.lastDataUpdate = 0;
        this.updateInterval = null;
        this.connectionCheckInterval = null;
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.updateConnectionStatus(false, 'Connecting...');
        this.startDataPolling();
        this.startConnectionMonitoring();
    }
    
    async fetchWithTimeout(url, options = {}, timeout = 5000) {
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
                this.showToast('Command sent successfully', 'success');
                return result;
            } else {
                const errorData = await response.json().catch(() => ({}));
                throw new Error(errorData.error || `HTTP ${response.status}`);
            }
        } catch (error) {
            console.error('Error sending motor command:', error);
            this.showToast(`Command failed: ${error.message}`, 'error');
            throw error;
        }
    }
    
    updateDashboard(data) {
        // Update motor speeds
        if (data.motor1) {
            document.getElementById('motor1Speed').innerHTML = `${data.motor1.speed || 0} <span class="unit">RPM</span>`;
            document.getElementById('motor1Current').innerHTML = `${(data.motor1.current || 0).toFixed(1)} <span class="unit">A</span>`;
            this.updateMotorIndicator('motor1Indicator', data.motor1.speed > 0);
        }
        
        if (data.motor2) {
            document.getElementById('motor2Speed').innerHTML = `${data.motor2.speed || 0} <span class="unit">RPM</span>`;
            document.getElementById('motor2Current').innerHTML = `${(data.motor2.current || 0).toFixed(1)} <span class="unit">A</span>`;
            this.updateMotorIndicator('motor2Indicator', data.motor2.speed > 0);
        }
        
        // Update PWM control
        if (data.pwm !== undefined) {
            const pwmSlider = document.getElementById('pwmSlider');
            const pwmValue = document.getElementById('pwmValue');
            pwmSlider.value = data.pwm;
            pwmValue.textContent = data.pwm;
            this.updateSliderTrack(data.pwm);
        }
        
        // Update system info
        if (data.voltage !== undefined) {
            document.getElementById('systemVoltage').innerHTML = `${data.voltage.toFixed(1)} <span class="unit">V</span>`;
        }
        
        if (data.wifi_rssi !== undefined) {
            document.getElementById('wifiSignal').innerHTML = `${data.wifi_rssi} <span class="unit">dBm</span>`;
        }
        
        if (data.uptime !== undefined) {
            document.getElementById('systemUptime').textContent = this.formatUptime(data.uptime);
        }
        
        if (data.heap !== undefined) {
            document.getElementById('freeHeap').innerHTML = `${Math.round(data.heap / 1024)} <span class="unit">KB</span>`;
        }
        
        // Update control states
        if (data.doors_locked !== undefined) {
            this.updateDoorButton(data.doors_locked);
        }
        
        if (data.lights_on !== undefined) {
            this.updateLightsButton(data.lights_on);
        }
    }
    
    updateMotorIndicator(elementId, isActive) {
        const indicator = document.getElementById(elementId);
        if (isActive) {
            indicator.classList.add('active');
        } else {
            indicator.classList.remove('active');
        }
    }
    
    updateSliderTrack(value) {
        const percentage = (value / 255) * 100;
        const track = document.getElementById('sliderTrack');
        if (track) {
            track.style.width = `${percentage}%`;
        }
    }
    
    updateDoorButton(locked) {
        const btn = document.getElementById('doorBtn');
        const icon = btn.querySelector('.btn-icon');
        const text = btn.querySelector('.btn-text');
        
        if (locked) {
            btn.classList.add('active');
            icon.textContent = '🔒';
            text.textContent = 'LOCKED';
        } else {
            btn.classList.remove('active');
            icon.textContent = '🔓';
            text.textContent = 'UNLOCKED';
        }
    }
    
    updateLightsButton(on) {
        const btn = document.getElementById('lightsBtn');
        const icon = btn.querySelector('.btn-icon');
        const text = btn.querySelector('.btn-text');
        
        if (on) {
            btn.classList.add('active');
            icon.textContent = '💡';
            text.textContent = 'LIGHTS ON';
        } else {
            btn.classList.remove('active');
            icon.textContent = '🔆';
            text.textContent = 'LIGHTS OFF';
        }
    }
    
    async setPWM(value) {
        try {
            await this.sendMotorCommand({ pwm: parseInt(value) });
        } catch (error) {
            console.error('Failed to set PWM:', error);
        }
    }
    
    async toggleDoors() {
        try {
            // Get current door status first
            const status = await this.getMotorStatus();
            const currentLocked = status.doors_locked || false;
            
            await this.sendMotorCommand({
                doors: {
                    lock_all: !currentLocked
                }
            });
        } catch (error) {
            console.error('Failed to toggle doors:', error);
        }
    }
    
    async toggleLights() {
        try {
            // Get current light status first
            const status = await this.getMotorStatus();
            const currentLightsOn = status.lights_on || false;
            
            await this.sendMotorCommand({
                lights: !currentLightsOn
            });
        } catch (error) {
            console.error('Failed to toggle lights:', error);
        }
    }
    
    async emergencyStop() {
        try {
            await this.sendMotorCommand({ pwm: 0 });
            this.showToast('EMERGENCY STOP ACTIVATED', 'error');
            // Reset UI to safe state
            document.getElementById('pwmSlider').value = 0;
            document.getElementById('pwmValue').textContent = '0';
            this.updateSliderTrack(0);
        } catch (error) {
            console.error('Failed to emergency stop:', error);
        }
    }
    
    startDataPolling() {
        // Poll for data every 1 second
        this.updateInterval = setInterval(async () => {
            try {
                await this.getMotorStatus();
            } catch (error) {
                // Error handling is done in getMotorStatus
            }
        }, 1000);
    }
    
    startConnectionMonitoring() {
        // Check connection status every 5 seconds
        this.connectionCheckInterval = setInterval(() => {
            if (Date.now() - this.lastDataUpdate > 10000) {
                this.updateConnectionStatus(false, 'No data received');
            }
        }, 5000);
    }
    
    updateConnectionStatus(connected, statusText) {
        const statusIndicator = document.getElementById('statusIndicator');
        const statusTextElement = document.getElementById('statusText');
        
        if (connected) {
            statusIndicator.classList.add('connected');
            statusIndicator.classList.remove('disconnected', 'error');
        } else {
            statusIndicator.classList.remove('connected');
            statusIndicator.classList.add('disconnected');
        }
        
        statusTextElement.textContent = statusText;
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
        document.getElementById('lastUpdate').textContent = `Last update: ${timeStr}`;
    }
    
    showToast(message, type = 'info') {
        const container = document.getElementById('toastContainer');
        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.textContent = message;
        
        container.appendChild(toast);
        
        // Trigger animation
        setTimeout(() => toast.classList.add('show'), 10);
        
        // Remove after 5 seconds
        setTimeout(() => {
            toast.classList.remove('show');
            setTimeout(() => container.removeChild(toast), 300);
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
    }
}

// Global API instance
let motorAPI;

// Global functions for HTML event handlers
function updatePWM(value) {
    document.getElementById('pwmValue').textContent = value;
    motorAPI.updateSliderTrack(value);
}

function setPWM(value) {
    motorAPI.setPWM(value);
}

function setPresetPWM(value) {
    document.getElementById('pwmSlider').value = value;
    updatePWM(value);
    setPWM(value);
}

function toggleDoors() {
    motorAPI.toggleDoors();
}

function toggleLights() {
    motorAPI.toggleLights();
}

function emergencyStop() {
    if (confirm('Are you sure you want to activate emergency stop?')) {
        motorAPI.emergencyStop();
    }
}

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    console.log('Motor Control Dashboard initializing...');
    motorAPI = new MotorControlAPI();
});

// Handle window events
window.addEventListener('beforeunload', function() {
    if (motorAPI) {
        motorAPI.destroy();
    }
});

window.addEventListener('online', function() {
    motorAPI.showToast('Network connection restored', 'success');
});

window.addEventListener('offline', function() {
    motorAPI.showToast('Network connection lost', 'warning');
});