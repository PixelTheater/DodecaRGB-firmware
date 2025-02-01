"""
Test error handling for parameter validation

>>> from util.parameters.validation import create_parameter

Test case insensitive parameter types:
>>> param = create_parameter("test", {
...     "type": "SeLeCt",
...     "values": ["a", "b"],
...     "default": "a",
...     "description": "Test param"
... })
>>> param.param_type
'select'

Test both forms of select parameters:
>>> param1 = create_parameter("pattern", {
...     "type": "select",
...     "values": ["sphere", "fountain", "cascade"],
...     "default": "sphere"
... })
>>> isinstance(param1.values, list)
True

>>> param2 = create_parameter("direction", {
...     "type": "select",
...     "values": {"forward": 1, "reverse": -1},
...     "default": "reverse"
... })
>>> isinstance(param2.values, dict)
True

Test error handling for malformed select parameters:
>>> create_parameter("test", {"type": "select", "values": []})
Traceback (most recent call last):
    ...
ValueError: Parameter 'test' values cannot be empty

>>> create_parameter("test", {"type": "select"})
Traceback (most recent call last):
    ...
ValueError: Parameter 'test' of type select requires 'values' field

Test description access through base:
>>> param = create_parameter("test", {
...     "type": "switch",
...     "default": True,
...     "description": "Test description"
... })
>>> param.base.description
'Test description'
>>> hasattr(param, "description")
False
"""

if __name__ == "__main__":
    import doctest
    doctest.testmod() 