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
        this.sceneList = []; // NEW: Stores { index, name } for dropdown
        this.isChangingScene = false;
        this.setupRetries = 0;  // Counter for scene setup retry attempts
        
        // Store parameter values for each scene
        this.sceneParameters = {};
        
        // Ensure canvas is square
        this.resizeCanvas();
        window.addEventListener('resize', () => this.resizeCanvas());
        
        // Set up external C function implementations BEFORE module initialization
        this.setupExternalFunctions();

        // Ensure these match your HTML structure for the custom dropdown
        this.sceneSelect = null; // No longer using <select> by default
        this.sceneSelector = document.querySelector('.scene-selector'); // The clickable area
        this.selectedSceneText = document.querySelector('.selected-scene'); // The text displaying current scene
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
                console.log("Module runtime was initialized");
                
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
    async onModuleReady() {
        console.log("Module is ready, initializing simulator");
        
        this.resizeCanvas();
        this.setupExternalFunctions();
        this.hasCAPI = this.checkCAPIFunctions();
        this.listAvailableModuleFunctions();
        
        console.log("Calling _init_simulator...");
        try {
            const initSuccess = this.callModule('init_simulator');
            if (!initSuccess) {
                throw new Error("_init_simulator returned false.");
            }
            console.log("_init_simulator call succeeded.");
        } catch (error) {
            console.error("Error calling _init_simulator:", error);
            return; 
        }

        // Populate the scene list UI after init is done
        await this.populateSceneList();
        
        this.setupControlValues(); 
        this.setupEventHandlers();
        this.setupCanvasInteraction();
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
     * Fetches metadata for all scenes, populates the internal list, and sets initial scene.
     */
    async populateSceneList() { 
        console.log("Populating scene list...");
        this.sceneList = [];
        // No longer interacting with sceneSelect element directly here

        try {
            this.totalScenes = this.callModule('get_num_scenes');
            if (this.totalScenes === null || this.totalScenes <= 0) {
                console.error("Could not get number of scenes.");
                return;
            }
            console.log(`Found ${this.totalScenes} scenes.`);

            // --- ADDED: Workaround for TestScene initial name ---
            if (this.totalScenes > 0) {
                 console.log("Pre-running scene 0 setup...");
                 this.callModule('change_scene', 0);
                 await new Promise(resolve => requestAnimationFrame(resolve)); 
                 console.log("Pre-run finished.");
            }
            // --- END WORKAROUND ---

            for (let i = 0; i < this.totalScenes; i++) {
                console.log(`Fetching metadata for scene index ${i}...`);
                
                // Change scene to ensure setup() runs for metadata fetch
                this.callModule('change_scene', i); 
                await new Promise(resolve => requestAnimationFrame(resolve)); 

                // Fetch metadata
                let metadata = {};
                let jsonPtr = 0;
                try {
                    jsonPtr = this.callModule('get_current_scene_metadata_json');
                    if (!jsonPtr) throw new Error("Metadata JSON pointer is null");
                    const metaJsonString = Module.UTF8ToString(jsonPtr);
                    if (!metaJsonString || metaJsonString.trim() === "") throw new Error("Received empty metadata JSON string.");
                    console.log(`  [Scene ${i}] Metadata JSON: ${metaJsonString}`);
                    metadata = JSON.parse(metaJsonString);
                } catch (error) {
                     console.error(`  [Scene ${i}] Error fetching/parsing metadata:`, error);
                     metadata = { name: `Scene ${i} (Error)` }; 
                } finally {
                     if (jsonPtr) { 
                          console.log(`  [Scene ${i}] Attempting to free metadata pointer: ${jsonPtr}`);
                          this.callModule('free_string_memory', jsonPtr);
                     }
                }

                // Store info in the list, don't update UI options here
                const sceneInfo = { index: i, name: metadata.name || `Scene ${i}` };
                this.sceneList.push(sceneInfo);

                // REMOVED: const option = ... 
                // REMOVED: this.sceneSelect.appendChild(option); 
            }

            console.log("Finished fetching all scene metadata for list.");

            // Set the final desired initial scene AFTER the loop completes
            const initialSceneIndex = 2; 
            if (initialSceneIndex >= 0 && initialSceneIndex < this.totalScenes) {
                console.log(`Setting initial displayed scene to index ${initialSceneIndex}...`);
                await this.changeScene(initialSceneIndex); 
                console.log("Initial scene set.");
            } else { 
                 console.warn(`Initial scene index ${initialSceneIndex} is invalid. Defaulting to 0.`);
                 if(this.totalScenes > 0) await this.changeScene(0); 
            }

        } catch (error) { 
             console.error("Error populating scene list:", error);
        } finally { 
             console.log("populateSceneList finished."); 
        }
    }
    
    /**
     * Updates UI elements with current scene's metadata (The main display text)
     * @param {object} metadata - Parsed metadata object { name, description, ... }
     */
    updateSceneDisplay(metadata) {
         console.log("Updating scene display with metadata:", metadata);
         
         // Update the main scene name text display element
         if (this.selectedSceneText) { 
             this.selectedSceneText.textContent = metadata.name || 'Unknown Scene';
             console.log(`Updated selectedSceneText to: ${this.selectedSceneText.textContent}`);
         } else {
              console.warn("selectedSceneText element not found to update.");
         }
         
         // No longer need to update sceneSelect.value here
    }
    
    /**
     * Change scene, fetch and update metadata and parameters.
     * @param {number} sceneIndex - Index of scene to change to
     * @returns {Promise<void>} Promise that resolves when scene change is complete
     */
    async changeScene(sceneIndex) {
        if (sceneIndex === this.currentSceneIndex && this.currentSceneIndex !== -1) {
             console.log(`Already on scene ${sceneIndex}.`);
             return; // Already on this scene
        }

        if (this.isChangingScene) {
            console.warn("changeScene called while already changing scene.");
            return;
        }
        this.isChangingScene = true;
        console.log(`Attempting to change scene to index: ${sceneIndex}`);

        try {
            // 1. Change scene in C++
            const result = this.callModule('change_scene', sceneIndex);
            if (result === null) { // change_scene might not return a useful value, check logs
                console.warn('callModule change_scene returned null, proceeding...');
                 // Consider adding a check or delay if issues persist
            }
            console.log(`C++ change_scene(${sceneIndex}) called.`);

            // 2. Update internal index immediately
            this.currentSceneIndex = sceneIndex;

            // 3. Wait for C++ scene setup to potentially complete
            // (May not be strictly necessary if calls are synchronous enough, but safer)
            await new Promise(resolve => requestAnimationFrame(resolve)); 
            console.log("Waited for next frame.");

            // 4. Fetch and update Metadata Display
            let metadata = {};
            let metaPtr = 0;
            try {
                metaPtr = this.callModule('get_current_scene_metadata_json');
                if (!metaPtr) throw new Error("Metadata JSON pointer is null");
                const metaJsonString = Module.UTF8ToString(metaPtr);
                console.log(`Metadata JSON received: ${metaJsonString}`);
                metadata = JSON.parse(metaJsonString);
            } catch (error) {
                console.error(`Error fetching/parsing metadata for scene ${sceneIndex}:`, error);
                metadata = { name: `Scene ${sceneIndex} (Error)` }; // Fallback
            } finally {
                if (metaPtr) this.callModule('free_string_memory', metaPtr);
            }
            this.updateSceneDisplay(metadata); // Update UI text/dropdown

            // 5. Fetch and update Parameter Controls
            let parameters = [];
            let paramsPtr = 0;
            try {
                 paramsPtr = this.callModule('get_scene_parameters_json');
                 if (!paramsPtr) throw new Error("Parameters JSON pointer is null");
                 const paramsJsonString = Module.UTF8ToString(paramsPtr);
                 console.log(`Parameters JSON received: ${paramsJsonString}`);
                 parameters = JSON.parse(paramsJsonString);
            } catch(error) {
                 console.error(`Error fetching/parsing parameters for scene ${sceneIndex}:`, error);
                 parameters = []; // Set empty on error
            } finally {
                 if (paramsPtr) this.callModule('free_string_memory', paramsPtr);
            }
            this.updateSceneParameterControls(parameters); // Update sliders etc.

            // 6. Update other UI indicators (like "2/4")
            this.updateSceneIndicator();

        } catch (error) {
            console.error(`Error during changeScene(${sceneIndex}):`, error);
            // Potentially revert UI changes or show error to user
        } finally {
            this.isChangingScene = false;
            console.log(`changeScene(${sceneIndex}) finished, isChangingScene set to false`);
        }
    }
    
    /**
     * Update scene parameters UI based on the selected scene
     * @param {Array<object>} parameters - Array of parameter objects from parsed JSON
     */
    updateSceneParameterControls(parameters) {
        this.sceneParams.innerHTML = ''; // Clear previous params

        if (!this.moduleReady || typeof Module === 'undefined') {
            console.warn("Module not ready, skipping parameter update.");
            return;
        }

        console.log(`Updating parameter controls for scene ${this.currentSceneIndex}`);

        try {
            if (!parameters || parameters.length === 0) {
                 // Handle empty array after successful parse
                 const noParamsMsg = document.createElement('div');
                 noParamsMsg.className = 'no-params-message';
                 noParamsMsg.textContent = 'This scene has no adjustable parameters.';
                 this.sceneParams.appendChild(noParamsMsg);
                 console.log("No parameters to display for this scene.");
                 return;
            }

            // Get stored parameters for this scene
            const storedParams = this.sceneParameters[this.currentSceneIndex] || {};

            // Create UI controls for each parameter
            parameters.forEach(param => {
                // Use stored value if available
                if (storedParams[param.id] !== undefined) {
                    console.log(`Using stored value for ${param.id}: ${storedParams[param.id]}`);
                    param.value = storedParams[param.id]; 

                    // Update C++ side immediately if needed
                    try {
                        this.handleSceneParameterChange(param.id, param.value.toString());
                    } catch (e) {
                        console.error(`Error re-applying stored parameter ${param.id}:`, e);
                    }
                } else {
                     // If no stored value, maybe ensure C++ has the default from JSON?
                     // Could call handleSceneParameterChange here too, but might be redundant
                     // if C++ already has the correct default from its schema.
                }
                this.addSceneParameter(param); // Use existing function to add controls
            });

        } catch (error) {
            console.error("Error updating scene parameter controls:", error);
            // Show error message in UI
            const errorMsg = document.createElement('div');
            errorMsg.className = 'error-message';
            errorMsg.textContent = 'Error loading scene parameters.';
            this.sceneParams.appendChild(errorMsg);
        } 
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
        // --- Restore click listener for the custom dropdown trigger ---
        if (this.sceneSelector) {
             // Remove listener first to prevent duplicates
             if (this._handleSceneSelectorClick) {
                  this.sceneSelector.removeEventListener('click', this._handleSceneSelectorClick);
             }
             // Store handler on instance for removal later if needed
             this._handleSceneSelectorClick = (e) => this.showSceneDropdown(e);
             this.sceneSelector.addEventListener('click', this._handleSceneSelectorClick);
             console.log("Click listener added to scene selector div (.scene-selector).");
        } else {
             console.warn("Scene selector element (.scene-selector) not found for event handler setup.");
        }
        // --- End Restore click listener ---

        // REMOVED: Listener setup for this.sceneSelect <select> element

        // --- Keep existing listeners ---
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
        this.prevButton.addEventListener('click', () => this.navigateScene(-1));
        this.nextButton.addEventListener('click', () => this.navigateScene(1));
        
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
        if (this.sceneIndicator && this.totalScenes > 0 && this.currentSceneIndex >= 0) {
            this.sceneIndicator.textContent = `${this.currentSceneIndex + 1}/${this.totalScenes}`;
        } else if (this.sceneIndicator) {
             this.sceneIndicator.textContent = `-/${this.totalScenes || '?'}`; // Indicate invalid state
        }
        // No longer need to update sceneSelect value here
    }
    
    /**
     * Show scene selection dropdown (Restored and simplified to use CSS classes)
     */
    showSceneDropdown(event) {
        console.log('Showing scene dropdown (custom div - CSS positioned)');

        if (event) event.stopPropagation();

        const existingDropdown = document.querySelector('.scene-dropdown');
        if (existingDropdown) {
            console.log('Removing existing dropdown');
            existingDropdown.remove();
            if (this._closeDropdownHandler) {
                 document.removeEventListener('click', this._closeDropdownHandler);
                 this._closeDropdownHandler = null;
            }
            return;
        }

        const dropdown = document.createElement('div');
        dropdown.className = 'scene-dropdown'; // Apply the main CSS class

        const sceneListDiv = document.createElement('div');
        sceneListDiv.className = 'scene-list'; // Apply the list container class

        console.log('Populating dropdown with scenes:', this.sceneList);

        // Use internal this.sceneList
        this.sceneList.forEach((sceneInfo) => {
            const sceneItem = document.createElement('div');
            sceneItem.className = 'scene-item'; // Apply list item class
            if (sceneInfo.index === this.currentSceneIndex) {
                sceneItem.classList.add('active'); // Mark active item
            }
            sceneItem.textContent = sceneInfo.name;

            sceneItem.addEventListener('click', async (e) => {
                console.log(`Clicked scene: ${sceneInfo.name} (index: ${sceneInfo.index})`);
                e.stopPropagation();

                if (sceneInfo.index !== this.currentSceneIndex && !this.isChangingScene) {
                    await this.changeScene(sceneInfo.index);
                }
                dropdown.remove(); // Close dropdown
                if (this._closeDropdownHandler) {
                    document.removeEventListener('click', this._closeDropdownHandler);
                    this._closeDropdownHandler = null;
                }
            });
            sceneListDiv.appendChild(sceneItem);
        });

        dropdown.appendChild(sceneListDiv);

        // Append the dropdown relative to the selector's parent
        // CSS (.scene-dropdown { position: absolute; ... }) will handle placement.
        if (!this.sceneSelector || !this.sceneSelector.offsetParent) {
             console.error("Cannot append dropdown, sceneSelector or its offsetParent not found.");
             return;
        }
        // Append to the same parent container as the selector
        this.sceneSelector.appendChild(dropdown);
        console.log("Appended .scene-dropdown; CSS should position it.");

        // Keep click handlers for closing
        dropdown.addEventListener('click', (e) => e.stopPropagation());

        this._closeDropdownHandler = (closeEvent) => {
            // Check if click is outside dropdown AND outside the trigger button
            if (!dropdown.contains(closeEvent.target) && !this.sceneSelector.contains(closeEvent.target)) {
                console.log('Closing dropdown (clicked outside)');
                dropdown.remove();
                document.removeEventListener('click', this._closeDropdownHandler);
                this._closeDropdownHandler = null;
            }
        };
        // Use requestAnimationFrame to ensure the click handler is added *after* the current event bubble phase
        requestAnimationFrame(() => {
            document.addEventListener('click', this._closeDropdownHandler);
        });
    }
    
    /**
     * Navigate to previous or next scene
     * @param {number} direction - Direction to navigate (-1 for prev, 1 for next)
     */
    async navigateScene(direction) {
        if (this.isChangingScene) {
            console.warn("NavigateScene ignored: Scene change already in progress.");
            return;
        }

        try {
            // Ensure totalScenes is valid before calculating new index
            if (this.totalScenes <= 0) {
                 console.error("Cannot navigate, totalScenes is not valid.");
                 return;
            }
            const newIndex = (this.currentSceneIndex + direction + this.totalScenes) % this.totalScenes;
            
            await this.changeScene(newIndex);
            
        } catch (error) {
            console.error("Error navigating scene:", error);
        }
    }
    
    /**
     * Add a parameter control to the scene parameters UI
     * @param {Object} param - Parameter object from Embind
     */
    addSceneParameter(param) {
        const paramRow = document.createElement('div');
        paramRow.className = 'param-row';
        
        const paramLabel = document.createElement('label');
        // Use param.id as label for now, could be param.label if available
        paramLabel.textContent = param.label || param.id;
        paramRow.appendChild(paramLabel);
        
        let input;
        let valueDisplay;
        
        // Helper function to format displayed values
        const formatValue = (valStr, paramType) => {
            const val = parseFloat(valStr); // Convert string value to number for formatting
            if (paramType === 'count') {
                return Math.round(val);
            }
            // For float types, show appropriate precision
            if (paramType === 'ratio' || paramType === 'signed_ratio' || 
                paramType === 'angle' || paramType === 'signed_angle' ||
                paramType === 'range') {
                // Adjust precision based on step or range if needed
                return val.toFixed(3); 
            }
            // Fallback for unknown types or boolean strings
            return valStr;
        };
        
        switch (param.controlType) {
            case 'slider':
                // Create slider
                input = document.createElement('input');
                input.type = 'range';
                input.min = param.min;
                input.max = param.max;
                input.step = param.step;
                input.value = parseFloat(param.value); // Set initial slider position
                input.id = `scene-param-${param.id}`;
                
                // Create value display
                valueDisplay = document.createElement('span');
                valueDisplay.className = 'param-value';
                valueDisplay.textContent = formatValue(param.value, param.type);
                
                // Add event listener
                input.addEventListener('input', (e) => {
                    const newValueStr = e.target.value; // Value from slider is always a string
                    valueDisplay.textContent = formatValue(newValueStr, param.type);
                    
                    // Send the raw value string back to C++
                    this.handleSceneParameterChange(param.id, newValueStr);
                });
                
                paramRow.appendChild(input);
                paramRow.appendChild(valueDisplay);
                break;
                
            case 'checkbox':
                // Create checkbox
                input = document.createElement('input');
                input.type = 'checkbox';
                // Value from C++ for bool is likely "true" or "false" string
                input.checked = (param.value.toLowerCase() === "true"); 
                input.id = `scene-param-${param.id}`;
                
                // Re-create label structure to wrap checkbox
                paramLabel.htmlFor = input.id; // Associate label with input
                paramRow.innerHTML = ''; // Clear previous label
                paramRow.appendChild(input);
                paramRow.appendChild(paramLabel);
                paramLabel.textContent = param.label || param.id; // Set text again

                // Add event listener
                input.addEventListener('change', (e) => {
                    this.handleSceneParameterChange(param.id, e.target.checked.toString());
                });
                
                break;
                
            case 'select':
                // Create select dropdown
                input = document.createElement('select');
                input.id = `scene-param-${param.id}`;
                
                // Add options (param.options should be an array of strings)
                if (param.options && Array.isArray(param.options)) {
                    param.options.forEach(option => {
                        const optionEl = document.createElement('option');
                        optionEl.value = option;
                        optionEl.textContent = option;
                        // param.value is the current selection string from C++
                        optionEl.selected = (option === param.value); 
                        input.appendChild(optionEl);
                    });
                }
                
                // Add event listener
                input.addEventListener('change', (e) => {
                    this.handleSceneParameterChange(param.id, e.target.value);
                });
                
                paramRow.appendChild(input);
                break;

             default:
                 // Handle unknown control types
                 const unknownMsg = document.createElement('span');
                 unknownMsg.textContent = `Unknown control: ${param.controlType}`;
                 paramRow.appendChild(unknownMsg);
                 break;
        }
        
        this.sceneParams.appendChild(paramRow);
    }

    /**
     * Handle scene parameter change
     * @param {string} paramId - Parameter ID
     * @param {string} value - New value (always as a string)
     */
    handleSceneParameterChange(paramId, value) {
        console.log(`Scene parameter changed: ${paramId}, value: ${value}`);
        
        if (!this.moduleReady || typeof Module === 'undefined') {
            console.warn("Module not ready, can't update parameter");
            return;
        }
        
        // Store the parameter value (as string) for the current scene
        if (!this.sceneParameters[this.currentSceneIndex]) {
            this.sceneParameters[this.currentSceneIndex] = {};
        }
        this.sceneParameters[this.currentSceneIndex][paramId] = value;
        
        try {
            // Call the new C API function to update the parameter
            // We need to pass strings
            // Module._update_scene_parameter_string(paramId, value.toString()); 
            
            // --- MODIFIED: Use ccall for explicit type handling --- 
            Module.ccall(
                'update_scene_parameter_string', // C function name (without underscore)
                null,                           // Return type (null for void)
                ['string', 'string'],           // Argument types
                [paramId, value.toString()]     // Arguments
            );
            // --- END MODIFIED --- 
            
        } catch (error) {
            console.error(`Error updating parameter ${paramId} via C API:`, error);
        }
    }
    
    /**
     * Check if the C API functions for JSON are available
     * @returns {boolean} True if functions are available
     */
    checkCAPIFunctions() { // Renamed from checkEmbindFunctions
        let available = true;
        if (typeof Module._get_scene_parameters_json !== 'function') {
            console.warn("Module._get_scene_parameters_json is not available");
            available = false;
        }
        if (typeof Module._update_scene_parameter_string !== 'function') {
            console.warn("Module._update_scene_parameter_string is not available");
            available = false;
        }
         if (typeof Module._free_string_memory !== 'function') {
            console.warn("Module._free_string_memory is not available");
            available = false;
        }
        
        if(available) console.log("C API JSON functions are available");
        return available;
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