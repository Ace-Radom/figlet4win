import os

PATH = os.environ["PATH"].split( os.pathsep )
gpp_paths = []
for path in PATH:
    this_gcc_path = os.path.join( path , "gcc.exe" )
    this_gpp_path = os.path.join( path , "g++.exe" )
    this_libstdcpp_path = os.path.join( path , "libwinpthread-1.dll" )
    if os.path.exists( this_gcc_path ) and os.path.exists( this_gpp_path ):
        if os.path.exists( this_libstdcpp_path ):
            print( this_libstdcpp_path.replace( "\\" , "/" ) , end = "" )
            exit( 0 )

exit( 1 )
