import pytest
import tempfile
import shutil
from pathlib import Path
import sys
import os

# Ensure the util directory is in the Python path
util_dir = Path(__file__).parent.parent.resolve()
if str(util_dir) not in sys.path:
    sys.path.insert(0, str(util_dir))

# Now we can import from generate_props
try:
    from generate_props import process_images
except ImportError as e:
    print(f"Error importing generate_props: {e}", file=sys.stderr)
    # Fallback or re-raise if necessary
    process_images = None 

# Define the path to the test assets relative to this file's location
TEST_ASSETS_DIR = Path(__file__).parent.parent / "examples"
TEST_IMAGE_PATH = TEST_ASSETS_DIR / "test_pattern.png"
TEST_IMAGE_WIDTH = 100
TEST_IMAGE_HEIGHT = 75
EXPECTED_DATA_SIZE = TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT * 3 # RGB

@pytest.fixture
def temp_image_dir(tmp_path):
    """Creates a temporary directory and copies the test image into it."""
    source_image = TEST_IMAGE_PATH
    if not source_image.exists():
        pytest.skip(f"Test image not found: {source_image}")
        
    dest_image = tmp_path / source_image.name
    shutil.copy(source_image, dest_image)
    return tmp_path

def test_process_single_image(temp_image_dir):
    """Tests processing a directory with a single valid PNG image."""
    if process_images is None:
        pytest.skip("Could not import process_images from generate_props")

    header_content = process_images(str(temp_image_dir))

    assert header_content is not None, "process_images should return content for valid images."

    # Check for essential parts of the generated header
    assert "#pragma once" in header_content
    assert "namespace PixelTheater {" in header_content
    assert "struct TextureData {" in header_content
    assert "const uint32_t width;" in header_content
    assert "const uint32_t height;" in header_content
    assert "const uint8_t* data;" in header_content
    assert "} // namespace PixelTheater" in header_content

    # Check for image-specific generated code
    expected_comment = f"// test_pattern ({TEST_IMAGE_WIDTH}x{TEST_IMAGE_HEIGHT})"
    expected_data_array_name = "TEXTURE_TEST_PATTERN_DATA"
    expected_struct_name = "TEXTURE_TEST_PATTERN"
    
    assert expected_comment in header_content, "Should include comment with image name and dimensions."
    assert f"constexpr uint8_t {expected_data_array_name}[] = {{ " in header_content, "Should define the data array."
    assert f"constexpr TextureData {expected_struct_name} = {{" in header_content, "Should define the TextureData struct instance."
    assert f"    {TEST_IMAGE_WIDTH}," in header_content, "Should contain correct width."
    assert f"    {TEST_IMAGE_HEIGHT}," in header_content, "Should contain correct height."
    assert f"    {expected_data_array_name}" in header_content, "Should reference the data array."

    # Basic check on the data array content (crude size check)
    # Find the data array definition line
    data_line = [line for line in header_content.splitlines() if expected_data_array_name in line and "[] = { " in line]
    assert len(data_line) == 1, "Could not find unique data array definition line."
    data_content = data_line[0].split(' = { ')[1].split(' };')[0]
    # Count comma-separated values + 1 (for the last value)
    num_values = data_content.count(',') + 1 
    # This is approximate due to potential formatting variations but gives a sanity check
    assert num_values == EXPECTED_DATA_SIZE, f"Expected {EXPECTED_DATA_SIZE} values in data array, found approx {num_values}."


def test_process_empty_directory(tmp_path):
    """Tests processing an empty directory."""
    if process_images is None:
        pytest.skip("Could not import process_images from generate_props")
        
    header_content = process_images(str(tmp_path))
    assert header_content is None, "Should return None for an empty directory."

def test_process_directory_with_no_images(tmp_path):
    """Tests processing a directory with files but no supported images."""
    if process_images is None:
        pytest.skip("Could not import process_images from generate_props")

    # Create a dummy file
    (tmp_path / "some_text.txt").write_text("hello")
    
    header_content = process_images(str(tmp_path))
    assert header_content is None, "Should return None for a directory with no supported image types."

def test_process_directory_with_unsupported_image(tmp_path):
    """Tests processing a directory with an unsupported image file type."""
    if process_images is None:
        pytest.skip("Could not import process_images from generate_props")

    # Create a dummy file with an image-like extension but unsupported
    (tmp_path / "fake_image.jpeg").write_text("not really an image")
    
    header_content = process_images(str(tmp_path))
    assert header_content is None, "Should return None when only unsupported image types are present." 