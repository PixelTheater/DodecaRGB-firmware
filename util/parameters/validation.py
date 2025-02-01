"""Parameter validation logic"""
from typing import Type
from .types import (
    Parameter, PARAM_TYPES, PARAM_FLAGS,  # Core definitions
    ParameterBase,
    SemanticParameter, RangeParameter,    # Parameter types
    SelectParameter, SwitchParameter, ResourceParameter
)

def validate_yaml_structure(name: str, param_data: dict) -> None:
    """Validate basic YAML structure and parameter type"""
    if not isinstance(param_data, dict):
        raise ValueError(f"Parameter '{name}' must be a dictionary")

    if 'type' not in param_data:
        raise ValueError(f"Parameter '{name}' missing required 'type' field")

    param_type = param_data['type'].lower()  # Case insensitive type matching
    if param_type not in PARAM_TYPES:
        valid_types = ", ".join(sorted(PARAM_TYPES.keys()))
        raise ValueError(
            f"Unknown parameter type '{param_type}' for parameter '{name}'\n"
            f"Valid types are: {valid_types}"
        )

    # Flag validation
    if 'flags' in param_data:
        flags = param_data['flags']
        if not isinstance(flags, list):
            raise ValueError(f"Parameter '{name}' flags must be a list")
        
        # Case insensitive flag matching
        invalid_flags = set(f.title() for f in flags) - set(PARAM_FLAGS.keys())
        if invalid_flags:
            valid_flags = ", ".join(sorted(PARAM_FLAGS.keys()))
            raise ValueError(
                f"Unknown flags for parameter '{name}': {invalid_flags}\n"
                f"Valid flags are: {valid_flags}"
            )

def create_parameter(name: str, param_data: dict) -> Parameter:
    """Create and validate appropriate Parameter instance from YAML data"""
    validate_yaml_structure(name, param_data)
    
    # Only create flags list if flags are specified
    flags = [flag.title() for flag in param_data.get('flags', [])] if 'flags' in param_data else None
    
    base = ParameterBase(
        name=name,
        description=param_data.get('description', ''),
        flags=flags  # Now None if no flags specified
    )
    
    param_type = param_data['type'].lower()
    
    if param_type in ['ratio', 'signed_ratio', 'angle', 'signed_angle']:
        default = param_data.get('default', PARAM_TYPES[param_type]['range'][0])
        param = SemanticParameter(base=base, param_type=param_type, default=default)
    elif param_type in ['range', 'count']:
        if 'range' not in param_data:
            raise ValueError(f"Parameter '{name}' of type {param_type} requires range")
        range_min, range_max = param_data['range']
        default = param_data.get('default', range_min)
        param = RangeParameter(base=base, param_type=param_type, range_min=range_min, range_max=range_max, default=default)
    elif param_type == 'select':
        if 'values' not in param_data:
            raise ValueError(f"Parameter '{name}' of type select requires 'values' field")
        values = param_data['values']
        if not values:
            raise ValueError(f"Parameter '{name}' values cannot be empty")
        
        # Handle both list and dict forms
        if isinstance(values, list):
            default = param_data.get('default', values[0])
        else:
            default = param_data.get('default', list(values.keys())[0])
        
        param = SelectParameter(base=base, values=values, default=default)
    elif param_type == 'switch':
        default = param_data.get('default', False)
        param = SwitchParameter(base=base, default=default)
    elif param_type in ['palette', 'bitmap']:
        default = param_data.get('default', '')
        param = ResourceParameter(base=base, param_type=param_type, default=default)
    else:
        raise ValueError(f"Unhandled parameter type '{param_type}'")
    
    # Validate the created parameter
    param.validate()
    return param 