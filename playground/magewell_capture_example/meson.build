project('Magewell_capture_example', 'c', 'cpp',
  version: '0.1.1',
  meson_version : '>= 0.57.1',
  default_options : [
    'buildtype=debugoptimized'
  ]
)


cc = meson.get_compiler('c')
#cppc = meson.get_compiler('cpp')


#{ distinguish arm64 and x86_64 -----------------------------------------------
#  TODO maybe extend to cross compilation
architectureString = 'arm64'
if build_machine.cpu_family() == 'aarch64'
    architectureString = 'arm64'
elif build_machine.cpu_family() == 'x86_64'
    architectureString = 'x64'
else
    error('CPU architecture not supported yet' + build_machine.cpu_family())
endif
message('Build machine CPU family : '+ build_machine.cpu_family())
message('Build machine CPU        : '+ build_machine.cpu())
message('Build architecture string: '+ architectureString)
#} end arm vc x86 -------------------------------------------------------------


#{ dependencies -----------------------------------------------------------------
gst_app_dep = dependency('gstreamer-app-1.0')
gst_video_dep = dependency('gstreamer-video-1.0')

##ximea_m3api_dep  = dependency('m3api')
ximea_m3api_dep  = cc.find_library('m3api', dirs : ['/usr/'])
ximea_dng_dep = cc.find_library('xiapi_dng_store', dirs : ['/opt/XIMEA/lib/'])

# https://mesonbuild.com/FAQ.html
threading_dep = dependency('threads')


# gui and show:
gtk2_dep = dependency('gtk+-2.0')
X11_dep = dependency('X11')




#{ magewell capture SDK: --------------

magewell_SDK_root_dir = '../../../deps/MagewellSDK'
message('magewell SDK root dir: '+ magewell_SDK_root_dir)


magewell_SDK_lib_dir = magewell_SDK_root_dir  + '/lib/' + architectureString
message('Megewell SDK lib dir: ' +  magewell_SDK_lib_dir)
magewell_SDK_include_dir = magewell_SDK_root_dir  + '/include/'

magewell_SDK_dep = declare_dependency(
    link_args : [
        '-L' + magewell_SDK_lib_dir, 
        '-lMWCapture'
    ],
    include_directories : [ magewell_SDK_include_dir ]
)

#pthread --> threading_dep, see above
dl_dep = cc.find_library('dl', required : true)
udev_dep = cc.find_library('udev', required : true)
asound_dep = cc.find_library('asound', required : true)
v4l2_dep = cc.find_library('v4l2', required : true)

magewell_deps = [
    magewell_SDK_dep,
    threading_dep,
    dl_dep,
    udev_dep,
    asound_dep,
    v4l2_dep,
]


#{ Hacky stuff in order to not mess with environment variables ($PATH)
#  before launching the app during runtime:
#  Bake the search path for shared libs into the executable;
#  Make it absolute so that the working directory is irrelevant.
#  This s a questionable decision, but I want this app to be as self-contained
#  as possible at the moment: There shall be as little hassle as possible
#  to make it compile and run on a new system after pulling the repo.
magewell_SDK_ABS_root_dir = meson.project_source_root() + \
    '/../../deps/MagewellSDK'
magewell_SDK_ABS_lib_dir =  magewell_SDK_ABS_root_dir  +  '/lib/' + \
    architectureString
message('Magewell SDK absolute lib path ' + \
        '(for runtime linking to shared libs:)\n\t' + \
        magewell_SDK_ABS_lib_dir)
   
my_rpath =  magewell_SDK_ABS_lib_dir
#}     
    
#} end Magewell stuff -----------------



all_deps = [
    gst_app_dep, 
    gst_video_dep,
    ximea_m3api_dep,
    ximea_dng_dep,
    threading_dep,
    gtk2_dep,
    X11_dep,
    magewell_deps
]





#-------------------------------------------------------------------------------

#install_directory = get_option('install_directory')

incdir = include_directories('include')


exe_source_files = [
  'src/CaptureByInput.cpp',
]

executable('captureByInput',
  exe_source_files,
  install : false,
  dependencies : all_deps,
  include_directories: incdir,
  # see explanation above
  build_rpath   : my_rpath,
  install_rpath : my_rpath,
)
