## Setting Scene Metadata

Each scene should have metadata that helps users understand what the scene does. You should set this metadata in your scene's `setup()` method:

```cpp
void setup() override {
    // Set scene metadata
    this->set_name("My Scene Name");
    this->set_description("A description of what this scene does");
    this->set_version("1.0");
    this->set_author("Your Name");
    
    // Define parameters
    // ...
}
```

Setting metadata in the `setup()` method keeps all initialization in one place and follows the same pattern as parameter definition. This makes your code more consistent and easier to understand.

The scene name will be displayed in the UI, and the other metadata can be used for documentation, credits, and version tracking.

You can access the metadata using the following methods:

```cpp
const std::string& name = scene->name();
const std::string& description = scene->description();
const std::string& version = scene->version();
const std::string& author = scene->author();
``` 