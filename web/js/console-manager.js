/**
 * Console manager for the DodecaRGB web simulator
 * Handles console logging, redirection, and integration with the UI
 */
class ConsoleManager {
    constructor() {
        this.consoleElement = document.getElementById('console');
        this.consoleContainer = document.getElementById('console-container');
        this.toggleConsoleButton = document.getElementById('toggle-console');
        this.clearConsoleButton = document.getElementById('clear-console');
        
        // Store original console methods
        this.originalConsoleLog = console.log;
        this.originalConsoleError = console.error;
        this.originalConsoleWarn = console.warn;
        
        // Initialize
        this.setupConsoleRedirection();
        this.setupEventHandlers();
    }
    
    /**
     * Set up console UI event handlers
     */
    setupEventHandlers() {
        // Toggle console visibility
        if (this.toggleConsoleButton) {
            this.toggleConsoleButton.addEventListener('click', () => this.toggleConsole());
        }
        
        // Clear console
        if (this.clearConsoleButton) {
            this.clearConsoleButton.addEventListener('click', () => this.clearConsole());
        }
    }
    
    /**
     * Toggle console visibility
     */
    toggleConsole() {
        if (this.consoleContainer.style.display === 'none' || !this.consoleContainer.style.display) {
            this.consoleContainer.style.display = 'block';
            this.toggleConsoleButton.textContent = 'Hide Console';
        } else {
            this.consoleContainer.style.display = 'none';
            this.toggleConsoleButton.textContent = 'Show Console';
        }
    }
    
    /**
     * Clear console content
     */
    clearConsole() {
        if (this.consoleElement) {
            this.consoleElement.innerHTML = '';
        }
    }
    
    /**
     * Get formatted timestamp for console messages
     * @returns {string} Formatted timestamp [HH:MM:SS.mmm]
     */
    getTimestamp() {
        const now = new Date();
        return `[${now.getHours().toString().padStart(2, '0')}:${
            now.getMinutes().toString().padStart(2, '0')}:${
            now.getSeconds().toString().padStart(2, '0')}.${
            now.getMilliseconds().toString().padStart(3, '0')}]`;
    }
    
    /**
     * Format arguments for console output
     * @param {Array} args - Arguments to format
     * @returns {string} Formatted message
     */
    formatArgs(args) {
        return args.map(arg => {
            if (typeof arg === 'object') {
                return JSON.stringify(arg);
            } else {
                return arg;
            }
        }).join(' ');
    }
    
    /**
     * Set up redirection for console methods
     */
    setupConsoleRedirection() {
        if (!this.consoleElement) return;
        
        // Override console.log
        console.log = (...args) => {
            this.originalConsoleLog.apply(console, args);
            
            const message = this.formatArgs(args);
            const timestamp = this.getTimestamp();
            this.consoleElement.innerHTML += `<span style="color: #aaaaaa;">${timestamp}</span> ${message}\n`;
            this.scrollToBottom();
        };
        
        // Override console.error
        console.error = (...args) => {
            this.originalConsoleError.apply(console, args);
            
            const message = this.formatArgs(args);
            const timestamp = this.getTimestamp();
            this.consoleElement.innerHTML += `<span style="color: #aaaaaa;">${timestamp}</span> <span class="error">${message}</span>\n`;
            this.scrollToBottom();
        };
        
        // Override console.warn
        console.warn = (...args) => {
            this.originalConsoleWarn.apply(console, args);
            
            const message = this.formatArgs(args);
            const timestamp = this.getTimestamp();
            this.consoleElement.innerHTML += `<span style="color: #aaaaaa;">${timestamp}</span> <span class="warning">${message}</span>\n`;
            this.scrollToBottom();
        };
    }
    
    /**
     * Scroll console to the bottom
     */
    scrollToBottom() {
        if (this.consoleElement) {
            this.consoleElement.scrollTop = this.consoleElement.scrollHeight;
        }
    }
}

// Initialize the console manager when the document is ready
document.addEventListener('DOMContentLoaded', () => {
    window.consoleManager = new ConsoleManager();
}); 