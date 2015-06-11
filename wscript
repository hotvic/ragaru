#! /usr/bin/env python
# encoding: utf-8

VERSION = '0.1a'
APPNAME = 'ragaru'

top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c compiler_cxx')

def configure(cnf):
    cnf.load('compiler_c compiler_cxx')

    cnf.env['INSTALL_DATA_DIR'] = '{0}/share/{1}'.format(cnf.env['PREFIX'], APPNAME)

    cnf.check_cfg(package='glibmm-2.4', uselib_store='GLIBMM', args='--cflags --libs', mandatory=True)
    cnf.check_cfg(package='glu', args='--cflags --libs', mandatory=True)
    cnf.check_cfg(package='ogg', args='--cflags --libs', mandatory=True)
    cnf.check_cfg(package='openal', args='--cflags --libs', mandatory=True)
    cnf.check_cfg(package='sdl2', args='--cflags --libs', madatory=True)
    cnf.check_cfg(package='libpng', args='--cflags --libs', mandatory=True)
    cnf.check_cfg(package='vorbisfile', args='--cflags --libs', mandatory=True)

    cnf.check_cc(type_name='uint32_t', header='stdint.h', mandatory=True)
    cnf.check_cc(type_name='uint64_t', header='stdint.h', mandatory=True)
    cnf.check_cc(type_name='int32_t', header='stdint.h', mandatory=True)
    cnf.check_cc(type_name='int64_t', header='stdint.h', mandatory=True)

    cnf.recurse('src')

def build(bld):
    bld.recurse('src')
