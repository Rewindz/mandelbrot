project('Mandelbrot', 'c')

gtkdep = dependency('gtk+-3.0')
epoxydep = dependency('epoxy')
cairodep = dependency('cairo')
glib_dep = dependency('glib-2.0')


cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

executable('demo', 'main.c', 'colours.c', c_args : ['-Wall', '-Wextra'], dependencies : [gtkdep, epoxydep, m_dep, cairodep, glib_dep])
