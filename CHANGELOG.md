# Change Log

0.3 - Apr 20
- ported remaining scenes
- introduced SceneKit, simplifying access to API and reducing namespace prefixes
- added sphere radius calculation to model generation, available in scenes
- updated tests to test new api
- Bump scene/project version to 2.1

0.2 - Apr 18
- refactored colors and palletes
- added back neighbor info
- ported more scenes
- cleaned up usage api 
- updated documentation
- web simulator refactoring

0.1 - Mar 4
- retire old versioning, completely new codebase for "PixelTheater" library (v0.1)
- introduce web simulator
- hardware tests and first working scenes
- new model generation and definition approach

(original firmware project "v2" beyond here)

v2.8.3 Feb 13 2025:

- refactored with type handlers and sentinel values, resulting in cleaner code
- improved tests and coverage
- documentation and test fixture updates

v2.8.2 Feb 9 2025:

- refactored settings, settings proxy, parameter values and generation
- documentation and test updates

v2.8.1 Feb 7 2025:

- refactoring of parameters, types, and ranges
- reworked documentation

v2.8.0 Feb 1 2025:

- rebuilt parameter system, added new settings.h and settings.cpp files
- YAML file for animation parameters
- new tests and runner (testunit and doctest both used now)
- settings[] allows inspection of parameters and metadata
- initial support for palettes
- TESTS PASS - but firmware not yet working (still need to migrate animations to new framework)

v2.7.1 Jan 22 2025:

- added new boids (flocking) animation
- updated FastLED to 3.9.12

v2.7.0 Jan 19 2025:

- migrated Processing visualizer and point generator to Python
- more documentation additions and updates
- unit tests for Python utilities
- point distance calculations are now done in Python, and no longer need to be calculated at firmware startup

v2.6.0 Jan 12 2025:

- moved rest of animations to new framework
- cleaned up setup, loop, and status message
- improved power estimation
- added playlist functions to animation manager (next, getCurrentAnimationName, etc)

v2.5 Jan 12 2025:

- added wandering particles animation
- added xyz-scanner animation

v2.4 Jan 11 2025:

- refactoring: as the main file is getting too big, I'm moving the animation code to separate files
- add animation manager to handle animations, and animation params to handle parameters
- refactored palettes and color lookup to be included in the header file
- new logging system for animations
- add sparkles animation
- TODO: migrate more animations to new structure

v2.3 Jan 10 2025:

- update FastLED to 3.9.10
- improved orientation demo, colors and transitions

v2.2 Jan 1 2025:

- added orientation demo, with rotating sphere animations

v2.1 Jan 2025:

- added support for orientation sensor
- improved random seeding based on CPU temp and analog noise
- new animations: ...

v2.0 Dec 8 2024:

- new hardware, micro-leds (1615 package), 104 per side
- overall size is smaller, around 80% compared to v1
- switched to Teensy microcontroller, code refactoring in progress
- working, but only tested with 6 sides active (624) and frame rates of 50fps are achievable so far

## Version 1 of the project

For more info see the [Version 1 README](../Dodeca-V1-info.md).

v1.0 Aug 2023:

- version 1 with only 26 LEDs per side, 312 in total
- initial animations, including blobs, 3d lines, particles, and color show
- released at CCC Camp 2023 <https://hackaday.io/project/192557-dodecargb>
