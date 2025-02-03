/**
 * PixelTheater Documentation JavaScript
 */

// Add language classes to code blocks based on fenced code block info
document.addEventListener('DOMContentLoaded', () => {
    // Look for code blocks with language hints in comments
    document.querySelectorAll('pre code').forEach(block => {
        // Check if the first line contains a language hint
        const lines = block.textContent.split('\n');
        if (lines.length > 0) {
            const firstLine = lines[0].trim();
            
            // Check for language hints like ```yaml, ```cpp, etc.
            if (firstLine.startsWith('```')) {
                const lang = firstLine.replace('```', '').trim();
                if (lang) {
                    // Add the language class
                    block.classList.add(`language-${lang}`);
                    
                    // Remove the language hint line
                    block.textContent = lines.slice(1).join('\n');
                }
            }
        }
    });
    
    // Handle mobile navigation toggle
    const navToggle = document.querySelector('.nav-toggle');
    if (navToggle) {
        navToggle.addEventListener('click', () => {
            document.querySelector('.nav-content ul').classList.toggle('show');
        });
    }
    
    // Add smooth scrolling for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            e.preventDefault();
            
            const targetId = this.getAttribute('href');
            const targetElement = document.querySelector(targetId);
            
            if (targetElement) {
                targetElement.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
                
                // Update URL hash without jumping
                history.pushState(null, null, targetId);
            }
        });
    });
}); 