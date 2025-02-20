# Test fixtures for integration tests

## Scene Configuration Test

`test_scene.yaml`: is the source for `test_scene_params.h`, which is used to test the scene configuration.

To re-generate:

```bash
python util/scene_generator.py test/fixtures/test_scene.yaml -o test/fixtures/test_scene_params.h
```

## Parameter Test Fixtures

- `parameter_test_params.h`: generated source from `parameter_test.yaml` which is used to test the parameter system.

- `params/fireworks_params.h`: generated source from `params/fireworks.yaml` a configuration example that is also used in python tests for generating C++ fixtures.

See `util/README.md` for more details.


## Model Fixtures

- `models/basic_pentagon_model.h`: a basic model with a pentagon face.
- `models/model_relationships.h`: validate the relationships between parts of the model.


