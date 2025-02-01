"""Parameter C++ code generation"""

import os
from datetime import datetime
from . import Parameter, PARAM_TYPES

def format_parameter_line(param: Parameter) -> str:
    """Generate C++ parameter definition line"""
    name_field = f'"{param.base.name}",'.ljust(20)
    
    # Get C++ enum name for parameter type
    type_info = PARAM_TYPES[param.param_type]
    cpp_type = type_info.get('cpp_name', param.param_type)
    
    # Format flags if present
    flags = "Flags::NONE"
    if param.base.flags:
        flags = " | ".join(f"Flags::{flag}" for flag in param.base.flags)
    
    # Generate line based on parameter type
    if param.param_type == 'select':
        return f'    {{{name_field} ParamType::{cpp_type}, {{.default_idx = 0, .options = {param.base.name}_options}}, {flags}, "{param.base.description}"}}'
    
    elif param.param_type in ['palette', 'bitmap']:
        return f'    {{{name_field} ParamType::{cpp_type}, {{.str_default = "{param.default}"}}, {flags}, "{param.base.description}"}}'
    
    elif param.param_type == 'switch':
        default = 'true' if param.default else 'false'
        return f'    {{{name_field} ParamType::{cpp_type}, {{.bool_default = {default}}}, {flags}, "{param.base.description}"}}'
    
    elif param.param_type == 'count':  # Integer range
        return f'    {{{name_field} ParamType::{cpp_type}, {{.range_min_i = {param.range_min}, .range_max_i = {param.range_max}, .default_val_i = {param.default}}}, {flags}, "{param.base.description}"}}'
    
    elif param.param_type == 'range':  # Float range with explicit min/max
        return f'    {{{name_field} ParamType::{cpp_type}, {{.range_min = {param.range_min}f, .range_max = {param.range_max}f, .default_val = {param.default}f}}, {flags}, "{param.base.description}"}}'
    
    else:  # Semantic types (ratio, angle) use float_default only
        return f'    {{{name_field} ParamType::{cpp_type}, {{.float_default = {param.default}f}}, {flags}, "{param.base.description}"}}'

def generate_options_arrays(parameters: list[Parameter]) -> str:
    """Generate options arrays for select parameters"""
    arrays = []
    for param in parameters:
        if param.param_type == 'select':
            values = [f'"{v}"' for v in param.values]
            values.append('nullptr')  # Null terminator
            array_def = f"static constexpr const char* const {param.base.name}_options[] = {{\n    {', '.join(values)}\n}};"
            arrays.append(array_def)
    return "\n".join(arrays)

def generate_header(scene_name: str, parameters: list[Parameter], description: str = '') -> str:
    """Generate complete C++ header file"""
    array_name = f"{scene_name.upper()}_PARAMS"
    
    # Generate options arrays first
    options = generate_options_arrays(parameters)
    
    # Generate parameter definitions with commas
    param_lines = [format_parameter_line(param) + ',' for param in parameters]
    # Remove comma from last line
    if param_lines:
        param_lines[-1] = param_lines[-1].rstrip(',')
    
    header = f"""// Auto-generated from {scene_name}.yaml
"""
    if description:
        header += f"// {description}\n"
    
    header += f"""// Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {{

{options}

// Parameter definitions - one line per param for easy diffing
constexpr ParamDef {array_name}[] = {{
{os.linesep.join(param_lines)}
}};

}} // namespace PixelTheater
"""
    return header
