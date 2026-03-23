// Motor Control Dashboard JavaScript - HTTP API Version (ESP32 Optimized)
class MotorControlAPI {
  constructor() {
    this.baseUrl = window.location.origin
    this.isConnected = false
    this.lastDataUpdate = 0
    this.updateInterval = null
    this.connectionCheckInterval = null

    // State management for additional features
    this.motorDirection = "forward" // 'forward' or 'reverse'
    this.motorSpeed = "fast" // 'fast' or 'slow'
    this.logsStatus = { active: false, count: 0 }

    // Cache DOM elements for better performance
    this.domCache = {}
    this.cacheDOMElements()

    this.init()

    // Initialize pin status button
    setTimeout(() => {
      this.updatePinStatusButton()
      // Initialize WiFi strength indicator with offline state
      this.updateWiFiStrength(-85) // Start with poor signal
    }, 100)
  }

  cacheDOMElements() {
    // Cache frequently accessed DOM elements
    this.domCache = {
      statusIndicator: document.getElementById("statusIndicator"),
      emergencyIndicator: document.getElementById("emergencyIndicator"),
      pwmSlider: document.getElementById("pwmSlider"),
      pwmValue: document.getElementById("pwmValue"),
      lastUpdate: document.getElementById("lastUpdate"),
      systemUptime: document.getElementById("systemUptime"),
      freeHeap: document.getElementById("freeHeap"),
      motor1Speed: document.getElementById("motor1Speed"),
      motor1Current: document.getElementById("motor1Current"),
      motor1Voltage: document.getElementById("motor1Voltage"),
      motor2Speed: document.getElementById("motor2Speed"),
      motor2Current: document.getElementById("motor2Current"),
      motor2Voltage: document.getElementById("motor2Voltage"),
      motor3Speed: document.getElementById("motor3Speed"),
      motor3Current: document.getElementById("motor3Current"),
      motor3Voltage: document.getElementById("motor3Voltage"),
      motor4Speed: document.getElementById("motor4Speed"),
      motor4Current: document.getElementById("motor4Current"),
      motor4Voltage: document.getElementById("motor4Voltage"),
      leftDoorBtn: document.getElementById("leftDoorBtn"),
      rightDoorBtn: document.getElementById("rightDoorBtn"),
      leftLightBtn: document.getElementById("leftLightBtn"),
      rightLightBtn: document.getElementById("rightLightBtn"),
      // WiFi dropdown elements
      wifiStatusBtn: document.getElementById("wifiStatusBtn"),
      wifiDropdownContent: document.getElementById("wifiDropdownContent"),
      wifiModeText: document.getElementById("wifiModeText"),
      currentModeValue: document.getElementById("currentModeValue"),
      connectionInfo: document.getElementById("connectionInfo"),
      ipAddress: document.getElementById("ipAddress"),
      switchToAP: document.getElementById("switchToAP"),
      switchToStation: document.getElementById("switchToStation"),
      // Pin status button elements
      pinStatusButton: document.getElementById("pinStatusButton"),
      // Feature elements
      directionToggle: document.getElementById("directionToggle"),
      directionText: document.getElementById("directionText"),
      speedToggle: document.getElementById("speedToggle"),
      speedText: document.getElementById("speedText"),
      downloadLogsBtn: document.getElementById("downloadLogsBtn"),
      wifiStrengthIndicator: document.getElementById("wifiStrengthIndicator"),

      dropdownStatusIndicator: document.getElementById("dropdownStatusIndicator"),
    }

    // Pin assignment command status tracking
    this.pinCommandStatus = "idle" // 'idle', 'success', 'error'
    this.pinStatusTimeout = null
    this.wifiSignalStrength = 0
  }

  init() {
    this.setupEventListeners()
    this.updateConnectionStatus(false, "Connecting...")
    this.startDataPolling()
    this.startConnectionMonitoring()

    // Initialize WiFi mode display
    setTimeout(() => {
      checkWiFiMode()
    }, 1000)

    // Initialize new features
    this.initializeNewFeatures()
  }

  async fetchWithTimeout(url, options = {}, timeout = 3000) {
    // Reduced timeout for ESP32
    const controller = new AbortController()
    const timeoutId = setTimeout(() => controller.abort(), timeout)

    try {
      const response = await fetch(url, {
        ...options,
        signal: controller.signal,
      })
      clearTimeout(timeoutId)
      return response
    } catch (error) {
      clearTimeout(timeoutId)
      throw error
    }
  }

  async getMotorStatus() {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/status`)
      if (response.ok) {
        const data = await response.json()
        this.updateDashboard(data)
        this.lastDataUpdate = Date.now()
        this.updateLastUpdateTime()
        this.updateConnectionStatus(true, "Connected")
        return data
      } else {
        throw new Error(`HTTP ${response.status}`)
      }
    } catch (error) {
      console.error("Error fetching motor status:", error)
      this.updateConnectionStatus(false, "Connection Error")
      throw error
    }
  }

  async sendMotorCommand(command) {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/control`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(command),
      })

      if (response.ok) {
        const result = await response.json()
        this.updatePinCommandStatus("success")
        return result
      } else {
        const errorData = await response.json().catch(() => ({}))
        throw new Error(errorData.error || `HTTP ${response.status}`)
      }
    } catch (error) {
      console.error("Error sending motor command:", error)
      this.updatePinCommandStatus("error")
      throw error
    }
  }

  updateDashboard(data) {
    // Update motor speeds - optimized for 4 motors
    const motors = ["motor1", "motor2", "motor3", "motor4"]
    motors.forEach((motor, index) => {
      if (data[motor]) {
        const speedEl = this.domCache[`${motor}Speed`]
        const currentEl = this.domCache[`${motor}Current`]
        const voltageEl = this.domCache[`${motor}Voltage`]
        const indicatorEl = document.getElementById(`${motor}Indicator`)

        if (speedEl) {
          speedEl.textContent = `${data[motor].speed || 0}`
        }
        if (currentEl) {
          currentEl.textContent = `${(data[motor].current || 0).toFixed(1)}A`
        }
        if (voltageEl) {
          voltageEl.textContent = `${(data.voltage || 0).toFixed(1)}V`
        }
        if (indicatorEl) {
          this.updateMotorIndicator(indicatorEl, data[motor].speed > 0)
        }
      }
    })

    // Update PWM control with improved synchronization
    if (data.pwm !== undefined && this.domCache.pwmSlider && this.domCache.pwmValue) {
      // Only update if value actually changed to avoid input interference
      if (Number.parseInt(this.domCache.pwmSlider.value) !== data.pwm) {
        this.domCache.pwmSlider.value = data.pwm
        this.domCache.pwmValue.textContent = data.pwm
        console.log("PWM slider synchronized to:", data.pwm)
      }
    }

    // Update footer system info
    if (data.uptime !== undefined && this.domCache.systemUptime) {
      this.domCache.systemUptime.textContent = this.formatUptime(data.uptime)
    }

    if (data.heap !== undefined && this.domCache.freeHeap) {
      this.domCache.freeHeap.textContent = `${Math.round(data.heap / 1024)} KB`
    }

    // Update WiFi signal strength for pin status button
    if (data.wifi_rssi !== undefined) {
      this.wifiSignalStrength = data.wifi_rssi
      this.updatePinStatusButton()
      this.updateWiFiStrength(data.wifi_rssi)
    }

    // Update control states with enhanced synchronization
    if (data.doors !== undefined) {
      this.updateLeftDoorButton(data.doors.left_locked || false)
      this.updateRightDoorButton(data.doors.right_locked || false)

      // Log door state changes for debugging
      console.log("Door states updated:", {
        left: data.doors.left_locked || false,
        right: data.doors.right_locked || false,
      })
    }

    if (data.lights !== undefined) {
      this.updateLeftLightButton(data.lights.left_on || false)
      this.updateRightLightButton(data.lights.right_on || false)

      // Log light state changes for debugging
      console.log("Light states updated:", {
        left: data.lights.left_on || false,
        right: data.lights.right_on || false,
      })
    }

    // Update new feature states if included in response
    if (data.direction !== undefined && data.direction !== this.motorDirection) {
      this.motorDirection = data.direction
      this.updateDirectionDisplay()
    }

    if (data.speed !== undefined && data.speed !== this.motorSpeed) {
      this.motorSpeed = data.speed
      this.updateSpeedDisplay()
    }
  }

  updateMotorIndicator(motorStatusElement, isActive) {
    const statusIndicator = motorStatusElement.querySelector(".status-dot")
    if (statusIndicator) {
      if (isActive) {
        statusIndicator.classList.remove("offline")
        statusIndicator.classList.add("online")
      } else {
        statusIndicator.classList.remove("online")
        statusIndicator.classList.add("offline")
      }
    }
  }

  updateLeftDoorButton(locked) {
    const btn = this.domCache.leftDoorBtn
    if (!btn) {
      console.warn("Left door button not found in DOM cache")
      return
    }

    const icon = btn.querySelector(".hardware-icon")
    if (!icon) {
      console.warn("Left door button icon not found")
      return
    }

    // Only update if state actually changed to avoid unnecessary DOM manipulation
    const isCurrentlyActive = btn.classList.contains("active")
    if (isCurrentlyActive === locked) return

    console.log("Updating left door button state:", locked ? "LOCKED" : "UNLOCKED")

    if (locked) {
      btn.classList.add("active")
      // Lock icon - closed lock
      icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 10 0v4"/>'
    } else {
      btn.classList.remove("active")
      // Unlock icon - open lock
      icon.innerHTML =
        '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 8.5 0v2.5"/>'
    }

    // Add visual feedback animation
    btn.style.transform = "scale(0.95)"
    setTimeout(() => {
      btn.style.transform = "scale(1)"
    }, 100)
  }

  updateRightDoorButton(locked) {
    const btn = this.domCache.rightDoorBtn
    if (!btn) {
      console.warn("Right door button not found in DOM cache")
      return
    }

    const icon = btn.querySelector(".hardware-icon")
    if (!icon) {
      console.warn("Right door button icon not found")
      return
    }

    // Only update if state actually changed to avoid unnecessary DOM manipulation
    const isCurrentlyActive = btn.classList.contains("active")
    if (isCurrentlyActive === locked) return

    console.log("Updating right door button state:", locked ? "LOCKED" : "UNLOCKED")

    if (locked) {
      btn.classList.add("active")
      // Lock icon - closed lock
      icon.innerHTML = '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 10 0v4"/>'
    } else {
      btn.classList.remove("active")
      // Unlock icon - open lock
      icon.innerHTML =
        '<rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="m7 11V7a5 5 0 0 1 8.5 0v2.5"/>'
    }

    // Add visual feedback animation
    btn.style.transform = "scale(0.95)"
    setTimeout(() => {
      btn.style.transform = "scale(1)"
    }, 100)
  }

  updateLeftLightButton(on) {
    const btn = this.domCache.leftLightBtn
    if (!btn) {
      console.warn("Left light button not found in DOM cache")
      return
    }

    const icon = btn.querySelector(".hardware-icon")
    if (!icon) {
      console.warn("Left light button icon not found")
      return
    }

    // Only update if state actually changed to avoid unnecessary DOM manipulation
    const isCurrentlyActive = btn.classList.contains("active")
    if (isCurrentlyActive === on) return

    console.log("Updating left light button state:", on ? "ON" : "OFF")

    if (on) {
      btn.classList.add("active")
      // Light On icon - sun
      icon.innerHTML =
        '<circle cx="12" cy="12" r="4"/><path d="M12 2v2"/><path d="M12 20v2"/><path d="m4.93 4.93 1.41 1.41"/><path d="m17.66 17.66 1.41 1.41"/><path d="M2 12h2"/><path d="M20 12h2"/><path d="m6.34 17.66-1.41 1.41"/><path d="m19.07 4.93-1.41 1.41"/>'
    } else {
      btn.classList.remove("active")
      // Light Off icon - moon
      icon.innerHTML = '<path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z"/>'
    }

    // Add visual feedback animation
    btn.style.transform = "scale(0.95)"
    setTimeout(() => {
      btn.style.transform = "scale(1)"
    }, 100)
  }

  updateRightLightButton(on) {
    const btn = this.domCache.rightLightBtn
    if (!btn) {
      console.warn("Right light button not found in DOM cache")
      return
    }

    const icon = btn.querySelector(".hardware-icon")
    if (!icon) {
      console.warn("Right light button icon not found")
      return
    }

    // Only update if state actually changed to avoid unnecessary DOM manipulation
    const isCurrentlyActive = btn.classList.contains("active")
    if (isCurrentlyActive === on) return

    console.log("Updating right light button state:", on ? "ON" : "OFF")

    if (on) {
      btn.classList.add("active")
      // Light On icon - sun
      icon.innerHTML =
        '<circle cx="12" cy="12" r="4"/><path d="M12 2v2"/><path d="M12 20v2"/><path d="m4.93 4.93 1.41 1.41"/><path d="m17.66 17.66 1.41 1.41"/><path d="M2 12h2"/><path d="M20 12h2"/><path d="m6.34 17.66-1.41 1.41"/><path d="m19.07 4.93-1.41 1.41"/>'
    } else {
      btn.classList.remove("active")
      // Light Off icon - moon
      icon.innerHTML = '<path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z"/>'
    }

    // Add visual feedback animation
    btn.style.transform = "scale(0.95)"
    setTimeout(() => {
      btn.style.transform = "scale(1)"
    }, 100)
  }

  async setPWM(value) {
    try {
      await this.sendMotorCommand({ pwm: Number.parseInt(value) })
    } catch (error) {
      console.error("Failed to set PWM:", error)
    }
  }

  async toggleLeftDoor() {
    try {
      // Get current state and toggle immediately
      const btn = this.domCache.leftDoorBtn
      const isCurrentlyLocked = btn.classList.contains("active")
      const newState = !isCurrentlyLocked

      // Update UI immediately
      this.updateLeftDoorButton(newState)

      // Send command to server
      await this.sendMotorCommand({
        doors: {
          left: "toggle",
        },
      })
    } catch (error) {
      console.error("Failed to toggle left door:", error)
      // Revert on error
      const btn = this.domCache.leftDoorBtn
      const isCurrentlyLocked = btn.classList.contains("active")
      this.updateLeftDoorButton(!isCurrentlyLocked)
    }
  }

  async toggleRightDoor() {
    try {
      // Get current state and toggle immediately
      const btn = this.domCache.rightDoorBtn
      const isCurrentlyLocked = btn.classList.contains("active")
      const newState = !isCurrentlyLocked

      // Update UI immediately
      this.updateRightDoorButton(newState)

      // Send command to server
      await this.sendMotorCommand({
        doors: {
          right: "toggle",
        },
      })
    } catch (error) {
      console.error("Failed to toggle right door:", error)
      // Revert on error
      const btn = this.domCache.rightDoorBtn
      const isCurrentlyLocked = btn.classList.contains("active")
      this.updateRightDoorButton(!isCurrentlyLocked)
    }
  }

  async toggleLeftLight() {
    try {
      // Get current state and toggle immediately
      const btn = this.domCache.leftLightBtn
      const isCurrentlyOn = btn.classList.contains("active")
      const newState = !isCurrentlyOn

      // Update UI immediately
      this.updateLeftLightButton(newState)

      // Send command to server
      await this.sendMotorCommand({
        lights: {
          left: "toggle",
        },
      })
    } catch (error) {
      console.error("Failed to toggle left light:", error)
      // Revert on error
      const btn = this.domCache.leftLightBtn
      const isCurrentlyOn = btn.classList.contains("active")
      this.updateLeftLightButton(!isCurrentlyOn)
    }
  }

  async toggleRightLight() {
    try {
      // Get current state and toggle immediately
      const btn = this.domCache.rightLightBtn
      const isCurrentlyOn = btn.classList.contains("active")
      const newState = !isCurrentlyOn

      // Update UI immediately
      this.updateRightLightButton(newState)

      // Send command to server
      await this.sendMotorCommand({
        lights: {
          right: "toggle",
        },
      })
    } catch (error) {
      console.error("Failed to toggle right light:", error)
      // Revert on error
      const btn = this.domCache.rightLightBtn
      const isCurrentlyOn = btn.classList.contains("active")
      this.updateRightLightButton(!isCurrentlyOn)
    }
  }

  async emergencyStop() {
    try {
      // Update emergency indicator immediately
      this.updateEmergencyIndicator(true)

      await this.sendMotorCommand({ pwm: 0 })

      // Reset UI to safe state
      if (this.domCache.pwmSlider) this.domCache.pwmSlider.value = 0
      if (this.domCache.pwmValue) this.domCache.pwmValue.textContent = "0"

      // Reset emergency indicator after 3 seconds
      setTimeout(() => {
        this.updateEmergencyIndicator(false)
      }, 3000)
    } catch (error) {
      console.error("Failed to emergency stop:", error)
      // Reset emergency indicator on error
      this.updateEmergencyIndicator(false)
    }
  }

  updateEmergencyIndicator(active) {
    const emergencyButton = document.querySelector(".emergency-button")
    if (emergencyButton) {
      if (active) {
        emergencyButton.classList.add("emergency-active")
      } else {
        emergencyButton.classList.remove("emergency-active")
      }
    }
  }

  startDataPolling() {
    // Poll for data every 1 second for better responsiveness
    this.updateInterval = setInterval(async () => {
      try {
        await this.getMotorStatus()

        // Poll additional endpoints every 5 seconds to reduce load
        if (Date.now() % 5000 < 1000) {
          await this.getLogsStatus()
        }
      } catch (error) {
        // Error handling is done in getMotorStatus
        console.error("Data polling error:", error)
      }
    }, 1000)
  }

  startConnectionMonitoring() {
    // Check connection status every 10 seconds (reduced frequency)
    this.connectionCheckInterval = setInterval(() => {
      if (Date.now() - this.lastDataUpdate > 15000) {
        // Increased timeout
        this.updateConnectionStatus(false, "No data received")
      }
    }, 10000)
  }

  updateConnectionStatus(connected, statusText) {
    // Update WiFi strength indicator based on connection status
    if (this.domCache.wifiStrengthIndicator) {
      this.domCache.wifiStrengthIndicator.classList.remove("excellent", "good", "fair", "poor", "offline")
      if (connected) {
        // Default to good signal when connected but no RSSI data
        this.domCache.wifiStrengthIndicator.classList.add("good")
      } else {
        this.domCache.wifiStrengthIndicator.classList.add("offline")
      }
    }

    // Update dropdown status indicator
    if (this.domCache.dropdownStatusIndicator) {
      this.domCache.dropdownStatusIndicator.classList.remove("connected", "error")
      if (connected) {
        this.domCache.dropdownStatusIndicator.classList.add("connected")
      } else {
        this.domCache.dropdownStatusIndicator.classList.add("error")
      }
    }

    // Update WiFi mode text with connection status if no mode text is available
    if (this.domCache.wifiModeText) {
      const currentText = this.domCache.wifiModeText.textContent
      if (!currentText || currentText === "Connecting" || currentText.includes("Error")) {
        this.domCache.wifiModeText.textContent = statusText
      }
    }

    this.isConnected = connected

    // Update pin status when connection changes
    if (!connected) {
      this.updatePinCommandStatus("error", 5000)
    }
  }

  formatUptime(milliseconds) {
    const seconds = Math.floor(milliseconds / 1000)
    const hours = Math.floor(seconds / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)
    const secs = seconds % 60

    return `${hours}:${minutes.toString().padStart(2, "0")}:${secs.toString().padStart(2, "0")}`
  }

  updateLastUpdateTime() {
    const now = new Date()
    const timeStr = now.toLocaleTimeString()
    if (this.domCache.lastUpdate) {
      this.domCache.lastUpdate.textContent = `Last update: ${timeStr}`
    }
  }

  updatePinCommandStatus(status, duration = 2000) {
    this.pinCommandStatus = status
    this.updatePinStatusButton()

    // Reset to idle after duration (longer for errors)
    if (status !== "idle") {
      if (this.pinStatusTimeout) {
        clearTimeout(this.pinStatusTimeout)
      }
      const resetDuration = status === "error" ? 4000 : duration
      this.pinStatusTimeout = setTimeout(() => {
        this.pinCommandStatus = "idle"
        this.updatePinStatusButton()
      }, resetDuration)
    }
  }

  updatePinStatusButton() {
    const button = this.domCache.pinStatusButton

    if (!button) return

    // Remove existing status classes
    button.classList.remove("success", "error", "wifi-signal")

    if (this.pinCommandStatus === "success") {
      button.classList.add("success")
      // Hide WiFi indicator and show success state
      const wifiIndicator = button.querySelector(".wifi-strength-indicator")
      if (wifiIndicator) {
        wifiIndicator.style.display = "none"
      }
    } else if (this.pinCommandStatus === "error") {
      button.classList.add("error")
      // Hide WiFi indicator and show error state
      const wifiIndicator = button.querySelector(".wifi-strength-indicator")
      if (wifiIndicator) {
        wifiIndicator.style.display = "none"
      }
    } else {
      // Show WiFi signal strength
      button.classList.add("wifi-signal")
      const wifiIndicator = button.querySelector(".wifi-strength-indicator")
      if (wifiIndicator) {
        wifiIndicator.style.display = "flex"
      }
    }
  }

  // showToast function removed - using navbar indicator instead

  // Update WiFi strength indicator based on RSSI value
  updateWiFiStrength(rssi) {
    const indicator = this.domCache.wifiStrengthIndicator
    if (!indicator) return

    // Remove existing strength classes
    indicator.classList.remove("excellent", "good", "fair", "poor", "offline")

    // Determine strength level based on RSSI
    let strengthClass = "offline"
    if (rssi >= -50) {
      strengthClass = "excellent"
    } else if (rssi >= -60) {
      strengthClass = "good"
    } else if (rssi >= -70) {
      strengthClass = "fair"
    } else if (rssi >= -80) {
      strengthClass = "poor"
    }

    indicator.classList.add(strengthClass)
  }

  // API methods for additional features
  async initializeNewFeatures() {
    try {
      // Get initial direction state
      await this.getDirection()
      // Get initial speed state
      await this.getSpeed()
      // Get logs status
      await this.getLogsStatus()
    } catch (error) {
      console.error("Failed to initialize new features:", error)
      // Set default values
      this.updateDirectionDisplay()
      this.updateSpeedDisplay()
    }
  }

  async getDirection() {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/direction`)
      if (response.ok) {
        const data = await response.json()
        this.motorDirection = data.direction || "forward"
        this.updateDirectionDisplay()
        return data
      }
    } catch (error) {
      console.error("Error fetching direction:", error)
    }
  }

  async setDirection(direction) {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/direction`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ direction: direction }),
      })

      if (response.ok) {
        const result = await response.json()
        this.motorDirection = direction
        this.updateDirectionDisplay()
        console.log("Direction updated:", direction)
        return result
      }
    } catch (error) {
      console.error("Failed to set direction:", error)
    }
  }

  async getSpeed() {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/speed`)
      if (response.ok) {
        const data = await response.json()
        this.motorSpeed = data.speed || "fast"
        this.updateSpeedDisplay()
        return data
      }
    } catch (error) {
      console.error("Error fetching speed:", error)
    }
  }

  async setSpeed(speed) {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/motor/speed`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ speed: speed }),
      })

      if (response.ok) {
        const result = await response.json()
        this.motorSpeed = speed
        this.updateSpeedDisplay()
        console.log("Speed updated:", speed)
        return result
      }
    } catch (error) {
      console.error("Failed to set speed:", error)
    }
  }

  async getLogsStatus() {
    try {
      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/logs/status`)
      if (response.ok) {
        const data = await response.json()
        this.logsStatus = data
        return data
      }
    } catch (error) {
      console.error("Error fetching logs status:", error)
      // Set default status on error
      this.logsStatus = { active: false, count: 0 }
    }
  }

  async downloadLogs() {
    try {
      this.updateLogsButtonState("downloading")

      const response = await this.fetchWithTimeout(`${this.baseUrl}/api/logs/download`, {
        method: "GET",
      })

      if (response.ok) {
        const blob = await response.blob()
        const url = window.URL.createObjectURL(blob)
        const a = document.createElement("a")
        a.href = url

        // Generate filename with current date
        const now = new Date()
        const dateStr = now.toISOString().split("T")[0] // YYYY-MM-DD format
        a.download = `motor-logs-${dateStr}.json`

        document.body.appendChild(a)
        a.click()
        window.URL.revokeObjectURL(url)
        document.body.removeChild(a)

        // Update navbar indicator for success instead of showing toast
        this.updatePinCommandStatus("success", 3000)
        this.updateLogsButtonState("success")

        // Reset button state after 2 seconds
        setTimeout(() => {
          this.updateLogsButtonState("idle")
        }, 2000)
      } else {
        throw new Error(`HTTP ${response.status}`)
      }
    } catch (error) {
      console.error("Failed to download logs:", error)
      // Update navbar indicator instead of showing toast
      this.updatePinCommandStatus("error", 5000)
      this.updateLogsButtonState("error")

      // Reset button state after 3 seconds
      setTimeout(() => {
        this.updateLogsButtonState("idle")
      }, 3000)
    }
  }

  // UI update methods for features

  updateDirectionDisplay() {
    const btn = this.domCache.directionToggle
    const text = this.domCache.directionText

    if (btn && text) {
      // Remove existing classes
      btn.classList.remove("reverse", "active")

      // Get the icon element
      const icon = btn.querySelector(".toggle-icon")

      if (this.motorDirection === "reverse") {
        btn.classList.add("reverse")
        text.textContent = "Reverse"
        // Reverse arrow - pointing left
        if (icon) {
          icon.innerHTML = '<polygon points="19,3 5,12 19,21"/>'
        }
      } else {
        text.textContent = "Forward"
        // Forward arrow - pointing right
        if (icon) {
          icon.innerHTML = '<polygon points="5,3 19,12 5,21"/>'
        }
      }
    }

    console.log("Direction display updated:", this.motorDirection)
  }

  updateSpeedDisplay() {
    const btn = this.domCache.speedToggle
    const text = this.domCache.speedText

    if (btn && text) {
      // Remove existing classes
      btn.classList.remove("slow", "active")

      if (this.motorSpeed === "slow") {
        btn.classList.add("slow")
        text.textContent = "Slow"
      } else {
        text.textContent = "Fast"
      }
    }

    console.log("Speed display updated:", this.motorSpeed)
  }

  updateLogsButtonState(state) {
    const btn = this.domCache.downloadLogsBtn

    if (!btn) return

    // Remove existing state classes
    btn.classList.remove("downloading", "success", "error")

    switch (state) {
      case "downloading":
        btn.classList.add("downloading")
        btn.disabled = true
        break
      case "success":
        btn.classList.add("success")
        btn.disabled = false
        break
      case "error":
        btn.classList.add("error")
        btn.disabled = false
        break
      default: // 'idle'
        btn.disabled = false
        break
    }
  }

  setupEventListeners() {
    // Connection monitoring is handled by startConnectionMonitoring
  }

  destroy() {
    if (this.updateInterval) {
      clearInterval(this.updateInterval)
    }
    if (this.connectionCheckInterval) {
      clearInterval(this.connectionCheckInterval)
    }
    // Clear DOM cache
    this.domCache = {}
  }
}

// Global API instance
let motorAPI

// Global functions for HTML event handlers - optimized
function updatePWM(value) {
  if (motorAPI && motorAPI.domCache.pwmValue) {
    motorAPI.domCache.pwmValue.textContent = value
  }
}

function setPWM(value) {
  if (motorAPI) {
    motorAPI.setPWM(value)
  }
}

function setPresetPWM(value) {
  if (motorAPI && motorAPI.domCache.pwmSlider) {
    motorAPI.domCache.pwmSlider.value = value
    updatePWM(value)
    setPWM(value)
  }
}

function toggleLeftDoor() {
  if (motorAPI) motorAPI.toggleLeftDoor()
}

function toggleRightDoor() {
  if (motorAPI) motorAPI.toggleRightDoor()
}

function toggleLeftLight() {
  if (motorAPI) motorAPI.toggleLeftLight()
}

function toggleRightLight() {
  if (motorAPI) motorAPI.toggleRightLight()
}

function emergencyStop() {
  if (confirm("Are you sure you want to activate emergency stop?")) {
    if (motorAPI) motorAPI.emergencyStop()
  }
}

// Global functions for additional features

function toggleDirection() {
  if (motorAPI) {
    const newDirection = motorAPI.motorDirection === "forward" ? "reverse" : "forward"
    motorAPI.setDirection(newDirection)
  }
}

function toggleSpeed() {
  if (motorAPI) {
    const newSpeed = motorAPI.motorSpeed === "fast" ? "slow" : "fast"
    motorAPI.setSpeed(newSpeed)
  }
}

function downloadLogs() {
  if (motorAPI) {
    motorAPI.downloadLogs()
  }
}

// Test function for debugging icon toggles
function testIconToggles() {
  if (!motorAPI) return

  console.log("Testing icon toggles...")

  // Test door toggles
  motorAPI.updateLeftDoorButton(true)
  motorAPI.updateRightDoorButton(true)
  setTimeout(() => {
    motorAPI.updateLeftDoorButton(false)
    motorAPI.updateRightDoorButton(false)
  }, 2000)

  // Test light toggles
  motorAPI.updateLeftLightButton(true)
  motorAPI.updateRightLightButton(true)
  setTimeout(() => {
    motorAPI.updateLeftLightButton(false)
    motorAPI.updateRightLightButton(false)
  }, 4000)
}

// WiFi Dropdown Management
let isWiFiDropdownOpen = false

function toggleWiFiDropdown() {
  const dropdown = document.getElementById("wifiDropdown")
  const btn = document.getElementById("wifiStatusBtn")
  const content = document.getElementById("wifiDropdownContent")

  if (!dropdown || !btn || !content) {
    console.error("WiFi dropdown elements not found")
    return
  }

  isWiFiDropdownOpen = !isWiFiDropdownOpen

  if (isWiFiDropdownOpen) {
    btn.classList.add("active")
    content.classList.add("show")
    // Update WiFi info when opening
    checkWiFiMode()
  } else {
    btn.classList.remove("active")
    content.classList.remove("show")
  }
}

// Close dropdown when clicking outside (supports both mouse and touch)
document.addEventListener("click", (event) => {
  const dropdown = document.getElementById("wifiDropdown")
  if (dropdown && !dropdown.contains(event.target) && isWiFiDropdownOpen) {
    toggleWiFiDropdown()
  }
})

// WiFi Mode Management Functions
async function checkWiFiMode() {
  try {
    const response = await fetch("/api/wifi/mode")
    if (response.ok) {
      const wifiInfo = await response.json()
      updateWiFiModeDisplay(wifiInfo)
    } else {
      throw new Error(`HTTP ${response.status}`)
    }
  } catch (error) {
    console.error("Failed to check WiFi mode:", error)
    updateWiFiModeDisplay({
      currentMode: "Unknown",
      modeDescription: "Connection Error",
      currentIP: "-",
      canSwitch: false,
      connectionStatus: "Disconnected",
    })
  }
}

async function switchToAPMode() {
  await switchWiFiMode("AP")
}

async function switchToStationMode() {
  await switchWiFiMode("Station")
}

async function switchWiFiMode(targetMode) {
  if (!confirm(`Switch to ${targetMode} mode? System will restart and you may need to reconnect.`)) {
    return
  }

  try {
    // Close dropdown first
    if (isWiFiDropdownOpen) {
      toggleWiFiDropdown()
    }

    const switchResponse = await fetch("/api/wifi/switch", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ mode: targetMode }),
    })

    if (switchResponse.ok) {
      // Update navbar indicator instead of showing toast
      if (motorAPI) motorAPI.updatePinCommandStatus("success", 3000)

      // Disable UI during restart
      document.body.style.pointerEvents = "none"
      document.body.style.opacity = "0.7"

      // Show loading indicator in navbar
      const wifiModeText = document.getElementById("wifiModeText")
      if (wifiModeText) {
        wifiModeText.textContent = "Restarting..."
      }

      // Update status indicator to show transition
      const statusIndicator = document.getElementById("statusIndicator")
      if (statusIndicator) {
        statusIndicator.classList.remove("connected", "disconnected")
        statusIndicator.classList.add("error") // Use error class for transition state
      }
    } else {
      throw new Error(`HTTP ${switchResponse.status}`)
    }
  } catch (error) {
    console.error("Failed to switch WiFi mode:", error)
    // Update navbar indicator instead of showing toast
    if (motorAPI) motorAPI.updatePinCommandStatus("error", 5000)
  }
}

function updateWiFiModeDisplay(wifiInfo) {
  // Update button text - show actual connection status
  const wifiModeText = document.getElementById("wifiModeText")
  if (wifiModeText) {
    // Show connection status instead of mode text
    const connectionStatus = wifiInfo.connectionStatus || (wifiInfo.canSwitch ? "Connected" : "Disconnected")
    wifiModeText.textContent = connectionStatus
  }

  // Update status indicator color based on connection status
  const statusIndicator = document.getElementById("statusIndicator")
  if (statusIndicator && motorAPI) {
    const isConnected = wifiInfo.canSwitch || wifiInfo.connectionStatus === "Connected"
    if (isConnected) {
      statusIndicator.classList.add("connected")
      statusIndicator.classList.remove("disconnected", "error")
    } else {
      statusIndicator.classList.remove("connected")
      statusIndicator.classList.add("disconnected")
    }
  }

  // Update dropdown content
  const currentModeValue = document.getElementById("currentModeValue")
  if (currentModeValue) {
    currentModeValue.textContent = wifiInfo.modeDescription || wifiInfo.currentMode || "Unknown"
  }

  const connectionInfo = document.getElementById("connectionInfo")
  if (connectionInfo) {
    const connectionStatus = wifiInfo.canSwitch ? "Connected" : "Disconnected"
    connectionInfo.textContent = wifiInfo.connectionStatus || connectionStatus
  }

  const ipAddress = document.getElementById("ipAddress")
  if (ipAddress) {
    ipAddress.textContent = wifiInfo.currentIP || wifiInfo.ipAddress || "-"

    // Update curl commands if IP changed (and modal is open)
    const modal = document.getElementById("pinModalOverlay")
    if (modal && modal.classList.contains("show")) {
      updateCurlCommands() // This will work since updateCurlCommands is now async
    }
  }

  // Update button states
  const switchToAP = document.getElementById("switchToAP")
  const switchToStation = document.getElementById("switchToStation")

  if (switchToAP && switchToStation) {
    // Disable the button for current mode
    if (wifiInfo.currentMode === "AP") {
      switchToAP.disabled = true
      switchToStation.disabled = false
    } else {
      switchToAP.disabled = false
      switchToStation.disabled = true
    }
  }

  console.log("WiFi mode display updated:", wifiInfo)
}

// Pin Assignment Modal Functions
async function showPinAssignment() {
  const modal = document.getElementById("pinModalOverlay")
  if (modal) {
    modal.classList.add("show")
    document.body.style.overflow = "hidden" // Prevent background scrolling

    // Update curl commands with current IP - fetch fresh WiFi info first
    await updateCurlCommands()

    // Show success status when pin assignment modal is opened
    if (motorAPI) {
      motorAPI.updatePinCommandStatus("success", 3000)
    }
  }
}

function hidePinAssignment() {
  const modal = document.getElementById("pinModalOverlay")
  if (modal) {
    modal.classList.remove("show")
    document.body.style.overflow = "" // Restore scrolling
  }
}

// Curl Commands Management
async function updateCurlCommands() {
  let esp32IP = "192.168.4.1" // Default fallback to AP mode IP

  try {
    // Fetch current WiFi info to get the real IP address
    const response = await fetch("/api/wifi/mode", {
      method: "GET",
      headers: { Accept: "application/json" },
      timeout: 2000,
    })

    if (response.ok) {
      const wifiInfo = await response.json()
      // Use currentIP from API response, fallback to ipAddress field
      const apiIP = wifiInfo.currentIP || wifiInfo.ipAddress

      if (apiIP && apiIP !== "-" && apiIP !== "0.0.0.0" && apiIP !== "") {
        esp32IP = apiIP
        console.log("Using API IP address:", esp32IP)
      } else {
        // Fallback to window location hostname if API doesn't provide valid IP
        const hostIP = window.location.hostname
        if (hostIP && hostIP !== "localhost" && hostIP !== "127.0.0.1") {
          esp32IP = hostIP
          console.log("Using window hostname:", esp32IP)
        } else {
          // Determine fallback based on current mode
          if (wifiInfo.currentMode === "Station") {
            esp32IP = "192.168.1.100" // Common station IP
          }
          console.log("Using fallback IP for", wifiInfo.currentMode || "unknown", "mode:", esp32IP)
        }
      }
    } else {
      throw new Error(`WiFi API responded with ${response.status}`)
    }
  } catch (error) {
    console.error("Failed to fetch WiFi info for curl commands:", error)

    // Try to get IP from current page URL as secondary fallback
    const hostIP = window.location.hostname
    if (hostIP && hostIP !== "localhost" && hostIP !== "127.0.0.1") {
      esp32IP = hostIP
      console.log("Using window hostname as fallback:", esp32IP)
    } else {
      // Use default AP mode IP as final fallback
      console.log("Using default AP mode IP as final fallback:", esp32IP)
    }
  }

  // Update all curl command elements with the resolved IP
  const commands = {
    "curl-index": `curl -X POST -F "file=@data/index.html" http://${esp32IP}/upload`,
    "curl-script": `curl -X POST -F "file=@data/script.js" http://${esp32IP}/upload`,
    "curl-style": `curl -X POST -F "file=@data/style.css" http://${esp32IP}/upload`,
    "curl-batch": `curl -X POST http://${esp32IP}/upload \\
  -F "file=@index.html" \\
  -F "file=@script.js" \\
  -F "file=@style.css"`,
  }

  // Update each command element
  Object.keys(commands).forEach((id) => {
    const element = document.getElementById(id)
    if (element) {
      element.textContent = commands[id]
    }
  })

  console.log("Curl commands updated with resolved IP:", esp32IP)
}

function copyToClipboard(elementId) {
  const element = document.getElementById(elementId)
  if (!element) {
    console.error("Element not found:", elementId)
    return
  }

  const text = element.textContent

  // Use modern clipboard API if available
  if (navigator.clipboard && window.isSecureContext) {
    navigator.clipboard
      .writeText(text)
      .then(() => {
        showCopySuccess(elementId)
      })
      .catch((err) => {
        console.error("Failed to copy using clipboard API:", err)
        fallbackCopyToClipboard(text, elementId)
      })
  } else {
    fallbackCopyToClipboard(text, elementId)
  }
}

function fallbackCopyToClipboard(text, elementId) {
  // Fallback method for older browsers
  const textArea = document.createElement("textarea")
  textArea.value = text
  textArea.style.position = "fixed"
  textArea.style.left = "-999999px"
  textArea.style.top = "-999999px"
  document.body.appendChild(textArea)
  textArea.focus()
  textArea.select()

  try {
    const successful = document.execCommand("copy")
    if (successful) {
      showCopySuccess(elementId)
    } else {
      console.error("Fallback copy failed")
      if (motorAPI) {
        motorAPI.updatePinCommandStatus("error", 3000)
      }
    }
  } catch (err) {
    console.error("Fallback copy error:", err)
    if (motorAPI) {
      motorAPI.updatePinCommandStatus("error", 3000)
    }
  }

  document.body.removeChild(textArea)
}

function showCopySuccess(elementId) {
  // Find the copy button for this command
  const commandElement = document.getElementById(elementId)
  if (!commandElement) return

  const commandItem = commandElement.closest(".command-item")
  if (!commandItem) return

  const copyBtn = commandItem.querySelector(".copy-btn")
  if (!copyBtn) return

  // Add copied class for visual feedback
  copyBtn.classList.add("copied")

  // Update icon to checkmark
  const icon = copyBtn.querySelector(".copy-icon")
  if (icon) {
    const originalHTML = icon.innerHTML
    icon.innerHTML = '<path d="m9 12 2 2 4-4"/><circle cx="12" cy="12" r="10"/>'

    // Revert after 2 seconds
    setTimeout(() => {
      copyBtn.classList.remove("copied")
      icon.innerHTML = originalHTML
    }, 2000)
  }

  // Show toast notification
  if (motorAPI) {
    // Update navbar indicator instead of showing toast
    motorAPI.updatePinCommandStatus("success", 2000)
  }

  console.log("Copied to clipboard:", elementId)
}

// Handle escape key to close modal and dropdown
document.addEventListener("keydown", (event) => {
  if (event.key === "Escape") {
    hidePinAssignment()
    // Close WiFi dropdown if open
    if (isWiFiDropdownOpen) {
      toggleWiFiDropdown()
    }
  }
})

// Initialize when page loads
document.addEventListener("DOMContentLoaded", () => {
  console.log("Motor Control Dashboard initializing...")
  motorAPI = new MotorControlAPI()

  // Check WiFi mode on startup
  setTimeout(() => {
    checkWiFiMode()
  }, 2000)

  // Update WiFi mode periodically (every 15 seconds, but only when dropdown is closed)
  setInterval(() => {
    if (!isWiFiDropdownOpen) {
      checkWiFiMode()
    }
  }, 15000)
})

// Handle window events
window.addEventListener("beforeunload", () => {
  if (motorAPI) {
    motorAPI.destroy()
  }
})

window.addEventListener("online", () => {
  if (motorAPI) motorAPI.updatePinCommandStatus("success", 3000)
})

window.addEventListener("offline", () => {
  if (motorAPI) motorAPI.updatePinCommandStatus("error", 5000)
})
