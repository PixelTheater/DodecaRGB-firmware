"""Parameter C++ code generation"""

import os
from datetime import datetime
from . import Parameter, PARAM_TYPES

def format_parameter_line(param: Parameter) -> str:
    """Generate C++ parameter definition line"""
    name_field = f'"{param.base.name}",'.ljust(20)
    
    # Format flags if present
    flags = []
    if param.base.flags:
        flags = [f"ParamFlag::{flag}" for flag in param.base.flags]
    flags_str = " | ".join(flags) if flags else ""
    
    # Generate line based on parameter type
    if param.param_type == 'select':
        quoted_values = [f'"{v}"' for v in param.values]
        values_array = f'{{{", ".join(quoted_values)}}}'
        return f'    {{{name_field} ParamType::{param.param_type}, 0, {flags_str}, "{param.base.description}", {values_array}}},'
    
    elif param.param_type in ['palette', 'bitmap']:
        return f'    {{{name_field} ParamType::{param.param_type}, "{param.default}", {flags_str}, "{param.base.description}"}}'
    
    elif param.param_type == 'switch':
        default = 'true' if param.default else 'false'
        return f'    {{{name_field} ParamType::{param.param_type}, {default}, {flags_str}, "{param.base.description}"}}'
    
    elif PARAM_TYPES[param.param_type].get('needs_range', False):
        flag_part = f", {flags_str}" if flags_str else ""
        return f'    {{{name_field} ParamType::{param.param_type}, {param.range_min}, {param.range_max}, {param.default}{flag_part}, "{param.base.description}"}}'
    
    else:  # Semantic types
        return f'    {{{name_field} ParamType::{param.param_type}, {param.default}f, {flags_str}, "{param.base.description}"}}'

def generate_header(scene_name: str, parameters: list[Parameter], description: str = '') -> str:
    """Generate complete C++ header file"""
    array_name = f"{scene_name.upper()}_PARAMS"
    param_lines = [format_parameter_line(param) for param in parameters]
    
    header = f"""// Auto-generated from {scene_name}.yaml
"""
    if description:
        header += f"// {description}\n"
    
    header += f"""// Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
#pragma once
#include "PixelTheater/parameter.h"
#include "PixelTheater/param_types.h"

namespace PixelTheater {{

// Parameter definitions - one line per param for easy diffing
constexpr ParamDef {array_name}[] = {{
{os.linesep.join(param_lines)}
}};

}} // namespace PixelTheater
"""
    return header
