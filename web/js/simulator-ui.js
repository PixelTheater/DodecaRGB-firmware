/**
 * DodecaRGB Web Simulator UI
 * Handles all the UI interaction, controls, and WASM module integration
 */
class DodecaSimulator {
    constructor() {
        // Store DOM elements
        this.canvas = document.getElementById('canvas');
        this.sceneButtons = document.getElementById('scene-buttons');
        this.brightnessSlider = document.getElementById('brightness');
        this.brightnessValue = document.getElementById('brightness-value');
        this.ledSizeSlider = document.getElementById('led-size');
        this.ledSizeValue = document.getElementById('led-size-value');
        this.glowSlider = document.getElementById('glow-intensity');
        this.glowValue = document.getElementById('glow-intensity-value');
        this.ledCountElement = document.getElementById('led-count');
        this.fpsElement = document.getElementById('fps');
        
        // Debug buttons
        this.benchmarkBtn = document.getElementById('benchmark-btn');
        this.debugBtn = document.getElementById('debug-btn');
        this.modelInfoBtn = document.getElementById('model-info-btn');
        
        // View controls
        this.viewSideBtn = document.getElementById('side-view');
        this.viewTopBtn = document.getElementById('top-view');
        this.viewAngleBtn = document.getElementById('angle-view');
        
        // Zoom controls
        this.zoomCloseBtn = document.getElementById('zoom-close');
        this.zoomNormalBtn = document.getElementById('zoom-normal');
        this.zoomFarBtn = document.getElementById('zoom-far');
        
        // Rotation controls
        this.rotationOffBtn = document.getElementById('rotation-off');
        this.rotationSlowBtn = document.getElementById('rotation-slow');
        this.rotationFastBtn = document.getElementById('rotation-fast');
        
        // Drag state
        this.isDragging = false;
        this.lastX = 0;
        this.lastY = 0;
        
        // Wait for module to initialize
        this.moduleReady = false;
        
        // Auto-updating info
        this.updateIntervalId = null;
    }
    
    /**
     * Initialize the simulator UI
     */
    initialize() {
        // Check WebGL support first
        if (!this.checkWebGL2Support()) {
            return;
        }
        
        // Set up event handlers for UI elements
        this.setupEventHandlers();
        
        // Set up auto-updating model info
        this.startModelInfoUpdates();
        
        // Wait for WASM module to initialize
        this.waitForModuleReady();
    }
    
    /**
     * Check for WebGL2 support
     * @returns {boolean} Whether WebGL2 is supported
     */
    checkWebGL2Support() {
        const canvas = document.createElement('canvas');
        const gl = canvas.getContext('webgl2');
        if (!gl) {
            document.getElementById('webgl-error').style.display = 'block';
            console.error('WebGL2 not supported');
            return false;
        }
        console.log('WebGL2 is supported!');
        return true;
    }
    
    /**
     * Safe function call to Module
     * @param {string} funcName - Function name without the _ prefix
     * @param {...any} args - Arguments to pass to the function
     * @returns {any} Result from the function call or null if function not found
     */
    callModule(funcName, ...args) {
        const fullName = '_' + funcName;
        if (!this.moduleReady) {
            console.warn(`Module not ready when calling ${funcName}`);
            return null;
        }
        
        if (typeof Module[fullName] === 'function') {
            try {
                return Module[fullName](...args);
            } catch (error) {
                console.error(`Error calling ${funcName}:`, error);
                return null;
            }
        } else {
            console.error(`Function ${funcName} not found in Module`);
            return null;
        }
    }
    
    /**
     * Wait for the WASM module to be ready
     */
    waitForModuleReady() {
        if (typeof Module !== 'undefined' && Module.ready) {
            this.onModuleReady();
        } else {
            // Set up the onRuntimeInitialized callback
            Module.onRuntimeInitialized = () => {
                this.onModuleReady();
            };
        }
    }
    
    /**
     * Called when the WASM module is initialized
     */
    onModuleReady() {
        console.log("WebAssembly runtime initialized");
        this.moduleReady = true;
        
        // Set up initial UI state
        this.setupScenes();
        this.setupControlValues();
    }
    
    /**
     * Set up scene buttons based on available scenes
     */
    setupScenes() {
        const numScenes = this.callModule('get_num_scenes');
        if (numScenes === null) return;
        
        console.log(`Found ${numScenes} scenes`);
        
        // Clear existing buttons
        this.sceneButtons.innerHTML = '';
        
        // Create a button for each scene
        for (let i = 0; i < numScenes; i++) {
            // Use a switch statement to match the C++ implementation
            let sceneName = "";
            switch (i) {
                case 0:
                    sceneName = "Test Scene";
                    break;
                case 1:
                    sceneName = "Blob Scene";
                    break;
                default:
                    sceneName = `Scene ${i}`;
                    break;
            }
            
            // Create the button
            const button = document.createElement('button');
            button.textContent = sceneName;
            button.dataset.sceneIndex = i;
            button.addEventListener('click', (e) => this.handleSceneChange(e));
            
            this.sceneButtons.appendChild(button);
            
            // Set the first scene as active
            if (i === 0) {
                button.classList.add('active');
            }
        }
    }
    
    /**
     * Set up the initial values for controls
     */
    setupControlValues() {
        // Set initial brightness
        const initialBrightness = parseInt(this.brightnessSlider.value);
        this.brightnessValue.textContent = initialBrightness;
        this.callModule('set_brightness', initialBrightness);
        
        // Set initial LED size
        const initialLedSize = parseInt(this.ledSizeSlider.value);
        this.ledSizeValue.textContent = initialLedSize;
        this.callModule('set_led_size', initialLedSize);
        
        // Set initial glow intensity
        const initialGlow = parseFloat(this.glowSlider.value) / 10.0;
        this.glowValue.textContent = initialGlow.toFixed(1);
        this.callModule('set_glow_intensity', initialGlow);
        
        // Set initial view
        this.callModule('set_preset_view', 0); // Start with side view
        this.setActiveViewButton(this.viewSideBtn);
        
        // Set initial zoom
        this.callModule('set_zoom_level', 1); // Normal zoom
        this.setActiveZoomButton(this.zoomNormalBtn);
        
        // Start with rotation off
        this.callModule('set_auto_rotation', false, 0);
        this.setActiveRotationButton(this.rotationOffBtn);
    }
    
    /**
     * Set up event handlers for UI controls
     */
    setupEventHandlers() {
        // Set up brightness slider
        if (this.brightnessSlider) {
            this.brightnessSlider.addEventListener('input', (e) => this.handleBrightnessChange(e));
        }
        
        // Set up LED size slider
        if (this.ledSizeSlider) {
            this.ledSizeSlider.addEventListener('input', (e) => this.handleLEDSizeChange(e));
        }
        
        // Set up glow intensity slider
        if (this.glowSlider) {
            this.glowSlider.addEventListener('input', (e) => this.handleGlowChange(e));
        }
        
        // Set up debug buttons
        if (this.benchmarkBtn) {
            this.benchmarkBtn.addEventListener('click', () => this.handleBenchmark());
        }
        
        if (this.debugBtn) {
            this.debugBtn.addEventListener('click', () => this.handleToggleDebug());
        }
        
        if (this.modelInfoBtn) {
            this.modelInfoBtn.addEventListener('click', () => this.handleModelInfo());
        }
        
        // Set up view buttons
        if (this.viewSideBtn) {
            this.viewSideBtn.addEventListener('click', () => this.handleViewChange(0, this.viewSideBtn));
        }
        
        if (this.viewTopBtn) {
            this.viewTopBtn.addEventListener('click', () => this.handleViewChange(1, this.viewTopBtn));
        }
        
        if (this.viewAngleBtn) {
            this.viewAngleBtn.addEventListener('click', () => this.handleViewChange(2, this.viewAngleBtn));
        }
        
        // Set up zoom buttons
        if (this.zoomCloseBtn) {
            this.zoomCloseBtn.addEventListener('click', () => this.handleZoomChange(0, this.zoomCloseBtn));
        }
        
        if (this.zoomNormalBtn) {
            this.zoomNormalBtn.addEventListener('click', () => this.handleZoomChange(1, this.zoomNormalBtn));
        }
        
        if (this.zoomFarBtn) {
            this.zoomFarBtn.addEventListener('click', () => this.handleZoomChange(2, this.zoomFarBtn));
        }
        
        // Set up rotation buttons
        if (this.rotationOffBtn) {
            this.rotationOffBtn.addEventListener('click', () => this.handleRotationChange(false, 0, this.rotationOffBtn));
        }
        
        if (this.rotationSlowBtn) {
            this.rotationSlowBtn.addEventListener('click', () => this.handleRotationChange(true, 1.0, this.rotationSlowBtn));
        }
        
        if (this.rotationFastBtn) {
            this.rotationFastBtn.addEventListener('click', () => this.handleRotationChange(true, 3.0, this.rotationFastBtn));
        }
        
        // Set up canvas interaction
        this.setupCanvasInteraction();
    }
    
    /**
     * Set up canvas mouse and touch interactions
     */
    setupCanvasInteraction() {
        if (!this.canvas) return;
        
        // Double click to reset rotation
        this.canvas.addEventListener('dblclick', (event) => {
            console.log("Double click detected - resetting view");
            this.callModule('reset_rotation');
            this.setActiveRotationButton(this.rotationOffBtn);
            event.preventDefault();
        });
        
        // Mouse down to start dragging
        this.canvas.addEventListener('mousedown', (event) => {
            console.log("Mouse down detected at", event.clientX, event.clientY);
            this.isDragging = true;
            this.lastX = event.clientX;
            this.lastY = event.clientY;
            event.preventDefault();
            
            // When user starts dragging, turn off auto-rotation
            this.callModule('set_auto_rotation', false, 0);
            this.setActiveRotationButton(this.rotationOffBtn);
            
            // Change cursor to indicate dragging
            this.canvas.style.cursor = 'grabbing';
        });
        
        // Mouse move to update rotation
        window.addEventListener('mousemove', (event) => {
            if (!this.isDragging) return;
            
            const deltaX = event.clientX - this.lastX;
            const deltaY = event.clientY - this.lastY;
            this.lastX = event.clientX;
            this.lastY = event.clientY;
            
            if (deltaX !== 0 || deltaY !== 0) {
                this.callModule('update_rotation', deltaX, deltaY);
            }
        });
        
        // Mouse up to stop dragging
        window.addEventListener('mouseup', () => {
            if (this.isDragging) {
                this.isDragging = false;
                this.canvas.style.cursor = 'grab';
            }
        });
        
        window.addEventListener('mouseleave', () => {
            if (this.isDragging) {
                this.isDragging = false;
                this.canvas.style.cursor = 'grab';
            }
        });
        
        // Touch events for mobile
        this.canvas.addEventListener('touchstart', (event) => {
            if (event.touches.length === 1) {
                this.isDragging = true;
                this.lastX = event.touches[0].clientX;
                this.lastY = event.touches[0].clientY;
                
                // When user starts dragging, turn off auto-rotation
                this.callModule('set_auto_rotation', false, 0);
                this.setActiveRotationButton(this.rotationOffBtn);
                
                event.preventDefault();
            }
        });
        
        this.canvas.addEventListener('touchmove', (event) => {
            if (!this.isDragging || event.touches.length !== 1) return;
            
            const deltaX = event.touches[0].clientX - this.lastX;
            const deltaY = event.touches[0].clientY - this.lastY;
            this.lastX = event.touches[0].clientX;
            this.lastY = event.touches[0].clientY;
            
            if (deltaX !== 0 || deltaY !== 0) {
                this.callModule('update_rotation', deltaX * 0.5, deltaY * 0.5);
            }
            
            event.preventDefault();
        });
        
        this.canvas.addEventListener('touchend', () => {
            this.isDragging = false;
        });
    }
    
    /**
     * Start the model info update interval
     */
    startModelInfoUpdates() {
        if (this.updateIntervalId) {
            clearInterval(this.updateIntervalId);
        }
        
        this.updateIntervalId = setInterval(() => this.updateModelInfo(), 1000);
    }
    
    /**
     * Update model information display
     */
    updateModelInfo() {
        if (!this.moduleReady) return;
        
        // Update LED count if available
        if (this.ledCountElement) {
            const ledCount = this.callModule('get_led_count');
            if (ledCount !== null) {
                this.ledCountElement.textContent = ledCount;
            }
        }
        
        // Update FPS if available
        if (this.fpsElement) {
            const fps = this.callModule('get_fps');
            if (fps !== null) {
                this.fpsElement.textContent = Math.round(fps);
            }
        }
    }
    
    /**
     * Handle scene button click
     * @param {Event} event - Click event
     */
    handleSceneChange(event) {
        const sceneIndex = parseInt(event.target.dataset.sceneIndex);
        console.log(`Switching to scene: ${event.target.textContent} (index: ${sceneIndex})`);
        
        this.callModule('set_scene', sceneIndex);
        
        // Update active button state
        document.querySelectorAll('#scene-buttons button').forEach(btn => {
            btn.classList.remove('active');
        });
        event.target.classList.add('active');
    }
    
    /**
     * Handle brightness slider change
     * @param {Event} event - Input event
     */
    handleBrightnessChange(event) {
        const value = parseInt(event.target.value);
        this.brightnessValue.textContent = value;
        console.log(`Setting brightness to: ${value}`);
        this.callModule('set_brightness', value);
    }
    
    /**
     * Handle LED size slider change
     * @param {Event} event - Input event
     */
    handleLEDSizeChange(event) {
        const size = parseInt(event.target.value);
        this.ledSizeValue.textContent = size;
        console.log(`Setting LED size to: ${size}`);
        this.callModule('set_led_size', size);
    }
    
    /**
     * Handle glow intensity slider change
     * @param {Event} event - Input event
     */
    handleGlowChange(event) {
        const intensity = parseFloat(event.target.value) / 10.0; // Convert to 0.0-2.0 range
        this.glowValue.textContent = intensity.toFixed(1);
        console.log(`Setting glow intensity to: ${intensity.toFixed(1)}`);
        this.callModule('set_glow_intensity', intensity);
    }
    
    /**
     * Handle benchmark button click
     */
    handleBenchmark() {
        console.log("Running benchmark test");
        this.callModule('show_benchmark_report');
    }
    
    /**
     * Handle toggle debug button click
     */
    handleToggleDebug() {
        console.log("Toggling debug mode");
        this.callModule('toggle_debug_mode');
    }
    
    /**
     * Handle model info button click
     */
    handleModelInfo() {
        console.log("Showing model information");
        this.callModule('print_model_info');
    }
    
    /**
     * Handle view preset change
     * @param {number} viewIndex - View preset index
     * @param {HTMLElement} button - Button that was clicked
     */
    handleViewChange(viewIndex, button) {
        console.log(`Switching to view preset: ${viewIndex}`);
        this.callModule('set_preset_view', viewIndex);
        this.setActiveViewButton(button);
    }
    
    /**
     * Handle zoom level change
     * @param {number} zoomLevel - Zoom level index
     * @param {HTMLElement} button - Button that was clicked
     */
    handleZoomChange(zoomLevel, button) {
        console.log(`Switching to zoom level: ${zoomLevel}`);
        this.callModule('set_zoom_level', zoomLevel);
        this.setActiveZoomButton(button);
    }
    
    /**
     * Handle rotation setting change
     * @param {boolean} enabled - Whether auto-rotation is enabled
     * @param {number} speed - Rotation speed
     * @param {HTMLElement} button - Button that was clicked
     */
    handleRotationChange(enabled, speed, button) {
        console.log(`Setting auto-rotation: ${enabled ? 'ON' : 'OFF'}, speed: ${speed}`);
        this.callModule('set_auto_rotation', enabled, speed);
        this.setActiveRotationButton(button);
    }
    
    /**
     * Set active view button
     * @param {HTMLElement} activeBtn - Button to set as active
     */
    setActiveViewButton(activeBtn) {
        [this.viewSideBtn, this.viewTopBtn, this.viewAngleBtn].forEach(btn => {
            if (btn) btn.classList.remove('active');
        });
        if (activeBtn) activeBtn.classList.add('active');
    }
    
    /**
     * Set active zoom button
     * @param {HTMLElement} activeBtn - Button to set as active
     */
    setActiveZoomButton(activeBtn) {
        [this.zoomCloseBtn, this.zoomNormalBtn, this.zoomFarBtn].forEach(btn => {
            if (btn) btn.classList.remove('active');
        });
        if (activeBtn) activeBtn.classList.add('active');
    }
    
    /**
     * Set active rotation button
     * @param {HTMLElement} activeBtn - Button to set as active
     */
    setActiveRotationButton(activeBtn) {
        [this.rotationOffBtn, this.rotationSlowBtn, this.rotationFastBtn].forEach(btn => {
            if (btn) btn.classList.remove('active');
        });
        if (activeBtn) activeBtn.classList.add('active');
    }
}

// Initialize the simulator when the document is ready
document.addEventListener('DOMContentLoaded', () => {
    window.simulator = new DodecaSimulator();
    window.simulator.initialize();
}); 