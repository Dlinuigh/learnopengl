deps = [
  dependency('glfw3'),
]
inc = [
  '-I../include',
]
executable('window', 'window.cpp', dependencies: deps, cpp_args: inc)
executable('triangle', 'triangle.cpp', dependencies: deps, cpp_args: inc)
executable('shader', 'shader.cpp', dependencies: deps, cpp_args: inc)
executable('texture', 'texture.cpp', dependencies: deps, cpp_args: inc)
executable('transformation', 'transformation.cpp', dependencies: deps, cpp_args: inc)
executable('coordinate', 'coordinate.cpp', dependencies: deps, cpp_args: inc)
executable(
  'coordinate_one_box',
  'coordinate_3d_one_box.cpp',
  dependencies: deps,
  cpp_args: inc,
)
executable(
  'coordinate_multi_box',
  'coordinate_3d_multi_boxes.cpp',
  dependencies: deps,
  cpp_args: inc,
)
executable('camera', 'camera.cpp', dependencies: deps, cpp_args: inc)