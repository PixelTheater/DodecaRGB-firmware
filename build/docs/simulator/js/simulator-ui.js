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
        this.currentSceneIndex = -1;  // Start with -1 to indicate no scene selected
        this.totalScenes = 0;
        this.sceneNames = [];
        this.isChangingScene = false;
        this.setupRetries = 0;  // Counter for scene setup retry attempts
        
        // Store parameter values for each scene
        this.sceneParameters = {};
        
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
        if (!this.moduleReady || typeof Module === 'undefined') {
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
                const paramIdPtr = Module.allocateUTF8(args[0]);
                const result = Module['_' + funcName](paramIdPtr, args[1]);
                Module._free(paramIdPtr);
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
     * Called when the WASM module is ready
     */
    onModuleReady() {
        console.log("Module is ready, initializing simulator");
        
        // Ensure canvas is properly sized
        this.resizeCanvas();
        
        // Set up external functions
        this.setupExternalFunctions();
        
        // Check if Embind functions are available
        this.hasEmbind = this.checkEmbindFunctions();
        
        // List available functions for debugging
        this.listAvailableModuleFunctions();
        
        // Set up scenes
        this.setupScenes();
        
        // Set up initial control values
        this.setupControlValues();
        
        // Set up event handlers
        this.setupEventHandlers();
        
        // Set up canvas interaction
        this.setupCanvasInteraction();
        
        // Start model info updates
        this.startModelInfoUpdates();
        
        console.log("Simulator initialization complete");
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
     * Change scene and wait for it to be ready
     * @param {number} sceneIndex - Index of scene to change to
     * @returns {Promise<void>} Promise that resolves when scene is ready
     */
    async changeScene(sceneIndex) {
        if (sceneIndex === this.currentSceneIndex) {
            return; // Already on this scene
        }

        try {
            // Change the scene
            const result = this.callModule('change_scene', sceneIndex);
            if (result === null) {
                throw new Error('Failed to change scene');
            }
            
            // Update UI immediately
            this.currentSceneIndex = sceneIndex;
            this.updateSceneIndicator();
            
            // Wait for next frame to ensure scene has changed
            await new Promise(resolve => requestAnimationFrame(resolve));
            
            // Update parameters
            await this.updateSceneParameters(sceneIndex);
            
        } catch (error) {
            console.error('Error changing scene:', error);
            throw error;
        }
    }

    /**
     * Navigate to previous or next scene
     * @param {number} direction - Direction to navigate (-1 for prev, 1 for next)
     */
    async navigateScene(direction) {
        if (this.isChangingScene) {
            return;
        }

        try {
            this.isChangingScene = true;
            const newIndex = (this.currentSceneIndex + direction + this.totalScenes) % this.totalScenes;
            
            await this.changeScene(newIndex);
            
        } catch (error) {
            console.error("Error navigating scene:", error);
        } finally {
            this.isChangingScene = false;
        }
    }

    /**
     * Set up scenes and populate scene selector
     */
    async setupScenes() {
        if (!this.moduleReady) {
            console.log("Module not ready, deferring scene setup");
            return;
        }
        
        try {
            const numScenes = this.callModule('get_num_scenes');
            if (numScenes === null || numScenes <= 0) {
                console.error("Could not get number of scenes from the simulator");
                return;
            }
            
            this.totalScenes = numScenes;
            this.sceneNames = [];
            
            // Create the select element if it doesn't exist
            if (!this.sceneSelect) {
                this.sceneSelect = document.createElement('select');
                this.sceneSelect.className = 'scene-select';
                
                // Replace the old scene selector with the new select element
                if (this.sceneSelector) {
                    this.sceneSelector.parentNode.replaceChild(this.sceneSelect, this.sceneSelector);
                }
                
                // Add change event listener
                this.sceneSelect.addEventListener('change', async (e) => {
                    const newIndex = parseInt(e.target.value);
                    if (!this.isChangingScene && newIndex !== this.currentSceneIndex) {
                        await this.changeScene(newIndex);
                    }
                });
            }
            
            // Clear existing options
            this.sceneSelect.innerHTML = '';
            
            // Add scene options
            for (let i = 0; i < numScenes; i++) {
                try {
                    const sceneName = Module.get_scene_name(i);
                    this.sceneNames.push(sceneName || `Scene ${i}`);
                    
                    const option = document.createElement('option');
                    option.value = i;
                    option.textContent = this.sceneNames[i];
                    this.sceneSelect.appendChild(option);
                } catch (error) {
                    console.error(`Error getting name for scene ${i}:`, error);
                    this.sceneNames.push(`Scene ${i}`);
                }
            }
            
            // Set to Wandering Particles scene by default (index 2)
            this.isChangingScene = true;
            await this.changeScene(2);
            
        } catch (error) {
            console.error("Error setting up scenes:", error);
        } finally {
            this.isChangingScene = false;
        }
    }
    
    /**
     * Get scene name using Emscripten's string handling
     */
    getSceneName(sceneIndex) {
        try {
            return Module.get_scene_name(sceneIndex) || `Scene ${sceneIndex}`;
        } catch (error) {
            console.error(`Error getting scene name for index ${sceneIndex}:`, error);
            return `Scene ${sceneIndex}`;
        }
    }
    
    /**
     * Update scene UI elements
     */
    updateSceneUI() {
        // Update scene indicator and other UI elements
        this.updateSceneIndicator();
        
        // Note: We no longer call Module._change_scene here to avoid double scene changes
        // That's now handled directly in navigateScene
    }
    
    /**
     * Set up the initial values for controls
     */
    setupControlValues() {
        // Get actual current values from the simulator (if available)
        
        // Brightness - internal value is 0-255, UI is 0-100
        try {
            // Get the current brightness from C++
            const actualBrightness = this.callModule('get_brightness');
            if (actualBrightness === null) {
                throw new Error("Could not get brightness");
            }
            
            // Convert from internal brightness (0-255) to percentage (0-100)
            const brightnessPercentage = Math.round((actualBrightness * 100) / 255);
            
            // Update the slider and text
            this.brightnessSlider.value = brightnessPercentage;
            this.brightnessValue.textContent = brightnessPercentage;
        } catch (error) {
            // Fallback to slider default
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
            } else {
                throw new Error("Could not get LED size");
            }
        } catch (error) {
            // Fallback to slider default
            const initialLedSize = parseFloat(this.ledSizeSlider.value);
            this.ledSizeValue.textContent = initialLedSize.toFixed(1);
            this.callModule('set_led_size', initialLedSize);
        }
        
        // Atmosphere intensity - get the actual value
        try {
            const actualAtmosphereIntensity = this.callModule('get_atmosphere_intensity');
            if (actualAtmosphereIntensity !== null) {
                // Convert from internal range (typically 0.0-3.0) to slider range (0-30)
                const atmosphereSliderValue = Math.round(actualAtmosphereIntensity * 10);
                this.atmosphereSlider.value = atmosphereSliderValue;
                this.atmosphereValue.textContent = actualAtmosphereIntensity.toFixed(1);
            } else {
                throw new Error("Could not get atmosphere intensity");
            }
        } catch (error) {
            // Fallback to slider default
            const atmosphereSliderValue = parseInt(this.atmosphereSlider.value);
            // Convert range 0-30 to 0.0-3.0
            const initialAtmosphere = atmosphereSliderValue / 10.0;
            this.atmosphereValue.textContent = initialAtmosphere.toFixed(1);
            this.callModule('set_atmosphere_intensity', initialAtmosphere);
        }
        
        // Mesh opacity - get the actual value
        try {
            const actualMeshOpacity = this.callModule('get_mesh_opacity');
            if (actualMeshOpacity !== null) {
                this.meshOpacitySlider.value = actualMeshOpacity;
                this.meshOpacityValue.textContent = actualMeshOpacity.toFixed(1);
            } else {
                throw new Error("Could not get mesh opacity");
            }
        } catch (error) {
            // Fallback to slider default
            const initialMeshOpacity = parseFloat(this.meshOpacitySlider.value);
            this.meshOpacityValue.textContent = initialMeshOpacity.toFixed(1);
            this.callModule('set_mesh_opacity', initialMeshOpacity);
        }
        
        // Show mesh checkbox - get the actual value
        try {
            const showMesh = this.callModule('get_show_mesh');
            if (showMesh !== null) {
                this.showMeshCheckbox.checked = showMesh;
            } else {
                throw new Error("Could not get show mesh state");
            }
        } catch (error) {
            // Fallback to checkbox default
            const initialShowMesh = this.showMeshCheckbox.checked;
            this.callModule('set_show_mesh', initialShowMesh);
        }
        
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
        this.prevButton.addEventListener('click', () => {
            // Only navigate if we're not already changing scenes
            if (!this.isChangingScene) {
                this.navigateScene(-1);
            } else {
                console.log("Ignoring navigation request - scene is already changing");
            }
        });
        
        this.nextButton.addEventListener('click', () => {
            // Only navigate if we're not already changing scenes
            if (!this.isChangingScene) {
                this.navigateScene(1);
            } else {
                console.log("Ignoring navigation request - scene is already changing");
            }
        });
        
        if (this.sceneSelector) {
            this.sceneSelector.addEventListener('click', (e) => this.showSceneDropdown(e));
        }
        
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
            this.callModule('reset_rotation');
            this.setActiveRotationButton(this.rotationOffBtn);
            event.preventDefault();
        });
        
        // Mouse down to start dragging
        this.canvas.addEventListener('mousedown', (event) => {
            this.isDragging = true;
            this.lastX = event.clientX;
            this.lastY = event.clientY;
            
            // When user starts dragging, turn off auto-rotation
            this.callModule('set_auto_rotation', false, 0);
            this.setActiveRotationButton(this.rotationOffBtn);
            
            // Change cursor to indicate dragging
            this.canvas.style.cursor = 'grabbing';
            event.preventDefault();
        });
        
        // Mouse move to update rotation
        const handleMouseMove = (event) => {
            if (!this.isDragging) return;
            
            const deltaX = event.clientX - this.lastX;
            const deltaY = event.clientY - this.lastY;
            this.lastX = event.clientX;
            this.lastY = event.clientY;
            
            if (deltaX !== 0 || deltaY !== 0) {
                this.callModule('update_rotation', deltaX, deltaY);
            }
        };
        
        // Mouse up to stop dragging
        const handleMouseUp = () => {
            if (this.isDragging) {
                this.isDragging = false;
                this.canvas.style.cursor = 'grab';
            }
        };
        
        window.addEventListener('mousemove', handleMouseMove);
        window.addEventListener('mouseup', handleMouseUp);
        window.addEventListener('mouseleave', handleMouseUp);
        
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
        
        const handleTouchMove = (event) => {
            if (!this.isDragging || event.touches.length !== 1) return;
            
            const deltaX = event.touches[0].clientX - this.lastX;
            const deltaY = event.touches[0].clientY - this.lastY;
            this.lastX = event.touches[0].clientX;
            this.lastY = event.touches[0].clientY;
            
            if (deltaX !== 0 || deltaY !== 0) {
                this.callModule('update_rotation', deltaX * 0.5, deltaY * 0.5);
            }
            
            event.preventDefault();
        };
        
        const handleTouchEnd = () => {
            this.isDragging = false;
        };
        
        this.canvas.addEventListener('touchmove', handleTouchMove);
        this.canvas.addEventListener('touchend', handleTouchEnd);
        this.canvas.addEventListener('touchcancel', handleTouchEnd);
        
        // Store event handlers for cleanup
        this._canvasEventHandlers = {
            mousemove: handleMouseMove,
            mouseup: handleMouseUp,
            mouseleave: handleMouseUp,
            touchmove: handleTouchMove,
            touchend: handleTouchEnd,
            touchcancel: handleTouchEnd
        };
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
        const brightness = parseInt(event.target.value);
        this.brightnessValue.textContent = brightness;
        
        // Convert from percentage (0-100) to byte (0-255)
        const scaledBrightness = Math.round((brightness * 255) / 100);
        this.callModule('set_brightness', scaledBrightness);
    }
    
    /**
     * Handle LED size slider change
     * @param {Event} event - Input event
     */
    handleLEDSizeChange(event) {
        const size = parseFloat(event.target.value);
        this.ledSizeValue.textContent = size.toFixed(1);
        this.callModule('set_led_size', size);
    }
    
    /**
     * Handle atmosphere intensity slider change
     * @param {Event} event - Input event
     */
    handleAtmosphereChange(event) {
        const value = parseInt(event.target.value);
        const intensity = value / 10.0; // Convert from 0-30 to 0-3.0
        this.atmosphereValue.textContent = intensity.toFixed(1);
        this.callModule('set_atmosphere_intensity', intensity);
    }
    
    /**
     * Handle mesh opacity slider change
     * @param {Event} event - Input event
     */
    handleMeshOpacityChange(event) {
        const opacity = parseFloat(event.target.value);
        this.meshOpacityValue.textContent = opacity.toFixed(1);
        this.callModule('set_mesh_opacity', opacity);
    }
    
    /**
     * Handle show mesh checkbox change
     * @param {Event} event - Input event
     */
    handleShowMeshChange(event) {
        const show = event.target.checked;
        this.callModule('set_show_mesh', show ? 1 : 0);
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
     * Update just the scene indicator in the UI
     */
    updateSceneIndicator() {
        // Update scene indicator (e.g., "1/8")
        if (this.sceneIndicator) {
            this.sceneIndicator.textContent = `${this.currentSceneIndex + 1}/${this.totalScenes}`;
        }
        
        // Update selected scene in dropdown
        if (this.sceneSelect) {
            this.sceneSelect.value = this.currentSceneIndex;
        }
    }
    
    /**
     * Show scene selection dropdown
     */
    showSceneDropdown(event) {
        console.log('Showing scene dropdown');
        
        // Prevent event from bubbling up
        if (event) {
            event.stopPropagation();
        }
        
        // Remove any existing dropdown
        const existingDropdown = document.querySelector('.scene-dropdown');
        if (existingDropdown) {
            console.log('Removing existing dropdown');
            existingDropdown.remove();
            return;
        }
        
        // Create dropdown container
        const dropdown = document.createElement('div');
        dropdown.className = 'scene-dropdown';
        
        // Create scene list
        const sceneList = document.createElement('div');
        sceneList.className = 'scene-list';
        
        console.log('Available scenes:', this.sceneNames);
        
        // Add scenes to the list
        this.sceneNames.forEach((name, index) => {
            const sceneItem = document.createElement('div');
            sceneItem.className = 'scene-item';
            if (index === this.currentSceneIndex) {
                sceneItem.classList.add('active');
            }
            sceneItem.textContent = name;
            
            // Add click handler
            sceneItem.addEventListener('click', async (e) => {
                console.log(`Clicked scene: ${name} (index: ${index})`);
                e.stopPropagation(); // Prevent event from bubbling up
                
                if (index !== this.currentSceneIndex && !this.isChangingScene) {
                    await this.changeScene(index);
                }
                dropdown.remove();
            });
            
            sceneList.appendChild(sceneItem);
        });
        
        dropdown.appendChild(sceneList);
        
        // Position the dropdown relative to the scene selector
        const rect = this.sceneSelector.getBoundingClientRect();
        const parentRect = this.sceneSelector.offsetParent.getBoundingClientRect();
        
        dropdown.style.position = 'absolute';
        dropdown.style.top = `${rect.bottom - parentRect.top}px`;
        dropdown.style.left = `${rect.left - parentRect.left}px`;
        dropdown.style.width = `${rect.width}px`;
        
        console.log('Positioning dropdown:', {
            top: `${rect.bottom - parentRect.top}px`,
            left: `${rect.left - parentRect.left}px`,
            width: `${rect.width}px`
        });
        
        // Add the dropdown to the scene selector's parent
        this.sceneSelector.offsetParent.appendChild(dropdown);
        
        // Prevent clicks on the dropdown from closing it
        dropdown.addEventListener('click', (e) => {
            e.stopPropagation();
        });
        
        // Close dropdown when clicking outside
        const closeDropdown = (event) => {
            if (!dropdown.contains(event.target) && !this.sceneSelector.contains(event.target)) {
                console.log('Closing dropdown (clicked outside)');
                dropdown.remove();
                document.removeEventListener('click', closeDropdown);
            }
        };
        
        // Delay adding the click listener to prevent immediate closure
        requestAnimationFrame(() => {
            document.addEventListener('click', closeDropdown);
        });
    }
    
    /**
     * Update scene parameters UI based on the selected scene
     * @param {number} sceneIndex - Index of the selected scene
     * @returns {Promise<void>} Promise that resolves when parameters are updated
     */
    async updateSceneParameters(sceneIndex) {
        this.sceneParams.innerHTML = '';
        
        if (!this.moduleReady) {
            return;
        }

        try {
            const parameters = Module.getSceneParameters();
            
            if (!parameters || parameters.length === 0) {
                const noParamsMsg = document.createElement('div');
                noParamsMsg.className = 'no-params-message';
                noParamsMsg.textContent = 'This scene has no adjustable parameters.';
                this.sceneParams.appendChild(noParamsMsg);
                return;
            }
            
            // Get stored parameters for this scene
            const storedParams = this.sceneParameters[sceneIndex] || {};
            
            // Create UI controls for each parameter
            for (const param of parameters) {
                if (storedParams[param.id] !== undefined) {
                    param.value = storedParams[param.id];
                    await this.handleSceneParameterChange(param.id, param.value);
                }
                this.addSceneParameter(param);
            }
        } catch (error) {
            console.error("Error updating scene parameters:", error);
            const errorMsg = document.createElement('div');
            errorMsg.className = 'error-message';
            errorMsg.textContent = 'Error loading scene parameters.';
            this.sceneParams.appendChild(errorMsg);
        }
    }
    
    /**
     * Add a parameter control to the scene parameters UI
     * @param {Object} param - Parameter object
     */
    addSceneParameter(param) {
        const paramRow = document.createElement('div');
        paramRow.className = 'param-row';
        
        const paramLabel = document.createElement('label');
        paramLabel.textContent = param.label;
        paramRow.appendChild(paramLabel);
        
        let input;
        let valueDisplay;
        
        // Determine if this is an integer parameter based on the parameter type
        const isInteger = param.controlType === 'slider' && (
            param.type === 'count' || // Count type is always integer
            (Number.isInteger(parseFloat(param.value)) && 
             Number.isInteger(param.min) && 
             Number.isInteger(param.max))
        );
        
        switch (param.controlType) {
            case 'slider':
                // Create slider
                input = document.createElement('input');
                input.type = 'range';
                input.min = param.min;
                input.max = param.max;
                
                // Use the step value provided by the C++ code
                input.step = param.step;
                
                input.value = parseFloat(param.value);
                input.id = `scene-param-${param.id}`;
                
                // Create value display
                valueDisplay = document.createElement('span');
                valueDisplay.className = 'param-value';
                
                // Format value display based on parameter type
                const formatValue = (val, paramType) => {
                    if (paramType === 'count') {
                        return Math.round(val);
                    }
                    if (paramType === 'ratio' || paramType === 'signed_ratio') {
                        return parseFloat(val).toFixed(3);  // Show 3 decimal places for ratios
                    }
                    if (paramType === 'angle' || paramType === 'signed_angle') {
                        return parseFloat(val).toFixed(3);  // Show 3 decimal places for angles
                    }
                    if (paramType === 'range') {
                        return parseFloat(val).toFixed(3);  // Show 3 decimal places for ranges
                    }
                    return val;
                };
                
                valueDisplay.textContent = formatValue(param.value, param.type);
                
                // Add event listener
                input.addEventListener('input', (e) => {
                    const newValue = parseFloat(e.target.value);
                    
                    // Format display value based on parameter type
                    valueDisplay.textContent = formatValue(newValue, param.type);
                    
                    // Log the parameter change
                    console.log(`Scene parameter changed: ${param.id} (${param.type}), value: ${newValue}`);
                    
                    // Send the raw value string to preserve decimal precision
                    this.handleSceneParameterChange(param.id, e.target.value);
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
        
        // Store the parameter value for the current scene
        if (!this.sceneParameters[this.currentSceneIndex]) {
            this.sceneParameters[this.currentSceneIndex] = {};
        }
        this.sceneParameters[this.currentSceneIndex][paramId] = value;
        
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

    // Add cleanup method
    cleanup() {
        // Remove window event listeners
        if (this._canvasEventHandlers) {
            window.removeEventListener('mousemove', this._canvasEventHandlers.mousemove);
            window.removeEventListener('mouseup', this._canvasEventHandlers.mouseup);
            window.removeEventListener('mouseleave', this._canvasEventHandlers.mouseleave);
            
            if (this.canvas) {
                this.canvas.removeEventListener('touchmove', this._canvasEventHandlers.touchmove);
                this.canvas.removeEventListener('touchend', this._canvasEventHandlers.touchend);
                this.canvas.removeEventListener('touchcancel', this._canvasEventHandlers.touchcancel);
            }
        }
        
        // Clear update interval
        if (this.updateIntervalId) {
            clearInterval(this.updateIntervalId);
            this.updateIntervalId = null;
        }
    }
}

// Initialize the simulator when the document is ready
document.addEventListener('DOMContentLoaded', () => {
    // Clean up any existing instance
    if (window.simulator) {
        window.simulator.cleanup();
    }
    
    window.simulator = new DodecaSimulator();
    window.simulator.initialize();
    
    // Add cleanup on page unload
    window.addEventListener('unload', () => {
        if (window.simulator) {
            window.simulator.cleanup();
            window.simulator = null;
        }
    });
}); 