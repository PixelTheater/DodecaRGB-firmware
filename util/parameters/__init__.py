"""
Parameter handling for DodecaRGB animations.
Provides type definitions, validation, and code generation for animation parameters.
"""

from .types import (
    Parameter,
    ParameterBase,
    SemanticParameter,
    RangeParameter,
    SelectParameter,
    SwitchParameter,
    ResourceParameter,
    PARAM_TYPES,
    PARAM_FLAGS
)
from .validation import create_parameter
from .generator import generate_header

__all__ = [
    'Parameter',
    'ParameterBase',
    'SemanticParameter',
    'RangeParameter',
    'SelectParameter',
    'SwitchParameter',
    'ResourceParameter',
    'PARAM_TYPES',
    'PARAM_FLAGS',
    'create_parameter',
    'generate_header'
] 