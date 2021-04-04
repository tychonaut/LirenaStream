project('LirenaCapture', 'c', 'cpp',
  version: '0.2.1',
  meson_version : '>= 0.57.999',
  default_options : [
    'buildtype=debugoptimized'
  ]
)


#'warning_level=1',
#'c_std=gnu99'
#gstreamer_api_version = '1.0'

cc = meson.get_compiler('c')
#cppc = meson.get_compiler('cpp')

# dependencies -----------------------------------------------------------------
gst_app_dep = dependency('gstreamer-app-1.0')
gst_video_dep = dependency('gstreamer-video-1.0')

#ximea_m3api_dep  = dependency('m3api')
ximea_m3api_dep  = cc.find_library('m3api', dirs : ['/usr/'])
ximea_dng_dep = cc.find_library('xiapi_dng_store', dirs : ['/opt/XIMEA/lib/'])

# https://mesonbuild.com/FAQ.html
threading_dep = dependency('threads')
# pthread_dep = dependency('pthread')

# gui and show:
gtk2_dep = dependency('gtk+-2.0')
X11_dep = dependency('X11')

all_deps = [
    gst_app_dep, 
    gst_video_dep,
    ximea_m3api_dep,
    ximea_dng_dep,
    threading_dep,
    gtk2_dep,
    X11_dep
]
#-------------------------------------------------------------------------------

install_directory = get_option('install_directory')

incdir = include_directories('include')


exe_source_files = [
  'src/LirenaCaptureApp.cpp',
  'src/LirenaConfig.cpp',
  'src/LirenaKLVappsrc.cpp',
  'src/LirenaCaptureDisplay.cpp']

executable('lirenaCapture',
  exe_source_files,
  install : true,
  dependencies : all_deps,
  include_directories: incdir
)
