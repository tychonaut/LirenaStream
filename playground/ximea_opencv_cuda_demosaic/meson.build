project('ximea_opencv_cuda_demosaic', 'c', 'cpp', version : '0.2.0')

cc = meson.get_compiler('c')


# dependencies -----------------------------------------------------------------

#ximea_m3api_dep  = dependency('m3api')
ximea_m3api_dep  = cc.find_library('m3api', dirs : ['/usr/'])
#ximea_dng_dep = cc.find_library('xiapi_dng_store', dirs : ['/opt/XIMEA/lib/'])



cuda_dep = dependency('cuda', version : '>=10', modules : ['cudart'])
#cudart -L/usr/local/lib -L/usr/local/cuda/lib64 



#set('PKG_CONFIG_PATH','/usr/local/lib/pkgconfig/opencv.pc')
opencv_dep = dependency('opencv', version : '>=3.4', 
                        modules : [
                            'opencv_core',
                            'opencv_highgui',
                            'opencv_cudaimgproc',
                            'opencv_cudaarithm' 
                        ])



gst_app_dep = dependency('gstreamer-app-1.0')
#gst_video_dep = dependency('gstreamer-video-1.0')


all_deps = [
    ximea_m3api_dep,
    cuda_dep,
    opencv_dep,
    gst_app_dep,
]
    ##gst_video_dep,



#gst_app_dep = dependency('gstreamer-app-1.0')
#gst_video_dep = dependency('gstreamer-video-1.0')
#
## https://mesonbuild.com/FAQ.html
#threading_dep = dependency('threads')
#
## gui and show:
#gtk2_dep = dependency('gtk+-2.0')
#X11_dep = dependency('X11')
#
#all_deps = [
#    gst_app_dep, 
#    gst_video_dep,
#    ximea_m3api_dep,
#    ximea_dng_dep,
#    threading_dep,
#    gtk2_dep,
#    X11_dep
#]

#-------------------------------------------------------------------------------

#install_directory = get_option('install_directory')

#hack: also use lirenaConfig:

exe_source_files = [
  'ocvDemosaic.cpp',
  'src/LirenaHackStreamer.cpp',
  'src/LirenaHackXimeaCamera.cpp',
  '../../LirenaCapture/src/LirenaConfig.cpp' 
  ]

executable('ocvDemosaic',
  exe_source_files,
  dependencies : all_deps,
  include_directories: [
    'include',
    '../../LirenaCapture/include',
    '/usr/local/include/', 
    '/usr/local/cuda/include',
  ]
)

