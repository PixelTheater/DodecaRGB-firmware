"""Parameter type definitions and constants"""

from dataclasses import dataclass
from typing import List, Optional, Union, Dict
from abc import ABC, abstractmethod
import math

__all__ = [
    'Parameter',
    'SemanticParameter',
    'RangeParameter',
    'SelectParameter',
    'SwitchParameter',
    'ResourceParameter',
    'PARAM_TYPES',
    'PARAM_FLAGS'
]

# At the top with other constants
PARAM_FLAGS = {
    'Clamp': {
        'description': 'Clamp value to range',
        'allowed_types': ['ratio', 'signed_ratio', 'range', 'count']
    },
    'Wrap': {
        'description': 'Wrap value around range',
        'allowed_types': ['ratio', 'signed_ratio', 'range', 'count', 'angle', 'signed_angle']
    },
    'Slew': {
        'description': 'Smooth value changes',
        'allowed_types': ['ratio', 'signed_ratio', 'range', 'angle', 'signed_angle']
    }
}

@dataclass
class ParameterBase:
    """Common parameter fields"""
    name: str
    description: str
    flags: Optional[List[str]] = None

    def validate_flags(self, param_type: Optional[str] = None) -> None:
        """Validate flags are allowed for this parameter type"""
        if not self.flags:
            return
            
        for flag in self.flags:
            if flag not in PARAM_FLAGS:
                raise ValueError(f"Unknown flag '{flag}' for parameter '{self.name}'")
            
            if param_type:  # Check against provided param_type
                flag_info = PARAM_FLAGS[flag]
                if param_type not in flag_info['allowed_types']:
                    raise ValueError(f"Flag '{flag}' not allowed for parameter type '{param_type}'")

@dataclass
class Parameter(ABC):
    """Abstract base class for all parameter types"""
    base: ParameterBase

    def validate(self) -> None:
        """Validate parameter configuration"""
        if hasattr(self, 'param_type'):
            self.base.validate_flags(self.param_type)
        else:
            self.base.validate_flags()

    @abstractmethod
    def generate_code(self) -> str:
        """Generate C++ code for this parameter"""
        pass

@dataclass
class SemanticParameter(Parameter):
    """Parameters with fixed semantic ranges (ratio, angle)"""
    base: ParameterBase
    param_type: str
    default: float

    def validate(self) -> None:
        """Validate semantic parameter configuration"""
        # Call base class validation first to handle flags
        super().validate()
        
        type_info = PARAM_TYPES[self.param_type]
        range_min, range_max = type_info['range']
        
        if not isinstance(self.default, (int, float)):
            raise ValueError(f"Parameter '{self.base.name}' default must be numeric")
        
        default = float(self.default)  # Ensure float comparison
        if default < float(range_min) or default > float(range_max):
            raise ValueError(
                f"Parameter '{self.base.name}' default must be between "
                f"{range_min} and {range_max}"
            )

    def generate_code(self) -> str:
        name_field = f'"{self.base.name}",'.ljust(20)
        flags_str = " | ".join(f"ParamFlag::{f}" for f in (self.base.flags or []))
        return f'    {{{name_field} ParamType::{self.param_type}, {self.default}f, {flags_str}, "{self.base.description}"}}'

@dataclass
class RangeParameter(Parameter):
    """Parameters with custom ranges (count, range)"""
    param_type: str
    range_min: float
    range_max: float
    default: float

    def validate(self) -> None:
        """Validate range parameter configuration"""
        if not isinstance(self.range_min, (int, float)) or not isinstance(self.range_max, (int, float)):
            raise ValueError(f"Parameter '{self.base.name}' range values must be numeric")
        if self.range_min >= self.range_max:
            raise ValueError(f"Parameter '{self.base.name}' range minimum must be less than maximum")
        if self.default < self.range_min or self.default > self.range_max:
            raise ValueError(f"Parameter '{self.base.name}' default must be within range")

    def generate_code(self) -> str:
        name_field = f'"{self.base.name}",'.ljust(20)
        flags_str = " | ".join(f"ParamFlag::{f}" for f in (self.base.flags or []))
        flag_part = f", {flags_str}" if flags_str else ""
        return f'    {{{name_field} ParamType::{self.param_type}, {self.range_min}, {self.range_max}, {self.default}{flag_part}, "{self.base.description}"}}'

@dataclass
class SelectParameter(Parameter):
    """Parameters with enumerated values"""
    base: ParameterBase
    values: List[str]
    default: str
    param_type: str = 'select'

    def validate(self) -> None:
        """Validate select parameter configuration"""
        # Call base class validation first to handle flags
        super().validate()
        if not self.values:
            raise ValueError(f"Parameter '{self.base.name}' values cannot be empty")
        if self.default not in self.values:
            raise ValueError(f"Parameter '{self.base.name}' default must be one of {self.values}")

    def generate_code(self) -> str:
        name_field = f'"{self.base.name}",'.ljust(20)
        flags_str = " | ".join(f"ParamFlag::{f}" for f in (self.base.flags or []))
        quoted_values = [f'"{v}"' for v in self.values]
        values_array = f'{{{", ".join(quoted_values)}}}'
        return f'    {{{name_field} ParamType::{self.param_type}, 0, {flags_str}, "{self.default}", "{self.base.description}", {values_array}}},'

@dataclass
class SwitchParameter(Parameter):
    """Boolean switch parameter"""
    base: ParameterBase
    default: bool
    param_type: str = 'switch'

    def validate(self) -> None:
        if not isinstance(self.default, bool):
            raise ValueError(f"Parameter '{self.base.name}' default must be boolean")

    def generate_code(self) -> str:
        name_field = f'"{self.base.name}",'.ljust(20)
        flags_str = " | ".join(f"ParamFlag::{f}" for f in (self.base.flags or []))
        default = 'true' if self.default else 'false'
        return f'    {{{name_field} ParamType::{self.param_type}, {default}, {flags_str}, "{self.base.description}"}}'

@dataclass
class ResourceParameter(Parameter):
    """Resource parameters (palette, bitmap)"""
    param_type: str
    default: str

    def validate(self) -> None:
        if self.param_type not in ['palette', 'bitmap']:
            raise ValueError(f"Resource parameter type must be 'palette' or 'bitmap', got '{self.param_type}'")
        if not isinstance(self.default, str):
            raise ValueError(f"Parameter '{self.base.name}' default must be string")

    def generate_code(self) -> str:
        name_field = f'"{self.base.name}",'.ljust(20)
        flags_str = " | ".join(f"ParamFlag::{f}" for f in (self.base.flags or []))
        return f'    {{{name_field} ParamType::{self.param_type}, "{self.default}", {flags_str}, "{self.base.description}"}}'

# Define semantic type mappings
PARAM_TYPES = {
    # Semantic Types (Fixed Ranges)
    'ratio': {
        'cpp_type': 'float',
        'range': (0.0, 1.0),
        'needs_range': False
    },
    'signed_ratio': {
        'cpp_type': 'float',
        'range': (-1.0, 1.0),
        'needs_range': False
    },
    'angle': {
        'cpp_type': 'float',
        'range': (0.0, math.pi),
        'needs_range': False
    },
    'signed_angle': {
        'cpp_type': 'float',
        'range': (-math.pi, math.pi),
        'needs_range': False
    },
    
    # Basic Types (Custom Ranges)
    'range': {
        'cpp_type': 'float',
        'needs_range': True
    },
    'count': {
        'cpp_type': 'int',
        'needs_range': True
    },
    
    # Choice Types
    'select': {
        'cpp_type': 'int',
        'needs_values': True
    },
    'switch': {
        'cpp_type': 'bool'
    },
    
    # Resource Types
    'palette': {
        'cpp_type': 'Palette'
    },
    'bitmap': {
        'cpp_type': 'Bitmap'
    }
} 