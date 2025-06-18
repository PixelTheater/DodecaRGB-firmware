#!/usr/bin/env python3

"""
Apply the exact fix for perfect LED/vertex alignment by accounting for PCB center offset in vertex generation.
"""

import re

def fix_vertex_pcb_offset():
    """Add PCB center offset to vertex generation to match LED positioning exactly"""
    
    generate_model_file = "generate_model.py"
    
    print("=== APPLYING EXACT GEOMETRY FIX ===\n")
    print(f"Modifying: {generate_model_file}")
    
    # Read the current file
    with open(generate_model_file, 'r') as f:
        content = f.read()
    
    # Find the vertex generation section and add PCB offset
    # Look for the line where vertices are generated:
    # "x = radius * math.cos(angle)"
    # "y = radius * math.sin(angle)"
    
    old_vertex_generation = '''                for j in range(num_sides):
                    angle = j * (2 * math.pi / num_sides)
                    x = radius * math.cos(angle)
                    y = radius * math.sin(angle)
                    base_vertices.append([x, y, 0])'''
    
    new_vertex_generation = '''                for j in range(num_sides):
                    angle = j * (2 * math.pi / num_sides)
                    x = radius * math.cos(angle)
                    y = radius * math.sin(angle)
                    
                    # Apply same PCB center offset as LEDs for perfect alignment
                    # (from load_pcb_points: x += 0.2, y -= 55.884, then scale by 5.15)
                    x += 0.2 * 5.15  # PCB X offset
                    y -= 55.884 * 5.15  # PCB Y offset
                    
                    base_vertices.append([x, y, 0])'''
    
    if old_vertex_generation in content:
        new_content = content.replace(old_vertex_generation, new_vertex_generation)
        
        # Write back to file
        with open(generate_model_file, 'w') as f:
            f.write(new_content)
        
        print("✓ Applied PCB center offset to vertex generation")
        print("  Vertices now use identical positioning as LEDs")
        print(f"  Offset applied: (+{0.2 * 5.15:.1f}, {-55.884 * 5.15:.1f}) mm")
        return True
    else:
        print("❌ Could not find vertex generation section")
        print("   Manual fix required:")
        print("   Add PCB offset to vertex coordinates in generate_model.py")
        return False

def restore_tight_tolerance():
    """Set validation tolerance to very tight value since geometry should now be exact"""
    
    model_file = "../lib/PixelTheater/include/PixelTheater/model/model.h"
    
    print(f"\nSetting exact geometry tolerance...")
    
    # Read the current file
    with open(model_file, 'r') as f:
        content = f.read()
    
    # Set tolerance to 1.05x for near-perfect geometry (allows tiny floating point errors)
    old_tolerance = 'float reasonable_distance = max_vertex_distance * 1.2f;'
    new_tolerance = 'float reasonable_distance = max_vertex_distance * 1.05f;'
    
    if old_tolerance in content:
        new_content = content.replace(old_tolerance, new_tolerance)
        
        # Write back to file
        with open(model_file, 'w') as f:
            f.write(new_content)
        
        print("✓ Set tolerance to 1.05x for exact geometry (allows tiny floating point errors)")
        return True
    else:
        print("❌ Could not find tolerance line to modify")
        return False

def main():
    print("APPLYING EXACT GEOMETRY FIX")
    print("=" * 40)
    
    success_count = 0
    
    # Fix 1: Apply PCB offset to vertex generation
    if fix_vertex_pcb_offset():
        success_count += 1
    
    # Fix 2: Set tight tolerance for exact geometry
    if restore_tight_tolerance():
        success_count += 1
    
    print(f"\n{'='*40}")
    print(f"COMPLETED: {success_count}/2 fixes applied")
    
    if success_count == 2:
        print("\n✅ EXACT GEOMETRY FIX APPLIED")
        print("\nNext steps:")
        print("1. Regenerate model: python generate_model.py -d ../src/models/DodecaRGBv2_1 -y")
        print("2. Verify alignment: python verify_geometry_fix.py") 
        print("3. Recompile firmware - should pass validation with <1mm tolerance!")
    else:
        print(f"\n⚠️  {2-success_count} fixes failed - manual intervention required")

if __name__ == "__main__":
    main() 