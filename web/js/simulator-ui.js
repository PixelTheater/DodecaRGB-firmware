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

        // UI elements
        this.fpsCounter = document.getElementById('fps-counter');
        this.sceneSelector = document.querySelector('.scene-selector');
        this.selectedSceneText = document.querySelector('.selected-scene');
        this.sceneIndicator = document.querySelector('.scene-indicator');
        this.prevButton = document.querySelector('.prev-button');
        this.nextButton = document.querySelector('.next-button');
        this.sceneParams = document.querySelector('.scene-params');
        
        // Scene state
        this.currentSceneIndex = 0;
        this.totalScenes = 0;
        this.sceneNames = [];
        
        // Ensure canvas is square
        this.resizeCanvas();
        window.addEventListener('resize', () => this.resizeCanvas());
        
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
            return this.canvas ? this.canvas.height : 800;
        };
        
        const getCurrentTime = () => {
            return performance.now() / 1000.0;
        };
        
        const updateUIFps = (fps) => {
            if (this.fpsCounter) {
                this.fpsCounter.textContent = `${Math.round(fps)}fps`;
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
     * Initialize the simulator
     */
    initialize() {
        console.log("Initializing DodecaRGB Simulator UI");
        
        // Check WebGL2 support
        if (!this.checkWebGL2Support()) {
            return;
        }
        
        // Set up event handlers
        this.setupEventHandlers();
        
        // Set up canvas interaction
        this.setupCanvasInteraction();
        
        // Wait for module to be ready
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
     * Call a function in the WASM module
     * @param {string} funcName - Name of the function to call
     * @param {...any} args - Arguments to pass to the function
     * @returns {any} - Return value from the function
     */
    callModule(funcName, ...args) {
        if (!this.moduleReady) {
            console.warn(`Module not ready, cannot call ${funcName}`);
            return null;
        }
        
        // Check if the function exists in the Module
        if (typeof Module['_' + funcName] !== 'function') {
            console.warn(`Function ${funcName} not found in Module`);
            return null;
        }
        
        try {
            // Handle special case for update_scene_parameter which takes a string
            if (funcName === 'update_scene_parameter' && typeof args[0] === 'string') {
                const paramIdPtr = this.Module.allocateUTF8(args[0]);
                const result = Module['_' + funcName](paramIdPtr, args[1]);
                this.Module._free(paramIdPtr);
                return result;
            }
            
            // Normal case for other functions
            return Module['_' + funcName](...args);
        } catch (e) {
            console.error(`Error calling ${funcName}:`, e);
            return null;
        }
    }
    
    /**
     * Wait for the WASM module to be ready
     */
    waitForModuleReady() {
        // Set up a callback for when the module is ready
        if (typeof Module !== 'undefined') {
            // If Module already has onRuntimeInitialized, store the original
            const originalCallback = Module.onRuntimeInitialized;
            
            // Set our own callback
            Module.onRuntimeInitialized = () => {
                // Call the original callback if it exists
                if (typeof originalCallback === 'function') {
                    originalCallback();
                }
                
                // Mark module as ready and initialize
                this.moduleReady = true;
                console.log("Module runtime initialized");
                
                // Wait a bit longer to ensure all module functions are available
                setTimeout(() => {
                    this.onModuleReady();
                }, 1000);
            };
        } else {
            // Module not defined yet, check again later
            console.log("Module not defined yet, waiting...");
            setTimeout(() => this.waitForModuleReady(), 100);
        }
    }
    
    /**
     * Called when the module is ready
     */
    onModuleReady() {
        console.log("Module ready, initializing simulator...");
        
        // Ensure canvas is properly sized
        this.resizeCanvas();
        
        // Set up external functions
        this.setupExternalFunctions();
        
        // Debug: List available functions in the Module
        this.listAvailableModuleFunctions();
        
        // Check if Embind functions are available
        const embindAvailable = this.checkEmbindFunctions();
        console.log("Embind functions available:", embindAvailable);
        
        // Add a delay before initial setup to ensure everything is properly initialized
        setTimeout(() => {
            try {
                // Initialize UI elements
                this.setupControlValues();
                
                // Set up scenes
                this.setupScenes();
                
                // Start model info updates
                this.startModelInfoUpdates();
                
                console.log("Simulator initialization complete");
            } catch (error) {
                console.error("Error during simulator initialization:", error);
            }
        }, 1000);
    }
    
    /**
     * List available functions in the Module for debugging
     */
    listAvailableModuleFunctions() {
        if (typeof Module !== 'undefined') {
            console.log("Available Module functions:");
            
            // List all properties that look like functions
            const functionNames = Object.keys(Module).filter(key => 
                typeof Module[key] === 'function' && key.startsWith('_')
            );
            
            functionNames.forEach(name => {
                console.log(`- ${name}`);
            });
            
            // Check for specific functions we're interested in
            const checkFunctions = [
                '_change_scene', 
                '_set_brightness',
                '_set_led_size',
                '_set_atmosphere_intensity',
                '_set_mesh_opacity',
                '_set_show_mesh',
                '_get_num_scenes'
            ];
            
            console.log("Checking for specific functions:");
            checkFunctions.forEach(name => {
                console.log(`- ${name}: ${typeof Module[name] === 'function' ? 'Available' : 'Not found'}`);
            });
        } else {
            console.warn("Module is not defined");
        }
    }
    
    /**
     * Set up scenes and populate scene selector
     */
    setupScenes() {
        // Wait for module to be ready before setting up scenes
        if (!this.moduleReady) {
            console.log("Module not ready, deferring scene setup");
            setTimeout(() => this.setupScenes(), 100);
            return;
        }
        
        const numScenes = this.callModule('get_num_scenes');
        if (numScenes === null) {
            console.log("Could not get number of scenes, using default scenes");
            this.totalScenes = 8; // Default number of scenes
            
            // Create default scene names
            this.sceneNames = [
                "Test Pattern",
                "Wandering Blobs",
                "Color Waves",
                "Fire Effect",
                "Rainbow Cycle",
                "Starfield",
                "Audio Visualizer",
                "Matrix Rain"
            ];
            
            // Set Wandering Blobs (index 1) as active by default
            this.currentSceneIndex = 1;
            this.updateSceneUI();
            this.updateSceneParameters(this.currentSceneIndex);
            return;
        }
        
        console.log(`Found ${numScenes} scenes`);
        this.totalScenes = numScenes;
        
        // Create scene names array
        this.sceneNames = [];
        for (let i = 0; i < numScenes; i++) {
            // Use a switch statement to match the C++ implementation
            let sceneName = "";
            switch (i) {
                case 0:
                    sceneName = "Test Pattern";
                    break;
                case 1:
                    sceneName = "Wandering Blobs";
                    break;
                case 2:
                    sceneName = "Color Waves";
                    break;
                case 3:
                    sceneName = "Fire Effect";
                    break;
                case 4:
                    sceneName = "Rainbow Cycle";
                    break;
                case 5:
                    sceneName = "Starfield";
                    break;
                case 6:
                    sceneName = "Audio Visualizer";
                    break;
                case 7:
                    sceneName = "Matrix Rain";
                    break;
                default:
                    sceneName = `Scene ${i}`;
                    break;
            }
            this.sceneNames.push(sceneName);
        }
        
        // Set Wandering Blobs (index 1) as active by default
        this.currentSceneIndex = 1;
        this.updateSceneUI();
        this.updateSceneParameters(this.currentSceneIndex);
    }
    
    /**
     * Update scene UI elements
     */
    updateSceneUI() {
        // Update scene indicator (e.g., "1/8")
        if (this.sceneIndicator) {
            this.sceneIndicator.textContent = `${this.currentSceneIndex + 1}/${this.totalScenes}`;
        }
        
        // Update selected scene text
        if (this.selectedSceneText && this.sceneNames[this.currentSceneIndex]) {
            this.selectedSceneText.textContent = this.sceneNames[this.currentSceneIndex];
        }
        
        // Set the scene using the available function
        console.log(`Setting scene to index: ${this.currentSceneIndex}`);
        
        if (typeof Module._change_scene === 'function') {
            Module._change_scene(this.currentSceneIndex);
        } else {
            console.warn("Scene change function not found in Module");
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
        
        // Scene navigation
        this.prevButton.addEventListener('click', () => this.navigateScene(-1));
        this.nextButton.addEventListener('click', () => this.navigateScene(1));
        this.sceneSelector.addEventListener('click', () => this.showSceneDropdown());
        
        // Set up canvas interaction
        this.setupCanvasInteraction();
        
        // Console controls
        const toggleConsoleBtn = document.getElementById('toggle-console');
        const clearConsoleBtn = document.getElementById('clear-console');
        const consoleContainer = document.getElementById('console-container');
        
        if (toggleConsoleBtn) {
            toggleConsoleBtn.addEventListener('click', () => {
                const isVisible = consoleContainer.style.display !== 'none';
                consoleContainer.style.display = isVisible ? 'none' : 'block';
                toggleConsoleBtn.textContent = isVisible ? 'Show Console' : 'Hide Console';
            });
        }
        
        if (clearConsoleBtn) {
            clearConsoleBtn.addEventListener('click', () => {
                const consoleElement = document.getElementById('console');
                if (consoleElement) consoleElement.innerHTML = '';
            });
        }
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
        try {
            const brightness = parseInt(event.target.value);
            document.getElementById('brightness-value').textContent = brightness;
            
            // Convert from percentage (0-100) to byte (0-255)
            const scaledBrightness = Math.round((brightness * 255) / 100);
            console.log(`Setting brightness to: ${brightness}% (${scaledBrightness}/255)`);
            
            // Call the available function
            if (typeof Module._set_brightness === 'function') {
                Module._set_brightness(scaledBrightness);
            } else {
                console.warn("Brightness function not found in Module");
            }
        } catch (error) {
            console.error("Error setting brightness:", error);
        }
    }
    
    /**
     * Handle LED size slider change
     * @param {Event} event - Input event
     */
    handleLEDSizeChange(event) {
        try {
            const size = parseFloat(event.target.value);
            document.getElementById('led-size-value').textContent = size.toFixed(1);
            console.log(`Setting LED size: ${size}x`);
            
            // Call the available function
            if (typeof Module._set_led_size === 'function') {
                Module._set_led_size(size);
            } else {
                console.warn("LED size function not found in Module");
            }
        } catch (error) {
            console.error("Error setting LED size:", error);
        }
    }
    
    /**
     * Handle atmosphere intensity slider change
     * @param {Event} event - Input event
     */
    handleAtmosphereChange(event) {
        try {
            const value = parseInt(event.target.value);
            const intensity = value / 10.0; // Convert from 0-30 to 0-3.0
            document.getElementById('glow-intensity-value').textContent = intensity.toFixed(1);
            console.log(`Setting atmosphere intensity: value=${intensity}, slider=${value}`);
            
            // Call the available function
            if (typeof Module._set_atmosphere_intensity === 'function') {
                Module._set_atmosphere_intensity(intensity);
            } else {
                console.warn("Atmosphere intensity function not found in Module");
            }
        } catch (error) {
            console.error("Error setting atmosphere intensity:", error);
        }
    }
    
    /**
     * Handle mesh opacity slider change
     * @param {Event} event - Input event
     */
    handleMeshOpacityChange(event) {
        try {
            const opacity = parseFloat(event.target.value);
            document.getElementById('mesh-opacity-value').textContent = opacity.toFixed(1);
            console.log(`Setting mesh opacity: ${opacity.toFixed(2)}`);
            
            // Call the available function
            if (typeof Module._set_mesh_opacity === 'function') {
                Module._set_mesh_opacity(opacity);
            } else {
                console.warn("Mesh opacity function not found in Module");
            }
        } catch (error) {
            console.error("Error setting mesh opacity:", error);
        }
    }
    
    /**
     * Handle show mesh checkbox change
     * @param {Event} event - Input event
     */
    handleShowMeshChange(event) {
        try {
            const show = event.target.checked;
            console.log(`Setting show mesh: ${show ? 'ON' : 'OFF'}`);
            
            // Call the available function
            if (typeof Module._set_show_mesh === 'function') {
                Module._set_show_mesh(show ? 1 : 0);
            } else {
                console.warn("Show mesh function not found in Module");
            }
        } catch (error) {
            console.error("Error setting show mesh:", error);
        }
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
    
    /**
     * Navigate to previous or next scene
     * @param {number} direction - Direction to navigate (-1 for prev, 1 for next)
     */
    navigateScene(direction) {
        try {
            // Calculate new scene index with wrapping
            const newIndex = (this.currentSceneIndex + direction + this.totalScenes) % this.totalScenes;
            console.log(`Navigating from scene ${this.currentSceneIndex} to ${newIndex}`);
            
            this.currentSceneIndex = newIndex;
            this.updateSceneUI();
            this.updateSceneParameters(this.currentSceneIndex);
        } catch (error) {
            console.error("Error navigating scene:", error);
        }
    }
    
    /**
     * Show scene selection dropdown (mock for now)
     */
    showSceneDropdown() {
        console.log('Scene dropdown clicked - would show dropdown menu here');
        // For now, just cycle to the next scene as a placeholder
        this.navigateScene(1);
    }
    
    /**
     * Update scene parameters UI based on the selected scene
     * @param {number} sceneIndex - Index of the selected scene
     */
    updateSceneParameters(sceneIndex) {
        // Clear existing parameters
        this.sceneParams.innerHTML = '';
        
        // Skip if module is not ready
        if (!this.moduleReady) {
            console.log("Module not ready, can't update scene parameters");
            return;
        }
        
        console.log(`Updating parameters for scene ${sceneIndex}`);
        
        try {
            // Get parameters using Embind
            const parameters = Module.getSceneParameters();
            console.log("Retrieved parameters:", parameters);
            
            if (!parameters || parameters.length === 0) {
                // No parameters for this scene
                const noParamsMsg = document.createElement('div');
                noParamsMsg.className = 'no-params-message';
                noParamsMsg.textContent = 'This scene has no adjustable parameters.';
                this.sceneParams.appendChild(noParamsMsg);
                return;
            }
            
            // Create UI controls for each parameter
            parameters.forEach(param => {
                this.addSceneParameter(param);
            });
        } catch (error) {
            console.error("Error updating scene parameters:", error);
            // Show error message in UI
            const errorMsg = document.createElement('div');
            errorMsg.className = 'error-message';
            errorMsg.textContent = 'Error loading scene parameters.';
            this.sceneParams.appendChild(errorMsg);
        }
    }
    
    /**
     * Add a parameter control to the scene parameters UI
     * @param {string} id - Parameter ID
     * @param {string} type - Parameter type (range, checkbox, etc.)
     * @param {number} min - Minimum value (for range)
     * @param {number} max - Maximum value (for range)
     * @param {any} value - Current value
     * @param {string} label - Parameter label
     */
    addSceneParameter(param) {
        const paramRow = document.createElement('div');
        paramRow.className = 'param-row';
        
        const paramLabel = document.createElement('label');
        paramLabel.textContent = param.label;
        paramRow.appendChild(paramLabel);
        
        let input;
        let valueDisplay;
        
        switch (param.controlType) {
            case 'slider':
                // Create slider
                input = document.createElement('input');
                input.type = 'range';
                input.min = param.min;
                input.max = param.max;
                input.step = param.step;
                input.value = parseFloat(param.value);
                input.id = `scene-param-${param.id}`;
                
                // Create value display
                valueDisplay = document.createElement('span');
                valueDisplay.className = 'param-value';
                valueDisplay.textContent = parseFloat(param.value).toFixed(2);
                
                // Add event listener
                input.addEventListener('input', (e) => {
                    const newValue = parseFloat(e.target.value);
                    valueDisplay.textContent = newValue.toFixed(2);
                    this.handleSceneParameterChange(param.id, newValue.toString());
                });
                
                paramRow.appendChild(input);
                paramRow.appendChild(valueDisplay);
                break;
                
            case 'checkbox':
                // Create checkbox
                input = document.createElement('input');
                input.type = 'checkbox';
                input.checked = param.value === "true";
                input.id = `scene-param-${param.id}`;
                
                const checkboxLabel = document.createElement('label');
                checkboxLabel.appendChild(input);
                checkboxLabel.appendChild(document.createTextNode(param.label));
                
                // Add event listener
                input.addEventListener('change', (e) => {
                    this.handleSceneParameterChange(param.id, e.target.checked.toString());
                });
                
                paramRow.innerHTML = '';
                paramRow.appendChild(checkboxLabel);
                break;
                
            case 'select':
                // Create select dropdown
                input = document.createElement('select');
                input.id = `scene-param-${param.id}`;
                
                // Add options
                if (param.options && param.options.length > 0) {
                    param.options.forEach(option => {
                        const optionEl = document.createElement('option');
                        optionEl.value = option;
                        optionEl.textContent = option;
                        optionEl.selected = option === param.value;
                        input.appendChild(optionEl);
                    });
                }
                
                // Add event listener
                input.addEventListener('change', (e) => {
                    this.handleSceneParameterChange(param.id, e.target.value);
                });
                
                paramRow.appendChild(input);
                break;
        }
        
        this.sceneParams.appendChild(paramRow);
    }
    
    /**
     * Handle scene parameter change
     * @param {string} paramId - Parameter ID
     * @param {any} value - New value
     */
    handleSceneParameterChange(paramId, value) {
        console.log(`Scene parameter changed: ${paramId}, value: ${value}`);
        
        if (!this.moduleReady) {
            console.warn("Module not ready, can't update parameter");
            return;
        }
        
        try {
            // Call the Embind function to update the parameter
            Module.updateSceneParameter(paramId, value);
        } catch (error) {
            console.error(`Error updating parameter ${paramId}:`, error);
            
            // Fallback to the old method if Embind fails
            try {
                this.callModule('update_scene_parameter', paramId, parseFloat(value));
            } catch (fallbackError) {
                console.error(`Fallback also failed: ${fallbackError}`);
            }
        }
    }
    
    /**
     * Check if the Embind functions are available
     * @returns {boolean} True if Embind functions are available
     */
    checkEmbindFunctions() {
        // Check if the Embind functions are available
        if (typeof Module.getSceneParameters !== 'function') {
            console.warn("Module.getSceneParameters is not available");
            return false;
        }
        
        if (typeof Module.updateSceneParameter !== 'function') {
            console.warn("Module.updateSceneParameter is not available");
            return false;
        }
        
        console.log("Embind functions are available");
        return true;
    }
    
    /**
     * Resize the canvas based on CSS dimensions
     * No longer enforcing square aspect ratio
     */
    resizeCanvas() {
        // Get the computed style to see what size CSS has set
        const computedStyle = window.getComputedStyle(this.canvas);
        const cssWidth = parseInt(computedStyle.width, 10);
        
        // Calculate an optimal height based on some criteria
        // For example, you could use a specific aspect ratio:
        const optimalHeight = cssWidth * 0.70; // For a 4:3 aspect ratio
        
        // Set the canvas dimensions
        this.canvas.width = cssWidth;
        this.canvas.height = optimalHeight;
        
        console.log(`Canvas resized to ${this.canvas.width}x${this.canvas.height}`);
        
        // Notify the C++ code about the resize
        if (this.moduleReady) {
            this.callModule('resizeCanvas', this.canvas.width, this.canvas.height);
        }
    }
}

// Initialize the simulator when the document is ready
document.addEventListener('DOMContentLoaded', () => {
    window.simulator = new DodecaSimulator();
    window.simulator.initialize();
}); 