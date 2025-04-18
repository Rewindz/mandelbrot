project('Mandelbrot', 'c')

gtkdep = dependency('gtk+-3.0')
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

executable('demo', 'main.c', 'colours.c', c_args : ['-Wall', '-Wextra'], dependencies : [gtkdep, m_dep])

