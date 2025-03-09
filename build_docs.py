import shutil
import os
import subprocess
from pathlib import Path
import markdown
import yaml
from jinja2 import Environment, FileSystemLoader
import re
import argparse

class DocBuilder:
    def __init__(self, skip_simulator_build=False):
        self.docs_dir = Path('docs')
        self.site_dir = Path('build/docs')
        self.template_dir = self.docs_dir / '_templates'
        self.simulator_dir = self.site_dir / 'simulator'
        self.project_root = Path('.')
        self.skip_simulator_build = skip_simulator_build
        
        # Setup Jinja2
        self.env = Environment(
            loader=FileSystemLoader(str(self.template_dir))
        )

    def build(self):
        # Clean and create site directory
        if self.site_dir.exists():
            shutil.rmtree(self.site_dir)
        self.site_dir.mkdir(parents=True, exist_ok=True)

        # Copy static assets
        if (self.docs_dir / 'assets').exists():
            shutil.copytree(
                self.docs_dir / 'assets', 
                self.site_dir / 'assets'
            )
            
        # Ensure JS directory exists
        js_dir = self.site_dir / 'assets' / 'js'
        js_dir.mkdir(parents=True, exist_ok=True)
            
        # Copy project images to assets/images
        self.copy_project_images()

        # Collect all markdown files for index generation
        markdown_files = []
        for md_file in self.docs_dir.rglob('*.md'):
            # Skip template directory and index.md
            if '_templates' in md_file.parts or md_file.name == 'index.md':
                continue
            
            # Add to list for index generation
            markdown_files.append(md_file)
        
        # Generate navigation data for templates
        nav_data = self.generate_navigation_data(markdown_files)

        # Generate index page using template
        self.generate_index_page(nav_data)
        
        # Process markdown files
        for md_file in self.docs_dir.rglob('*.md'):
            # Skip template directory
            if '_templates' in md_file.parts:
                continue

            # Calculate relative path
            rel_path = md_file.relative_to(self.docs_dir)
            html_path = self.site_dir / rel_path.with_suffix('.html')
            html_path.parent.mkdir(parents=True, exist_ok=True)

            # Convert markdown to HTML
            self.convert_markdown_file(md_file, html_path, nav_data)

        # Build web simulator
        try:
            self.build_web_simulator(nav_data)
        except Exception as e:
            print(f"Warning: Web simulator build failed: {e}")
            print("Documentation will be built without the web simulator.")
            
            # Create a placeholder page for the simulator
            self.create_simulator_placeholder(nav_data)

        print(f"Documentation built from {self.docs_dir} to {self.site_dir}")

    def copy_project_images(self):
        """Copy images from project root to the build directory"""
        # Create images directory in assets if it doesn't exist
        images_dir = self.site_dir / 'assets' / 'images'
        images_dir.mkdir(parents=True, exist_ok=True)
        
        # Copy images from project root /images directory if it exists
        project_images_dir = self.project_root / 'images'
        if project_images_dir.exists() and project_images_dir.is_dir():
            print(f"Copying project images from {project_images_dir} to {images_dir}")
            for img_file in project_images_dir.glob('**/*'):
                if img_file.is_file() and img_file.suffix.lower() in ['.jpg', '.jpeg', '.png', '.gif', '.svg']:
                    # Create relative path in target directory
                    rel_path = img_file.relative_to(project_images_dir)
                    target_path = images_dir / rel_path
                    
                    # Create parent directories if needed
                    target_path.parent.mkdir(parents=True, exist_ok=True)
                    
                    # Copy the file
                    shutil.copy2(img_file, target_path)
                    print(f"  Copied {rel_path}")
        else:
            print("No project images directory found at /images")
            
        # Also look for images in docs/images and copy them if they're not already in assets/images
        docs_images_dir = self.docs_dir / 'images'
        if docs_images_dir.exists() and docs_images_dir.is_dir():
            print(f"Copying documentation images from {docs_images_dir} to {images_dir}")
            for img_file in docs_images_dir.glob('**/*'):
                if img_file.is_file() and img_file.suffix.lower() in ['.jpg', '.jpeg', '.png', '.gif', '.svg']:
                    # Create relative path in target directory
                    rel_path = img_file.relative_to(docs_images_dir)
                    target_path = images_dir / rel_path
                    
                    # Create parent directories if needed
                    target_path.parent.mkdir(parents=True, exist_ok=True)
                    
                    # Copy the file
                    shutil.copy2(img_file, target_path)
                    print(f"  Copied {rel_path}")

    def generate_navigation_data(self, markdown_files):
        """Generate navigation data for templates"""
        # Check if TOC file exists
        toc_path = self.docs_dir / 'toc.yaml'
        if toc_path.exists():
            return self.generate_navigation_from_toc(toc_path, markdown_files)
        else:
            return self.generate_navigation_automatically(markdown_files)
    
    def generate_navigation_from_toc(self, toc_path, markdown_files):
        """Generate navigation data from TOC file"""
        # Load TOC file
        with open(toc_path, 'r') as f:
            toc_data = yaml.safe_load(f)
        
        # Create navigation data structure
        nav_data = {
            'main_nav': [],
            'sections': {}
        }
        
        # Process main navigation
        if 'main_nav' in toc_data:
            nav_data['main_nav'] = toc_data['main_nav']
        
        # Create a lookup dictionary for markdown files
        file_lookup = {}
        for md_file in markdown_files:
            rel_path = str(md_file.relative_to(self.docs_dir))
            file_lookup[rel_path] = {
                'path': rel_path,
                'url': '/' + rel_path.replace('.md', '.html'),
                'name': md_file.stem,
                'title': self.get_title_from_markdown(md_file) or md_file.stem.replace('_', ' ').title()
            }
        
        # Process sections
        if 'sections' in toc_data:
            for section in toc_data['sections']:
                section_title = section['title']
                nav_data['sections'][section_title] = []
                
                if 'items' in section:
                    for item in section['items']:
                        # If item has a direct URL and title, use those
                        if 'url' in item and 'title' in item:
                            nav_data['sections'][section_title].append(item)
                        # Otherwise, look up the file in our markdown files
                        elif 'file' in item:
                            file_path = item['file']
                            if file_path in file_lookup:
                                nav_item = file_lookup[file_path].copy()
                                # Allow overriding title in TOC
                                if 'title' in item:
                                    nav_item['title'] = item['title']
                                # Add special flag if specified
                                if 'is_special' in item:
                                    nav_item['is_special'] = item['is_special']
                                nav_data['sections'][section_title].append(nav_item)
                            else:
                                print(f"Warning: File {file_path} specified in TOC not found")
        
        return nav_data
    
    def generate_navigation_automatically(self, markdown_files):
        """Generate navigation data automatically from markdown files"""
        nav_data = {
            'main_nav': [],
            'sections': {}
        }
        
        # Add home link
        nav_data['main_nav'].append({
            'title': 'Home',
            'url': '/'
        })
        
        # Organize files by directory
        file_structure = {}
        for md_file in markdown_files:
            rel_path = md_file.relative_to(self.docs_dir)
            parent_dir = str(rel_path.parent)
            if parent_dir == '.':
                parent_dir = 'Root'
            
            if parent_dir not in file_structure:
                file_structure[parent_dir] = []
            
            # Get title from front matter if available
            title = self.get_title_from_markdown(md_file)
            
            file_structure[parent_dir].append({
                'path': str(rel_path),
                'url': '/' + str(rel_path).replace('.md', '.html'),
                'name': md_file.stem,
                'title': title or md_file.stem.replace('_', ' ').title()
            })
        
        # Sort files within each directory
        for dir_name, files in file_structure.items():
            file_structure[dir_name] = sorted(files, key=lambda x: x['title'])
        
        # Add main navigation items
        key_sections = ['guides', 'PixelTheater']
        for section in key_sections:
            if section in file_structure:
                # Find a good landing page for this section
                landing_page = next((f for f in file_structure[section] if f['name'] in ['index', 'getting-started', 'parameters']), 
                                   file_structure[section][0] if file_structure[section] else None)
                
                if landing_page:
                    nav_data['main_nav'].append({
                        'title': section.replace('_', ' ').title(),
                        'url': landing_page['url']
                    })
        
        # Add direct link to the live simulator
        nav_data['main_nav'].append({
            'title': 'Web Simulator',
            'url': '/simulator/index.html',
            'is_special': True
        })
        
        # Create sidebar sections
        for dir_name, files in sorted(file_structure.items()):
            if dir_name == 'Root':
                section_title = 'Main Documentation'
            else:
                section_title = dir_name.split('/')[-1].replace('_', ' ').title()
            
            nav_data['sections'][section_title] = files
        
        # Add special links to the sidebar
        if 'Special Links' not in nav_data['sections']:
            nav_data['sections']['Special Links'] = []
            
        nav_data['sections']['Special Links'].append({
            'title': 'Live Web Simulator',
            'url': '/simulator/index.html',
            'is_special': True
        })
        
        return nav_data

    def generate_index_page(self, nav_data):
        """Generate index page using template"""
        # Render template
        template = self.env.get_template('index_content.html')
        content = template.render(nav=nav_data)
        
        # Render page template with the content
        page_template = self.env.get_template('page.html')
        output = page_template.render(
            content=content,
            meta={'title': 'PixelTheater Documentation'},
            title='PixelTheater Documentation',
            nav=nav_data,
            current_path='/'
        )
        
        # Write output
        index_path = self.site_dir / 'index.html'
        index_path.write_text(output, encoding='utf-8')
        
        print(f"Generated index page with {sum(len(items) for items in nav_data['sections'].values())} documentation links")

    def get_title_from_markdown(self, md_path):
        """Extract title from markdown front matter"""
        content = md_path.read_text(encoding='utf-8')
        
        # Parse front matter if it exists
        if content.startswith('---'):
            end = content.find('---', 3)
            if end != -1:
                front_matter = content[3:end]
                try:
                    meta = yaml.safe_load(front_matter)
                    if meta and 'title' in meta:
                        return meta['title']
                except Exception:
                    pass
        
        # Try to find first heading if no front matter title
        lines = content.split('\n')
        for line in lines:
            if line.startswith('# '):
                return line[2:].strip()
        
        return None

    def convert_markdown_file(self, md_path, html_path, nav_data):
        # Read markdown content
        content = md_path.read_text(encoding='utf-8')
        
        # Parse front matter if it exists
        meta = {}
        if content.startswith('---'):
            end = content.find('---', 3)
            if end != -1:
                front_matter = content[3:end]
                meta = yaml.safe_load(front_matter)
                content = content[end + 4:]
        
        # Fix image paths - convert /images/ to /assets/images/
        content = self.fix_image_paths(content)

        # Convert markdown to HTML
        md = markdown.Markdown(extensions=['fenced_code', 'tables', 'codehilite'])
        html_content = md.convert(content)
        
        # Add language classes to code blocks
        html_content = self.enhance_code_blocks(html_content)

        # Render template
        template = self.env.get_template('page.html')
        output = template.render(
            content=html_content,
            meta=meta,
            title=meta.get('title', 'Documentation'),
            nav=nav_data,
            current_path='/' + str(md_path.relative_to(self.docs_dir)).replace('.md', '.html')
        )

        # Write output
        html_path.write_text(output, encoding='utf-8')
    
    def enhance_code_blocks(self, html_content):
        """Add language classes to code blocks for syntax highlighting"""
        import re
        
        # Pattern to find code blocks with language info
        pattern = r'<pre><code class="language-(\w+)">(.*?)</code></pre>'
        
        def replace_code_block(match):
            lang = match.group(1)
            code = match.group(2)
            return f'<pre><code class="language-{lang} hljs">{code}</code></pre>'
        
        # Replace code blocks with enhanced versions
        enhanced_html = re.sub(pattern, replace_code_block, html_content, flags=re.DOTALL)
        
        # Also handle code blocks without language info
        pattern_no_lang = r'<pre><code>(.*?)</code></pre>'
        
        def replace_no_lang_block(match):
            code = match.group(1)
            return f'<pre><code class="hljs">{code}</code></pre>'
        
        enhanced_html = re.sub(pattern_no_lang, replace_no_lang_block, enhanced_html, flags=re.DOTALL)
        
        return enhanced_html

    def fix_image_paths(self, content):
        """Fix image paths in markdown content"""
        # Replace image references from /images/ to /assets/images/
        content = re.sub(r'!\[(.*?)\]\(/images/', r'![\1](/assets/images/', content)
        content = re.sub(r'!\[(.*?)\]\(images/', r'![\1](/assets/images/', content)
        
        # Handle relative paths like ../images/
        content = re.sub(r'!\[(.*?)\]\(\.\./images/', r'![\1](/assets/images/', content)
        content = re.sub(r'!\[(.*?)\]\(\.\.\/images\/', r'![\1](/assets/images/', content)
        
        # Handle other relative paths to images directory
        content = re.sub(r'!\[(.*?)\]\((?:\.\./)+images/', r'![\1](/assets/images/', content)
        
        # Also fix HTML img tags
        content = re.sub(r'<img\s+src=["\'](?:/)?images/', r'<img src="/assets/images/', content)
        content = re.sub(r'<img\s+src=["\']\.\./images/', r'<img src="/assets/images/', content)
        content = re.sub(r'<img\s+src=["\']((?:\.\./)+)images/', r'<img src="/assets/images/', content)
        
        # Log a message if we still have image references that might be broken
        if re.search(r'!\[.*?\]\(.*?\.(?:png|jpg|jpeg|gif|svg)', content) and not re.search(r'!\[.*?\]\(/assets/images/', content):
            print("Warning: Found image references that might not be properly fixed:")
            for match in re.finditer(r'!\[(.*?)\]\((.*?\.(?:png|jpg|jpeg|gif|svg))', content):
                print(f"  Image: {match.group(1)} -> {match.group(2)}")
        
        return content

    def build_web_simulator(self, nav_data):
        """Build the web simulator and copy it to the documentation site"""
        print("Building web simulator...")
        
        # Create simulator directory
        self.simulator_dir.mkdir(parents=True, exist_ok=True)
        
        # Check if we have a simulator documentation page
        simulator_md = self.docs_dir / 'simulator' / 'index.md'
        simulator_doc_exists = simulator_md.exists() and simulator_md.stat().st_size > 0
        
        # If we have a non-empty simulator documentation, convert it
        if simulator_doc_exists:
            # Create a docs directory for the simulator documentation
            simulator_docs_dir = self.site_dir / 'simulator-docs'
            simulator_docs_dir.mkdir(parents=True, exist_ok=True)
            
            # Convert the markdown to HTML in the docs directory
            simulator_docs_html = simulator_docs_dir / 'index.html'
            self.convert_markdown_file(simulator_md, simulator_docs_html, nav_data)
            print(f"Generated simulator documentation at {simulator_docs_html}")
        else:
            # If the simulator/index.md file is empty or doesn't exist,
            # we'll use the web/index.html as the documentation base
            web_index_html = Path('web') / 'index.html'
            if web_index_html.exists():
                # Create a docs directory for the simulator documentation
                simulator_docs_dir = self.site_dir / 'simulator-docs'
                simulator_docs_dir.mkdir(parents=True, exist_ok=True)
                
                # Generate documentation from the web/index.html
                self.generate_simulator_docs_from_html(web_index_html, simulator_docs_dir / 'index.html', nav_data)
                print(f"Generated simulator documentation from web/index.html")
        
        # Check if we should skip the simulator build
        if self.skip_simulator_build:
            # Check if the web directory exists
            web_dir = Path('web')
            if not web_dir.exists() or not web_dir.is_dir():
                print("Warning: Web directory not found, cannot copy simulator files")
                self.create_simulator_placeholder(nav_data)
                return
                
            # Copy the web directory contents to the simulator directory
            print("Skipping simulator build, copying existing web files...")
            
            # Copy all files from the web directory to the simulator directory
            for item in web_dir.glob('*'):
                if item.is_file():
                    # Copy the file directly
                    shutil.copy2(item, self.simulator_dir / item.name)
                    print(f"  Copied {item.name}")
                elif item.is_dir():
                    # Copy the directory recursively
                    dest_dir = self.simulator_dir / item.name
                    if dest_dir.exists():
                        shutil.rmtree(dest_dir)
                    shutil.copytree(item, dest_dir)
                    print(f"  Copied directory {item.name}/")
                
            # Check if we successfully copied the index.html
            if (self.simulator_dir / 'index.html').exists():
                # Modify the simulator index.html to include our navigation
                self.integrate_simulator_with_theme(nav_data)
                
                # If we have simulator documentation, add a link to it
                if simulator_doc_exists:
                    self.add_docs_link_to_simulator()
            else:
                print("Warning: Could not copy simulator index.html")
                self.create_simulator_placeholder(nav_data)
                
            return
        
        # Run the build_web.sh script with the simulator directory as the destination
        try:
            result = subprocess.run(
                ['bash', 'build_web.sh', str(self.simulator_dir)],
                check=True,
                capture_output=True,
                text=True
            )
            print(f"Web simulator build output:\n{result.stdout}")
            
            # Check if index.html exists in the simulator directory
            if (self.simulator_dir / 'index.html').exists():
                # Modify the simulator index.html to include our navigation
                self.integrate_simulator_with_theme(nav_data)
                
                # If we have simulator documentation, add a link to it
                if simulator_doc_exists:
                    self.add_docs_link_to_simulator()
                
        except subprocess.CalledProcessError as e:
            print(f"Error building web simulator: {e}")
            print(f"Error output: {e.stderr}")
            raise
    
    def add_docs_link_to_simulator(self):
        """Add a link to the simulator documentation in the simulator page"""
        simulator_index = self.simulator_dir / 'index.html'
        
        if not simulator_index.exists():
            return
            
        # Read the simulator HTML
        html_content = simulator_index.read_text(encoding='utf-8')
        
        # Add a link to the documentation
        docs_link = '<div class="docs-link-container"><a href="/simulator-docs/index.html" class="docs-link">View Simulator Documentation</a></div>'
        
        # Insert the link after the simulator-container div
        html_content = html_content.replace('<div class="simulator-container">', 
                                           '<div class="simulator-container">' + docs_link)
        
        # Write the modified HTML back
        simulator_index.write_text(html_content, encoding='utf-8')
        
        print("Added documentation link to simulator page")
    
    def integrate_simulator_with_theme(self, nav_data):
        """Integrate the simulator with the site theme and navigation"""
        simulator_index = self.simulator_dir / 'index.html'
        
        if not simulator_index.exists():
            print("Warning: Simulator index.html not found, cannot integrate with theme")
            return
            
        # Read the simulator HTML
        simulator_html = simulator_index.read_text(encoding='utf-8')
        
        # Extract the content from the body
        body_start = simulator_html.find('<body')
        body_end = simulator_html.find('</body>')
        
        if body_start == -1 or body_end == -1:
            print("Warning: Could not find body tags in simulator HTML")
            return
            
        # Find where the actual content starts (after the body tag)
        content_start = simulator_html.find('>', body_start) + 1
        
        # Extract the content
        simulator_content = simulator_html[content_start:body_end].strip()
        
        # Extract any scripts from the head
        head_start = simulator_html.find('<head')
        head_end = simulator_html.find('</head>')
        
        scripts = []
        styles = []
        
        if head_start != -1 and head_end != -1:
            head_content = simulator_html[head_start:head_end]
            
            # Extract scripts
            script_pattern = re.compile(r'<script[^>]*>.*?</script>', re.DOTALL)
            scripts = script_pattern.findall(head_content)
            
            # Extract styles
            style_pattern = re.compile(r'<link[^>]*rel=["\']stylesheet["\'][^>]*>', re.DOTALL)
            styles = style_pattern.findall(head_content)
            
            # Also get inline styles
            inline_style_pattern = re.compile(r'<style[^>]*>.*?</style>', re.DOTALL)
            styles.extend(inline_style_pattern.findall(head_content))
        
        # Get the simulator template
        try:
            # Try to get the simulator template
            simulator_template = self.env.get_template('simulator.html')
            
            # Render the template with the simulator content
            output = simulator_template.render(
                content=simulator_content,
                styles_placeholder=''.join(styles),
                scripts_placeholder=''.join(scripts),
                meta={'layout': 'simulator', 'title': 'Web Simulator'},
                title='Web Simulator',
                nav=nav_data,
                current_path='/simulator/index.html'
            )
            
            # Write the integrated HTML back to the file
            simulator_index.write_text(output, encoding='utf-8')
            print("Integrated simulator with site theme and navigation")
            
        except Exception as e:
            print(f"Error integrating simulator with theme: {e}")
            # If there's an error, create a placeholder instead
            self.create_simulator_placeholder(nav_data)
            
    def create_simulator_placeholder(self, nav_data):
        """Create a placeholder page for the simulator when the build fails"""
        self.simulator_dir.mkdir(parents=True, exist_ok=True)
        
        # Render the placeholder template
        placeholder_template = self.env.get_template('simulator_placeholder.html')
        placeholder_content = placeholder_template.render()
        
        # Render simulator template with the content
        simulator_template = self.env.get_template('simulator.html')
        output = simulator_template.render(
            content=placeholder_content,
            styles_placeholder='',
            scripts_placeholder='',
            meta={'layout': 'simulator', 'title': 'Web Simulator'},
            title='Web Simulator',
            nav=nav_data,
            current_path='/simulator/index.html'
        )
        
        # Write the placeholder file
        (self.simulator_dir / 'index.html').write_text(output)
        print("Created simulator placeholder page")

    def generate_simulator_docs_from_html(self, html_path, output_path, nav_data):
        """Generate simulator documentation from the web/index.html file"""
        # Read the HTML content
        html_content = html_path.read_text(encoding='utf-8')
        
        # Extract the title
        title_match = re.search(r'<title>(.*?)</title>', html_content)
        title = title_match.group(1) if title_match else "Web Simulator"
        
        # Check if we have a simulator docs template
        simulator_docs_template_path = self.template_dir / 'simulator_docs.html'
        if simulator_docs_template_path.exists():
            # Use the template if it exists
            simulator_docs_template = self.env.get_template('simulator_docs.html')
            doc_content = simulator_docs_template.render(title=title)
        else:
            # Create a minimal content if no template exists
            doc_content = f"<h1>{title}</h1>"
            
        # Render page template with the content
        page_template = self.env.get_template('page.html')
        output = page_template.render(
            content=doc_content,
            meta={'title': title},
            title=title,
            nav=nav_data,
            current_path='/simulator-docs/index.html'
        )
        
        # Write the output file
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(output, encoding='utf-8')
        print(f"Generated simulator documentation at {output_path}")

if __name__ == '__main__':
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Build PixelTheater documentation')
    parser.add_argument('--skip-simulator', action='store_true', 
                        help='Skip building the web simulator and just copy existing files')
    args = parser.parse_args()
    
    # Print a message about the simulator build mode
    if args.skip_simulator:
        print("Running in fast mode: Skipping web simulator compilation")
        print("Use this mode for faster documentation builds during development")
        print("For a full build including the simulator, omit the --skip-simulator flag")
    else:
        print("Running full build including web simulator compilation (this may take a while)")
        print("For faster builds during development, use the --skip-simulator flag")
    
    # Create builder with the skip_simulator_build flag
    builder = DocBuilder(skip_simulator_build=args.skip_simulator)
    builder.build() 