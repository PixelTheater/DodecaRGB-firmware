# Test fixtures for integration tests

## Scene Configuration Test



`test_scene.yaml`: is the source for `test_scene_params.h`, which is used to test the scene configuration.

To re-generate:

```bash
python util/scene_generator.py test/fixtures/test_scene.yaml -o test/fixtures/test_scene_params.h
```

`parameter_test_params.h`: is the source for `parameter_test.cpp`, which is used to test the parameter system.

`fireworks_params.h`: is the source for `fireworks.yaml`, which is used to test the python tests for generating C++ fixtures. See `util/README.md` for more details.