/**
 * DodecaRGB Web Simulator UI
 * Handles all the UI interaction, controls, and WASM module integration
 */

// Set up external C functions in the global scope
window._get_canvas_width = () => {
    const canvas = document.getElementById('canvas');
    return canvas ? canvas.width : 800;
};

window._get_canvas_height = () => {
    const canvas = document.getElementById('canvas');
    return canvas ? canvas.height : 600;
};

window._get_current_time = () => {
    return performance.now() / 1000.0;
};

window._update_ui_fps = (fps) => {
    const fpsElement = document.getElementById('fps');
    if (fpsElement) {
        fpsElement.textContent = Math.round(fps);
    }
};

window._update_ui_brightness = (brightness) => {
    const brightnessSlider = document.getElementById('brightness');
    const brightnessValue = document.getElementById('brightness-value');
    if (brightnessSlider && brightnessValue) {
        // Convert from 0-1 to 0-100 for the slider
        const percentage = Math.round(brightness * 100);
        brightnessSlider.value = percentage;
        brightnessValue.textContent = percentage;
    }
};

class DodecaSimulator {
    constructor() {
        // Store DOM elements
        this.canvas = document.getElementById('canvas');
        this.sceneButtons = document.getElementById('scene-buttons');
        this.brightnessSlider = document.getElementById('brightness');
        this.brightnessValue = document.getElementById('brightness-value');
        this.ledSizeSlider = document.getElementById('led-size');
        this.ledSizeValue = document.getElementById('led-size-value');
        this.atmosphereSlider = document.getElementById('glow-intensity');
        this.atmosphereValue = document.getElementById('glow-intensity-value');
        this.meshOpacitySlider = document.getElementById('mesh-opacity');
        this.meshOpacityValue = document.getElementById('mesh-opacity-value');
        this.showMeshCheckbox = document.getElementById('show-mesh');
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

        // Set up external C function implementations BEFORE module initialization
        this.setupExternalFunctions();
    }
    
    /**
     * Set up external C function implementations
     */
    setupExternalFunctions() {
        // Define the functions
        const getCanvasWidth = () => {
            return this.canvas ? this.canvas.width : 800;
        };
        
        const getCanvasHeight = () => {
            return this.canvas ? this.canvas.height : 600;
        };
        
        const getCurrentTime = () => {
            return performance.now() / 1000.0;
        };
        
        const updateUIFps = (fps) => {
            if (this.fpsElement) {
                this.fpsElement.textContent = Math.round(fps);
            }
        };
        
        const updateUIBrightness = (brightness) => {
            if (this.brightnessSlider && this.brightnessValue) {
                // Convert from 0-1 to 0-100 for the slider
                const percentage = Math.round(brightness * 100);
                this.brightnessSlider.value = percentage;
                this.brightnessValue.textContent = percentage;
            }
        };

        // Set up on window object
        window._get_canvas_width = getCanvasWidth;
        window._get_canvas_height = getCanvasHeight;
        window._get_current_time = getCurrentTime;
        window._update_ui_fps = updateUIFps;
        window._update_ui_brightness = updateUIBrightness;

        // Set up on Module object if it exists
        if (typeof Module !== 'undefined') {
            Module._get_canvas_width = getCanvasWidth;
            Module._get_canvas_height = getCanvasHeight;
            Module._get_current_time = getCurrentTime;
            Module._update_ui_fps = updateUIFps;
            Module._update_ui_brightness = updateUIBrightness;
        }
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
        
        // Add a short delay before initial setup to ensure everything is properly initialized
        console.log("Setting up initial state in 100ms...");
        setTimeout(() => {
            // Set up initial UI state
            this.setupScenes();
            this.setupControlValues();
            console.log("Initial UI state setup complete");
        }, 100);
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
            
            // Set the Blob Scene (index 1) as active by default to match C++ initialization
            if (i === 1) {
                button.classList.add('active');
            }
        }
    }
    
    /**
     * Set up the initial values for controls
     */
    setupControlValues() {
        // Get actual current values from the simulator (if available)
        
        // Brightness - internal value is 0-255, UI is 0-100
        let actualBrightness;
        try {
            // Get the current brightness from C++
            actualBrightness = this.callModule('get_brightness');
            if (actualBrightness === null) {
                throw new Error("Could not get brightness");
            }
            
            // Convert from internal brightness (0-255) to percentage (0-100)
            const brightnessPercentage = Math.round((actualBrightness * 100) / 255);
            
            // Update the slider and text
            this.brightnessSlider.value = brightnessPercentage;
            this.brightnessValue.textContent = brightnessPercentage;
        } catch (error) {
            // Fallback to slider default if we can't get the actual value
            const initialBrightness = parseInt(this.brightnessSlider.value);
            this.brightnessValue.textContent = initialBrightness;
            // Convert from percentage (0-100) to the internal brightness range (0-255)
            const scaledBrightness = Math.round((initialBrightness * 255) / 100);
            this.callModule('set_brightness', scaledBrightness);
        }
        
        // LED size - get the actual value
        try {
            const actualLedSize = this.callModule('get_led_size');
            if (actualLedSize !== null) {
                this.ledSizeSlider.value = actualLedSize;
                this.ledSizeValue.textContent = parseFloat(actualLedSize).toFixed(1);
                console.log(`Setting initial LED size: ${parseFloat(actualLedSize).toFixed(1)}x`);
            } else {
                throw new Error("Could not get LED size");
            }
        } catch (error) {
            // Fallback to slider default
            const initialLedSize = parseFloat(this.ledSizeSlider.value);
            this.ledSizeValue.textContent = initialLedSize.toFixed(1);
            this.callModule('set_led_size', initialLedSize);
            console.log(`Fallback LED size: ${initialLedSize.toFixed(1)}x`);
        }
        
        // Atmosphere intensity - get the actual value
        try {
            const actualAtmosphereIntensity = this.callModule('get_atmosphere_intensity');
            if (actualAtmosphereIntensity !== null) {
                // Convert from internal range (typically 0.0-3.0) to slider range (0-30)
                const atmosphereSliderValue = Math.round(actualAtmosphereIntensity * 10);
                this.atmosphereSlider.value = atmosphereSliderValue;
                this.atmosphereValue.textContent = actualAtmosphereIntensity.toFixed(1);
                console.log(`Setting initial atmosphere intensity: value=${actualAtmosphereIntensity.toFixed(1)}, slider=${atmosphereSliderValue}`);
            } else {
                throw new Error("Could not get atmosphere intensity");
            }
        } catch (error) {
            // Fallback to slider default - but make sure we use the slider's actual value
            const atmosphereSliderValue = parseInt(this.atmosphereSlider.value);
            // Convert range 0-30 to 0.0-3.0
            const initialAtmosphere = atmosphereSliderValue / 10.0;
            this.atmosphereValue.textContent = initialAtmosphere.toFixed(1);
            this.callModule('set_atmosphere_intensity', initialAtmosphere);
            console.log(`Fallback atmosphere intensity: value=${initialAtmosphere.toFixed(1)}, slider=${atmosphereSliderValue}`);
        }
        
        // Mesh opacity - get the actual value
        try {
            const actualMeshOpacity = this.callModule('get_mesh_opacity');
            if (actualMeshOpacity !== null) {
                this.meshOpacitySlider.value = actualMeshOpacity;
                this.meshOpacityValue.textContent = actualMeshOpacity.toFixed(1);
                console.log(`Setting initial mesh opacity: ${actualMeshOpacity.toFixed(1)}`);
            } else {
                throw new Error("Could not get mesh opacity");
            }
        } catch (error) {
            // Fallback to slider default
            const initialMeshOpacity = parseFloat(this.meshOpacitySlider.value);
            this.meshOpacityValue.textContent = initialMeshOpacity.toFixed(1);
            this.callModule('set_mesh_opacity', initialMeshOpacity);
            console.log(`Fallback mesh opacity: ${initialMeshOpacity.toFixed(1)}`);
        }
        
        // Show mesh checkbox - get the actual value
        try {
            const showMesh = this.callModule('get_show_mesh');
            if (showMesh !== null) {
                this.showMeshCheckbox.checked = showMesh;
                console.log(`Setting initial show mesh: ${showMesh ? 'ON' : 'OFF'}`);
            } else {
                throw new Error("Could not get show mesh state");
            }
        } catch (error) {
            // Fallback to checkbox default
            const initialShowMesh = this.showMeshCheckbox.checked;
            this.callModule('set_show_mesh', initialShowMesh);
            console.log(`Fallback show mesh: ${initialShowMesh ? 'ON' : 'OFF'}`);
        }
        
        // Set initial view
        this.callModule('set_preset_view', 0); // Start with side view
        this.setActiveViewButton(this.viewSideBtn);
        
        // Set initial zoom
        this.callModule('set_zoom_level', 1); // Normal zoom
        this.setActiveZoomButton(this.zoomNormalBtn);
        
        // Start with rotation off
        this.callModule('set_auto_rotation', false, 0);
        this.setActiveRotationButton(this.rotationOffBtn);
        
        // Set up atmosphere intensity slider
        if (this.atmosphereSlider) {
            this.atmosphereSlider.addEventListener('input', (e) => this.handleAtmosphereChange(e));
        }
        
        // Set up mesh opacity slider
        if (this.meshOpacitySlider) {
            this.meshOpacitySlider.addEventListener('input', (e) => this.handleMeshOpacityChange(e));
        }
        
        // Set up show mesh checkbox
        if (this.showMeshCheckbox) {
            this.showMeshCheckbox.addEventListener('change', (e) => this.handleShowMeshChange(e));
        }
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
        
        // Set up atmosphere intensity slider
        if (this.atmosphereSlider) {
            this.atmosphereSlider.addEventListener('input', (e) => this.handleAtmosphereChange(e));
        }
        
        // Set up mesh opacity slider
        if (this.meshOpacitySlider) {
            this.meshOpacitySlider.addEventListener('input', (e) => this.handleMeshOpacityChange(e));
        }
        
        // Set up show mesh checkbox
        if (this.showMeshCheckbox) {
            this.showMeshCheckbox.addEventListener('change', (e) => this.handleShowMeshChange(e));
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
        
        // Convert from percentage (0-100) to the internal brightness range (0-255)
        const scaledBrightness = Math.round((value * 255) / 100);
        console.log(`Setting brightness to: ${value}% (${scaledBrightness}/255)`);
        this.callModule('set_brightness', scaledBrightness);
    }
    
    /**
     * Handle LED size slider change
     * @param {Event} event - Input event
     */
    handleLEDSizeChange(event) {
        const size = parseFloat(event.target.value);
        this.ledSizeValue.textContent = size.toFixed(1);
        console.log(`Setting LED size ratio to: ${size.toFixed(1)}x`);
        this.callModule('set_led_size', size);
    }
    
    /**
     * Handle atmosphere intensity slider change
     * @param {Event} event - Input event
     */
    handleAtmosphereChange(event) {
        const rawValue = parseFloat(event.target.value);
        // Convert range 0-30 to 0.0-3.0
        const intensity = rawValue / 10.0;
        this.atmosphereValue.textContent = intensity.toFixed(1);
        console.log(`Setting atmosphere intensity to: ${intensity.toFixed(1)}`);
        this.callModule('set_atmosphere_intensity', intensity);
    }
    
    /**
     * Handle mesh opacity slider change
     * @param {Event} event - Input event
     */
    handleMeshOpacityChange(event) {
        const value = parseFloat(event.target.value);
        this.meshOpacityValue.textContent = value.toFixed(1);
        console.log(`Setting mesh opacity to: ${value.toFixed(1)}`);
        this.callModule('set_mesh_opacity', value);
    }
    
    /**
     * Handle show mesh checkbox change
     * @param {Event} event - Change event
     */
    handleShowMeshChange(event) {
        const enabled = event.target.checked;
        console.log(`Setting show mesh: ${enabled ? 'ON' : 'OFF'}`);
        this.callModule('set_show_mesh', enabled);
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