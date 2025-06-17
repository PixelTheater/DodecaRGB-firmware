#!/usr/bin/env python3
"""
Test Fixture Generation Script

Generates model.h files for test fixtures from YAML definitions.
This ensures test fixtures stay in sync with the main generation logic.
"""

import os
import sys
import argparse
import subprocess
from pathlib import Path


def generate_test_fixture(fixture_path: Path) -> bool:
    """
    Generate a single test fixture model.h file from its YAML definition.
    
    Args:
        fixture_path: Path to the fixture directory (e.g., test/fixtures/models/basic_pentagon_model)
        
    Returns:
        True if generation succeeded, False otherwise
    """
    yaml_file = fixture_path / "model.yaml"
    if not yaml_file.exists():
        print(f"❌ No model.yaml found in {fixture_path}")
        return False
        
    pcb_dir = fixture_path / "pcb"
    if not pcb_dir.exists() or not any(pcb_dir.glob("*.csv")):
        print(f"❌ No PCB CSV files found in {pcb_dir}")
        return False
    
    print(f"🔄 Generating fixture: {fixture_path.name}")
    
    try:
        # Call the main generate_model.py script with the model directory
        script_path = Path(__file__).parent / "generate_model.py"
        cmd = [
            sys.executable,
            str(script_path),
            "--model-dir", str(fixture_path),
            "--yes"  # Auto-overwrite
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✅ Generated {fixture_path.name}/model.h")
            return True
        else:
            print(f"❌ Failed to generate {fixture_path.name}:")
            print(f"   STDOUT: {result.stdout}")
            print(f"   STDERR: {result.stderr}")
            return False
        
    except Exception as e:
        print(f"❌ Failed to generate {fixture_path.name}: {e}")
        return False


def find_test_fixtures() -> list[Path]:
    """Find all test fixture directories with model.yaml files."""
    fixtures_root = Path("test/fixtures/models")
    fixtures = []
    
    if not fixtures_root.exists():
        return fixtures
    
    for item in fixtures_root.iterdir():
        if item.is_dir() and (item / "model.yaml").exists():
            fixtures.append(item)
    
    return sorted(fixtures)


def main():
    parser = argparse.ArgumentParser(description='Generate test fixture model.h files from YAML definitions')
    parser.add_argument('--fixture', help='Generate specific fixture by name (e.g., basic_pentagon_model)')
    parser.add_argument('--all', action='store_true', help='Generate all test fixtures')
    parser.add_argument('--list', action='store_true', help='List available test fixtures')

    args = parser.parse_args()

    # Find all fixtures
    fixtures = find_test_fixtures()
    
    if args.list:
        print("📋 Available test fixtures:")
        for fixture in fixtures:
            yaml_file = fixture / "model.yaml"
            print(f"  • {fixture.name}")
            if yaml_file.exists():
                print(f"    📄 {yaml_file}")
            pcb_files = list((fixture / "pcb").glob("*.csv")) if (fixture / "pcb").exists() else []
            for pcb_file in pcb_files:
                print(f"    📄 {pcb_file}")
        return

    if args.fixture:
        # Generate specific fixture
        fixture_path = Path(f"test/fixtures/models/{args.fixture}")
        if not fixture_path.exists():
            print(f"❌ Fixture not found: {fixture_path}")
            print(f"Available fixtures: {[f.name for f in fixtures]}")
            sys.exit(1)
        
        success = generate_test_fixture(fixture_path)
        sys.exit(0 if success else 1)
    
    elif args.all:
        # Generate all fixtures
        success_count = 0
        for fixture in fixtures:
            if generate_test_fixture(fixture):
                success_count += 1
        
        print(f"\n📊 Generated {success_count}/{len(fixtures)} test fixtures successfully")
        sys.exit(0 if success_count == len(fixtures) else 1)
    
    else:
        parser.print_help()
        print(f"\nAvailable fixtures: {[f.name for f in fixtures]}")


if __name__ == "__main__":
    main() 