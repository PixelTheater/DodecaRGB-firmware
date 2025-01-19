import sys
import os
import logging

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from mpl_toolkits.mplot3d import proj3d
import matplotlib
import math
from util.matrix3d import Matrix3D

# Enable interactive backend
matplotlib.use('TkAgg')

# Import shared constants and functions
from util.dodeca_core import (
    TWO_PI, zv, ro, xv, radius, 
    side_rotation, FACE_COLORS,
    transform_led_point, load_pcb_points
)

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def draw_dodecahedron(ax, collections=None):
    """Draw a dodecahedron with filled faces that match LED orientation."""
    m = Matrix3D()
    
    # Generate base pentagon vertices (at origin, in XY plane)
    pentagon = []
    for i in range(5):
        angle = i * ro  # ro = TWO_PI/5
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        pentagon.append([x, y, 0])
    
    # Position faces using same angles as LED transforms
    faces = []
    
    for side in range(12):
        m.push_matrix()
        
        # Match face positioning from transform_led_point()
        if side == 0:  # bottom
            m.rotate_z(-zv - ro*2)
        elif side > 0 and side < 6:  # bottom half
            m.rotate_z(ro*side + zv - ro)
            m.rotate_x(xv)
        elif side >= 6 and side < 11:  # top half
            m.rotate_z(ro*side - zv + ro*3)
            m.rotate_x(math.pi - xv)
        else:  # side == 11, top
            m.rotate_x(math.pi)
            m.rotate_z(zv)
        
        # Move face out to radius
        m.translate(0, 0, radius*1.31)
        
        # Additional hemisphere rotation
        if side >= 6 and side < 11:
            m.rotate_z(zv)
        else:
            m.rotate_z(-zv)
        
        # Transform pentagon vertices to face position
        face = [m.apply(v) for v in pentagon]
        faces.append(face)
        
        m.pop_matrix()
    
    # Create the 3D polygons with proper depth sorting
    poly = Poly3DCollection(faces, alpha=0.3)
    poly.set_facecolor(FACE_COLORS)  # Use our custom colors
    poly.set_edgecolor('gray')
    poly.set_zorder(1)
    
    if collections is not None:
        collections['faces'] = poly
    
    ax.add_collection3d(poly)
    return faces

def visualize_model(pcb_points=None):
    """Draw the dodecahedron using matplotlib"""
    # Create figure and 3D axes with perspective projection
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, projection='3d')
    
    # Set camera position and focal length using set_box_aspect
    ax.set_box_aspect([1, 1, 1])  # Equal aspect ratio for all axes
    
    # Enable mouse interaction
    ax.mouse_init()
    
    # Draw coordinate axes for reference
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    
    # Add RGB axis lines with labels
    ax.plot([0, radius], [0, 0], [0, 0], color='r', label='X (Red)', linewidth=2)
    ax.plot([0, 0], [0, radius], [0, 0], color='g', label='Y (Green)', linewidth=2)
    ax.plot([0, 0], [0, 0], [0, radius], color='b', label='Z (Blue)', linewidth=2)
    
    # Collections dict to store plot elements
    collections = {
        'faces': None,
        'leds': None,
        'text': None,
        'led_labels': [],
        'face_labels': []
    }
    
    # Draw the dodecahedron structure
    faces = draw_dodecahedron(ax, collections)
    
    # Calculate all LED positions
    led_positions = []
    for side in range(12):
        for led in pcb_points:
            world_pos = transform_led_point(led['x'], led['y'], led['num'], side)
            led_positions.append(world_pos)
    
    # Convert to numpy arrays for faster plotting
    led_positions = np.array(led_positions)
    
    # Plot all LEDs as tiny points
    collections['leds'] = ax.scatter(led_positions[:, 0], 
                                   led_positions[:, 1], 
                                   led_positions[:, 2],
                                   c='white', edgecolor='none', s=5, alpha=0.5,
                                   depthshade=True, zorder=2)
    
    # Add face numbers
    for i, face in enumerate(faces):
        center = np.mean(face, axis=0)
        label = ax.text(center[0], center[1], center[2], 
                      str(i),
                      fontsize=10,
                      color='black',
                      backgroundcolor='white',
                      alpha=0.7,
                      ha='center',
                      va='center')
        collections['face_labels'].append(label)
    
    # Add status text (initially hidden)
    collections['text'] = ax.text2D(0.02, 0.98, '', transform=ax.transAxes)
    collections['text'].set_visible(False)
    
    def on_drag(event):
        """Handle mouse drag events"""
        if event.inaxes != ax or event.button != 1:  # Only handle left clicks
            return
        
        # Get mouse coordinates
        x, y = event.xdata, event.ydata
        logging.info(f"Drag at screen coordinates: ({x}, {y})")
        
        # Find closest face considering the current view
        closest_face = None
        min_dist = float('inf')
        distance_threshold = 20  # Lower the threshold for more precise selection
        
        for i, face in enumerate(faces):
            # Convert face vertices to numpy arrays
            face = np.array(face)
            
            # Calculate face center in 3D
            center = np.mean(face, axis=0)
            
            # Project center to screen coordinates
            xs, ys, _ = proj3d.proj_transform(center[0], center[1], center[2], ax.get_proj())
            
            # Calculate screen-space distance
            dist = np.sqrt((xs - x)**2 + (ys - y)**2)
            logging.info(f"Distance to face {i}: {dist}")
            
            # Get view direction - use camera position from matplotlib's view
            view_dir = np.array([
                np.cos(np.radians(ax.azim)) * np.cos(np.radians(ax.elev)),
                np.sin(np.radians(ax.azim)) * np.cos(np.radians(ax.elev)),
                np.sin(np.radians(ax.elev))
            ])
            
            # Calculate face normal
            v1 = face[1] - face[0]
            v2 = face[2] - face[0]
            normal = np.cross(v1, v2)
            normal = normal / np.linalg.norm(normal)
            
            # Check if face is visible (dot product with view direction)
            if np.dot(normal, view_dir) > 0:  # Face is visible if normal points towards camera
                if dist < min_dist:
                    min_dist = dist
                    closest_face = i
        
        # If a face is dragged over, highlight it
        if closest_face is not None and min_dist < distance_threshold:
            logging.info(f"Face {closest_face} selected with distance {min_dist}")
            # Set face colors
            face_colors = [(0.3, 0.4, 0.3, 0.5)] * 12  # Unselected faces semi-transparent
            face_colors[closest_face] = (*FACE_COLORS[closest_face], 0.5)  # Selected face more solid
            collections['faces'].set_facecolor(face_colors)
            
            # Set LED colors and properties
            led_colors = [(0.5, 0.5, 0.5, 1.0)] * len(led_positions)  # Very faded unselected LEDs
            led_edges = ['none'] * len(led_positions)  # No edges by default
            
            # Calculate start and end indices for the selected face
            start_idx = closest_face * len(pcb_points)
            end_idx = start_idx + len(pcb_points)
            
            # Update selected face LEDs
            for i in range(start_idx, end_idx):
                led_colors[i] = (1.0, 1.0, 1.0, 1.0)  # Bright white fill
                led_edges[i] = 'black'  # Black border for contrast
            
            collections['leds'].set_color(led_colors)
            collections['leds'].set_edgecolor(led_edges)
            collections['leds'].set_zorder(3)
            
            # Clear any existing LED labels
            for label in collections['led_labels']:
                label.remove()
            collections['led_labels'].clear()
            
            # Add LED labels for selected face
            for i in range(start_idx, end_idx):
                led_num = (i - start_idx)  # Convert to 0-based LED number
                pos = led_positions[i]
                label = ax.text(pos[0], pos[1], pos[2], 
                              str(led_num),
                              fontsize=8,
                              color='black',
                              backgroundcolor='white',
                              alpha=0.7,
                              ha='left',
                              va='bottom')
                collections['led_labels'].append(label)
            
            # Draw the red line for top orientation
            top_led_indices = [65, 66, 67, 68]  # Skip corners 63 and 69
            top_led_positions = []
            for led_num in top_led_indices:
                idx = start_idx + led_num
                top_led_positions.append(led_positions[idx])
            
            top_edge = np.array([top_led_positions[0], top_led_positions[-1]])
            
            if not hasattr(ax, '_orientation_line'):
                ax._orientation_line = ax.plot(top_edge[:, 0], top_edge[:, 1], top_edge[:, 2],
                                     color='red', linewidth=5, zorder=4)[0]
            else:
                ax._orientation_line.set_data_3d(top_edge[:, 0], top_edge[:, 1], top_edge[:, 2])
                ax._orientation_line.set_visible(True)
            
            # Update status text with face and rotation info
            if collections['text']:
                collections['text'].remove()
            status_text = f'Face {closest_face}\nRotation: {side_rotation[closest_face]}\nRed Line indicates top orientation'
            collections['text'] = ax.text2D(0.02, 0.98, status_text,
                                          transform=ax.transAxes,
                                          fontsize=10,
                                          verticalalignment='top',
                                          bbox=dict(facecolor='white', alpha=0.7))
            
            fig.canvas.draw_idle()

    def on_release(event):
        """Handle mouse release events"""
        if event.inaxes != ax or event.button != 1:  # Only handle left clicks
            return
        
        logging.info("Mouse released, resetting view")
        # Reset colors to colorful state
        collections['faces'].set_facecolor(FACE_COLORS)
        collections['leds'].set_color('white')
        
        # Clear LED labels
        for label in collections['led_labels']:
            label.remove()
        collections['led_labels'].clear()
        
        # Add face numbers
        for i, face in enumerate(faces):
            center = np.mean(face, axis=0)
            label = ax.text(center[0], center[1], center[2], 
                          str(i),
                          fontsize=10,
                          color='black',
                          backgroundcolor='white',
                          alpha=0.7,
                          ha='center',
                          va='center')
            collections['face_labels'].append(label)
        
        fig.canvas.draw_idle()
    
    # Connect mouse drag and release events
    fig.canvas.mpl_connect('motion_notify_event', on_drag)
    fig.canvas.mpl_connect('button_release_event', on_release)
    
    # Set equal aspect ratio for all axes
    ax.set_box_aspect([1,1,1])
    
    # Set initial view angle for isometric-like view
    ax.view_init(elev=35, azim=45)
    
    # Set axis limits to center the model
    max_range = np.array([
        led_positions[:,0].max() - led_positions[:,0].min(),
        led_positions[:,1].max() - led_positions[:,1].min(),
        led_positions[:,2].max() - led_positions[:,2].min()
    ]).max() / 2.0
    
    mid_x = (led_positions[:,0].max() + led_positions[:,0].min()) * 0.5
    mid_y = (led_positions[:,1].max() + led_positions[:,1].min()) * 0.5
    mid_z = (led_positions[:,2].max() + led_positions[:,2].min()) * 0.5
    
    ax.set_xlim(mid_x - max_range, mid_x + max_range)
    ax.set_ylim(mid_y - max_range, mid_y + max_range)
    ax.set_zlim(mid_z - max_range, mid_z + max_range)
    
    # Reduce number of ticks and adjust their appearance
    ax.set_xticks(np.linspace(ax.get_xlim()[0], ax.get_xlim()[1], 5))
    ax.set_yticks(np.linspace(ax.get_ylim()[0], ax.get_ylim()[1], 5))
    ax.set_zticks(np.linspace(ax.get_zlim()[0], ax.get_zlim()[1], 5))
    
    # Make axis labels and ticks smaller for cleaner look
    ax.tick_params(labelsize=8)
    
    # Check if there are any labeled artists before calling legend
    labeled_artists = [artist for artist in ax.get_children() if artist.get_label() and artist.get_label() != "_nolegend_"]
    if labeled_artists:
        ax.legend(fontsize=8)
    
    plt.show()

if __name__ == "__main__":
    # Load PCB points
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    # Show visualization
    visualize_model(pcb_points) 