print("Testing matplotlib/Tk...")

try:
    import matplotlib
    print(f"Matplotlib version: {matplotlib.__version__}")
    
    print("Setting Tk backend...")
    matplotlib.use('TkAgg')
    
    import matplotlib.pyplot as plt
    print("Matplotlib.pyplot imported")
    
    # Create a simple 2D plot first
    plt.figure()
    plt.plot([1,2,3], [1,2,3])
    print("Created test plot")
    
    plt.show()
    print("Plot displayed")
    
except Exception as e:
    print(f"Error: {e}")
    import traceback
    traceback.print_exc()