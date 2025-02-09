import frontmatter
import os
from datetime import datetime
import glob
from typing import Optional
import re

class DocBuilder:
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
        
    def update_doc(self, filepath: str) -> None:
        """Update frontmatter and insert version/date below title"""
        with open(filepath, 'r', encoding='utf-8') as f:
            post = frontmatter.load(f)
            
        # Update or create frontmatter with current timestamp
        post.metadata.update({
            'version': self.version,
            'generated': datetime.now().strftime('%Y-%m-%d %H:%M'),
        })
        
        # Write updated file
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(frontmatter.dumps(post))
            
    def process_docs(self) -> None:
        """Process all markdown files in docs directory"""
        for filepath in glob.glob(os.path.join(self.docs_dir, '**/*.md'), recursive=True):
            print(f"Processing {filepath}")
            self.update_doc(filepath)

def main():
    docs_dir = "../docs"
    builder = DocBuilder(docs_dir)
    builder.process_docs()

if __name__ == "__main__":
    main() 