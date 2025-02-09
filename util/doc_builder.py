import frontmatter
import os
from datetime import datetime, timedelta
import glob
from typing import Optional
import re
import argparse  # Import the argparse module

class DocBuilder:
    # Add color/symbol constants
    GREEN_CHECK = "\033[32m✓\033[0m"  # Green checkmark
    YELLOW_DASH = "\033[33m-\033[0m"   # Yellow dash
    
    def __init__(self, docs_dir: str):
        self.docs_dir = docs_dir
        self.repository = "https://github.com/somebox/DodecaRGB-firmware"
        self.author = "Jeremy Seitz - somebox.com"
        self.version = self._get_version()
        self.project = "DodecaRGB Firmware"
        
    def _get_version(self) -> str:
        """Read version from VERSION file"""
        version_file = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'VERSION')
        try:
            with open(version_file, 'r') as f:
                return f.read().strip()
        except FileNotFoundError:
            print(f"Warning: VERSION file not found at {version_file}")
            return "0.0.0"
        
    def update_doc(self, filepath: str, force: bool = False) -> None:
        """Update frontmatter and insert version/date below title"""
        with open(filepath, 'r', encoding='utf-8') as f:
            post = frontmatter.load(f)
        
        # Get the existing 'generated' date from frontmatter
        generated_str = post.metadata.get('generated')
        generated_time: Optional[datetime] = None
        if generated_str:
            try:
                generated_time = datetime.strptime(generated_str, '%Y-%m-%d %H:%M')
            except ValueError:
                pass
        
        # Get the file modification time and check if update needed
        modified_time = datetime.fromtimestamp(os.path.getmtime(filepath))
        should_update = (force or 
                        generated_time is None or 
                        modified_time > generated_time + timedelta(minutes=1))
        
        if not should_update:
            print(f"{self.YELLOW_DASH} {os.path.relpath(filepath, self.docs_dir)}")
            return

        # Update frontmatter with current timestamp
        post.metadata.update({
            'version': self.version,
            'generated': datetime.now().strftime('%Y-%m-%d %H:%M'),
        })
        
        # Write updated file
        with open(filepath, 'w', encoding='utf-8') as f:
            content = frontmatter.dumps(post)
            if not content.endswith('\n'):
                content += '\n'
            f.write(content)
        print(f"{self.GREEN_CHECK} {os.path.relpath(filepath, self.docs_dir)}")
        
    def process_docs(self, force: bool = False) -> None:
        """Process all markdown files in docs directory"""
        for filepath in glob.glob(os.path.join(self.docs_dir, '**/*.md'), recursive=True):
            self.update_doc(filepath, force)

def main():
    parser = argparse.ArgumentParser(description="Update frontmatter in markdown files.")
    parser.add_argument("docs_dir", nargs='?', help="The directory containing the markdown files.")
    parser.add_argument("-f", "--force", action="store_true", help="Force update all files, skip date check.")
    args = parser.parse_args()

    if not args.docs_dir:
        parser.print_help()
        return

    builder = DocBuilder(args.docs_dir)
    builder.process_docs(args.force)

if __name__ == "__main__":
    main() 