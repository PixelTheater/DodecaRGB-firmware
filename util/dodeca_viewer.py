#!/usr/bin/env python3

import sys
import os
import json
import logging
import argparse
import matplotlib
import numpy as np
from pathlib import Path

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

def setup_matplotlib_backend():
    """Setup matplotlib with the first available backend"""
    backends = ['MacOSX', 'TkAgg', 'Qt5Agg', 'Agg']
    
    for backend in backends:
        try:
            matplotlib.use(backend, force=True)
            import matplotlib.pyplot as plt
            plt.figure()
            plt.close()
            return matplotlib.get_backend()
        except Exception:
            continue
    
    raise RuntimeError("No suitable matplotlib backend found")

# Setup matplotlib before imports
setup_matplotlib_backend()

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection
from mpl_toolkits.mplot3d import proj3d

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

class DodecaVisualizerJSON:
    """3D visualizer for dodecahedron LED models using JSON data"""
    
    def __init__(self, json_file: str):
        """Load model data from JSON file"""
        self.json_file = json_file
        self.model_data = None
        self.load_model_data()
        
        # Extract key data for easier access
        self.points = np.array([[p['x'], p['y'], p['z']] for p in self.model_data['points']])
        self.face_data = self.model_data['faces']
        self.edges = self.model_data.get('edges', [])
        self.face_types = self.model_data['face_types']
        
        # UI state
        self.selected_face = None
        self.collections = {}
        
    def load_model_data(self):
        """Load and validate JSON model data"""
        try:
            with open(self.json_file, 'r') as f:
                self.model_data = json.load(f)
            
            # Validate required sections
            required_sections = ['model', 'points', 'faces', 'face_types']
            missing = [s for s in required_sections if s not in self.model_data]
            if missing:
                raise ValueError(f"Missing required sections in JSON: {missing}")
                
            print(f"✓ Loaded model: {self.model_data['model']['name']} v{self.model_data['model']['version']}")
            edge_count = len(self.model_data.get('edges', []))
            print(f"✓ {len(self.model_data['points'])} LEDs, {len(self.model_data['faces'])} faces, {edge_count} edges")
            
        except Exception as e:
            raise RuntimeError(f"Failed to load model data from {self.json_file}: {e}")
    
    def get_face_vertices_from_edges(self, face_id: int):
        """Get the 3D vertices for a face from edge data"""
        face_edges = [e for e in self.edges if e['face_id'] == face_id]
        if not face_edges:
            return []
        
        # Extract vertices from edges - each edge has start and end points
        vertices = []
        for edge in face_edges:
            vertices.append(edge['start'])
        
        return np.array(vertices)
    
    def draw_face_wireframe(self, ax):
        """Draw the dodecahedron face wireframes using edge data"""
        if not self.edges:
            print("No edge data available for wireframe display")
            return
            
        face_lines = []
        
        for edge in self.edges:
            start = edge['start']
            end = edge['end']
            face_lines.append([start, end])
        
        if face_lines:
            lines = Line3DCollection(face_lines, colors='gray', alpha=0.6, linewidths=1.5)
            ax.add_collection3d(lines)
            self.collections['wireframe'] = lines
    
    def draw_face_surfaces(self, ax):
        """Draw filled face surfaces using edge-derived vertices"""
        if not self.edges:
            return
            
        faces_3d = []
        face_colors = []
        
        # Generate colors for faces
        cmap = plt.cm.tab10
        
        for face in self.face_data:
            face_id = face['id']
            vertices = self.get_face_vertices_from_edges(face_id)
            
            if len(vertices) >= 3:
                faces_3d.append(vertices)
                color = cmap(face_id % 10)
                face_colors.append((*color[:3], 0.2))  # Semi-transparent
        
        if faces_3d:
            poly = Poly3DCollection(faces_3d, alpha=0.2)
            poly.set_facecolor(face_colors)
            poly.set_edgecolor('black')
            poly.set_linewidth(0.5)
            ax.add_collection3d(poly)
            self.collections['faces'] = poly
    
    def draw_leds(self, ax):
        """Draw all LEDs as points"""
        # Draw all LEDs as small white dots
        led_scatter = ax.scatter(self.points[:, 0], 
                               self.points[:, 1], 
                               self.points[:, 2],
                               c='white', 
                               edgecolor='black',
                               s=20, 
                               alpha=0.8,
                               zorder=5)
        self.collections['leds'] = led_scatter
        
        # Add face number labels at face centers
        self.collections['face_labels'] = []
        for face in self.face_data:
            face_id = face['id']
            # Calculate face center from its LEDs
            face_led_indices = [led['index'] for led in face['leds']]
            if face_led_indices:
                face_led_positions = self.points[face_led_indices]
                center = np.mean(face_led_positions, axis=0)
                
                label = ax.text(center[0], center[1], center[2], 
                              str(face_id),
                              fontsize=14,
                              color='black',
                              backgroundcolor='yellow',
                              alpha=0.9,
                              ha='center',
                              va='center',
                              weight='bold',
                              zorder=10)
                self.collections['face_labels'].append(label)
    
    def highlight_face(self, ax, face_id: int):
        """Highlight a specific face and its LEDs"""
        if face_id is None:
            return
        
        face = self.face_data[face_id]
        face_led_indices = [led['index'] for led in face['leds']]
        
        if not face_led_indices:
            return
        
        # Clear previous LED labels
        if 'led_labels' in self.collections:
            for label in self.collections['led_labels']:
                label.remove()
            self.collections['led_labels'] = []
        
        # Highlight face LEDs
        face_led_colors = ['lightgray'] * len(self.points)
        face_led_edges = ['gray'] * len(self.points)
        
        for led_idx in face_led_indices:
            face_led_colors[led_idx] = 'red'
            face_led_edges[led_idx] = 'darkred'
        
        # Update LED visualization
        self.collections['leds'].set_color(face_led_colors)
        self.collections['leds'].set_edgecolor(face_led_edges)
        
        # Add LED number labels for highlighted face
        self.collections['led_labels'] = []
        for led_idx in face_led_indices:
            pos = self.points[led_idx]
            led_label = self.model_data['points'][led_idx]['label']  # 1-based LED number
            label = ax.text(pos[0], pos[1], pos[2], 
                          str(led_label),
                          fontsize=8,
                          color='white',
                          backgroundcolor='red',
                          alpha=0.9,
                          ha='center',
                          va='center',
                          weight='bold',
                          zorder=10)
            self.collections['led_labels'].append(label)
            
        # Show face information
        geometric_id = face.get('remap_to', face_id)
        remap_text = f" (→ geometric pos {geometric_id})" if face.get('remap_to') else ""
        info_text = (f"Face {face_id}{remap_text}\n"
                    f"Rotation: {face['rotation']}\n"
                    f"LEDs: {len(face_led_indices)}\n"
                    f"Type: {face['type']}")
        
        if 'info_text' in self.collections:
            self.collections['info_text'].remove()
        
        self.collections['info_text'] = ax.text2D(0.02, 0.98, info_text,
                                                transform=ax.transAxes,
                                                fontsize=12,
                                                verticalalignment='top',
                                                bbox=dict(facecolor='white', alpha=0.9, pad=5))
    
    def setup_interactivity(self, fig, ax):
        """Setup mouse interaction for face selection"""
        
        def on_click(event):
            """Handle mouse click events for face selection"""
            if event.inaxes != ax or event.button != 1:
                return
            
            x, y = event.xdata, event.ydata
            if x is None or y is None:
                return
            
            # Find closest face
            closest_face = None
            min_dist = float('inf')
            
            for face in self.face_data:
                face_id = face['id']
                face_led_indices = [led['index'] for led in face['leds']]
                if not face_led_indices:
                    continue
                
                # Calculate face center
                face_center_3d = np.mean(self.points[face_led_indices], axis=0)
                
                # Project to screen coordinates
                try:
                    xs, ys, _ = proj3d.proj_transform(face_center_3d[0], 
                                                   face_center_3d[1], 
                                                   face_center_3d[2], 
                                                   ax.get_proj())
                    dist = np.sqrt((xs - x)**2 + (ys - y)**2)
                    
                    if dist < min_dist:
                        min_dist = dist
                        closest_face = face_id
                except:
                    continue
            
            # Highlight selected face
            if closest_face is not None and min_dist < 0.05:  # Click tolerance
                self.selected_face = closest_face
                self.highlight_face(ax, closest_face)
                fig.canvas.draw_idle()
                print(f"Selected face {closest_face}")

        def on_key(event):
            """Handle keyboard events"""
            if event.key == 'r':
                # Reset view
                self.reset_view(ax)
                fig.canvas.draw_idle()
            elif event.key == 'c':
                # Clear selection
                self.selected_face = None
                self.reset_led_visualization()
                if 'info_text' in self.collections:
                    self.collections['info_text'].remove()
                if 'led_labels' in self.collections:
                    for label in self.collections['led_labels']:
                        label.remove()
                    self.collections['led_labels'] = []
                fig.canvas.draw_idle()
            elif event.key.isdigit():
                # Select face by number
                face_num = int(event.key)
                if 0 <= face_num < len(self.face_data):
                    self.selected_face = face_num
                    self.highlight_face(ax, face_num)
                    fig.canvas.draw_idle()
                    print(f"Selected face {face_num}")
        
        fig.canvas.mpl_connect('button_press_event', on_click)
        fig.canvas.mpl_connect('key_press_event', on_key)
    
    def reset_led_visualization(self):
        """Reset LED visualization to default state"""
        if 'leds' in self.collections:
            num_leds = len(self.points)
            self.collections['leds'].set_color(['white'] * num_leds)
            self.collections['leds'].set_edgecolor(['black'] * num_leds)
    
    def reset_view(self, ax):
        """Reset the 3D view to default position and set proper axis limits"""
        ax.view_init(elev=20, azim=45)
        
        # Set axis limits to center the model
        max_range = np.array([
            self.points[:,0].max() - self.points[:,0].min(),
            self.points[:,1].max() - self.points[:,1].min(),
            self.points[:,2].max() - self.points[:,2].min()
        ]).max() / 2.0
        
        mid_x = (self.points[:,0].max() + self.points[:,0].min()) * 0.5
        mid_y = (self.points[:,1].max() + self.points[:,1].min()) * 0.5
        mid_z = (self.points[:,2].max() + self.points[:,2].min()) * 0.5
        
        ax.set_xlim(mid_x - max_range, mid_x + max_range)
        ax.set_ylim(mid_y - max_range, mid_y + max_range)
        ax.set_zlim(mid_z - max_range, mid_z + max_range)
    
    def visualize(self):
        """Create and display the 3D visualization"""
        print("Creating 3D visualization...")
        
        # Create figure and 3D axes
        fig = plt.figure(figsize=(14, 10))
        ax = fig.add_subplot(111, projection='3d')
        
        # Set equal aspect ratio
        ax.set_box_aspect([1, 1, 1])
        
        # Draw model components
        self.draw_face_wireframe(ax)
        self.draw_face_surfaces(ax)
        self.draw_leds(ax)
        
        # Setup axes
        ax.set_xlabel('X (mm)')
        ax.set_ylabel('Y (mm)')
        ax.set_zlabel('Z (mm)')
        
        # Set initial view and limits
        self.reset_view(ax)
        
        # Setup interactivity
        self.setup_interactivity(fig, ax)
    
        # Add title and instructions
        model_name = self.model_data['model']['name']
        title = f"{model_name} - Interactive LED Model Viewer with Edges"
        fig.suptitle(title, fontsize=16, weight='bold')
        
        instructions = ("Controls:\n"
                       "• Click face to select\n"
                       "• Press 0-9 to select face\n" 
                       "• Press 'c' to clear selection\n"
                       "• Press 'r' to reset view\n"
                       "• Mouse drag to rotate")
        
        fig.text(0.02, 0.02, instructions, fontsize=10, 
                verticalalignment='bottom',
                bbox=dict(facecolor='lightgray', alpha=0.8, pad=5))
        
        print("✓ Visualization ready")
        print("Click on faces to explore the model!")
        
        plt.tight_layout()
        plt.show()


def find_model_json(path: str) -> str:
    """Find model.json file, handling various input formats"""
    path = Path(path)
    
    if path.is_file() and path.suffix == '.json':
        return str(path)
    elif path.is_dir():
        json_file = path / 'model.json'
        if json_file.exists():
            return str(json_file)
        else:
            raise FileNotFoundError(f"No model.json found in directory: {path}")
    else:
        raise FileNotFoundError(f"Invalid path: {path}")


def main():
    parser = argparse.ArgumentParser(description='3D LED Model Visualizer')
    parser.add_argument('model', 
                       help='Path to model.json file or directory containing it')
    parser.add_argument('--info', action='store_true',
                       help='Show model information and exit')
    
    args = parser.parse_args()
    
    try:
        # Find JSON file
        json_file = find_model_json(args.model)
        print(f"Loading model from: {json_file}")
        
        # Create visualizer
        visualizer = DodecaVisualizerJSON(json_file)
        
        if args.info:
            # Just show info and exit
            model = visualizer.model_data['model']
            print(f"\nModel Information:")
            print(f"  Name: {model['name']}")
            print(f"  Version: {model['version']}")
            print(f"  Description: {model['description']}")
            print(f"  Author: {model.get('author', 'Unknown')}")
            print(f"  LEDs: {len(visualizer.model_data['points'])}")
            print(f"  Faces: {len(visualizer.model_data['faces'])}")
            print(f"  Edges: {len(visualizer.model_data.get('edges', []))}")
        else:
            # Show visualization
            visualizer.visualize()
            
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main() 