#ifndef _aitag_lentil_h_
#define _aitag_lentil_h_

enum
{
   C4DAI_LENTIL_MAIN_GRP                              = 2001,
   C4DAI_LENTIL_GENERAL_GRP,
   C4DAI_LENTIL_BOKEH_IMAGE_GRP,
   C4DAI_LENTIL_BIDIRECTIONAL_GRP,
   C4DAI_LENTIL_ADVANCED_GRP,
   C4DAI_LENTIL_ARNOLDNATIVE_GRP,

   C4DAIP_LENTIL_POSITION                             = 1271837232,
   C4DAIP_LENTIL_LOOK_AT                              = 1748112668,
   C4DAIP_LENTIL_UP                                   = 1985597632,
   C4DAIP_LENTIL_MATRIX                               = 1461908720,
   C4DAIP_LENTIL_HANDEDNESS                           = 1180930152,
   C4DAIP_LENTIL_NEAR_CLIP                            = 1275106216,
   C4DAIP_LENTIL_FAR_CLIP                             = 663115077,
   C4DAIP_LENTIL_SCREEN_WINDOW_MIN                    = 1449377099,
   C4DAIP_LENTIL_SCREEN_WINDOW_MAX                    = 1449377353,
   C4DAIP_LENTIL_SHUTTER_START                        = 1823805687,
   C4DAIP_LENTIL_SHUTTER_END                          = 1027087680,
   C4DAIP_LENTIL_SHUTTER_TYPE                         = 465293397,
   C4DAIP_LENTIL_SHUTTER_CURVE                        = 1804885518,
   C4DAIP_LENTIL_ROLLING_SHUTTER                      = 1698475488,
   C4DAIP_LENTIL_ROLLING_SHUTTER_DURATION             = 2035937093,
   C4DAIP_LENTIL_MOTION_START                         = 1263534782,
   C4DAIP_LENTIL_MOTION_END                           = 1326313799,
   C4DAIP_LENTIL_EXPOSURE                             = 751603850,
   C4DAIP_LENTIL_FILTERMAP                            = 1392239457,


   C4DAIP_LENTIL_UNITSPO                              = 1239258157,
   C4DAIP_LENTIL_LENS_MODELPO                         = 1820202076,
   C4DAIP_LENTIL_SENSOR_WIDTHPO                       = 1518683341,
   C4DAIP_LENTIL_WAVELENGTHPO                         = 931803025,
   C4DAIP_LENTIL_DOFPO                                = 184637357,
   C4DAIP_LENTIL_FSTOPPO                              = 744353050,
   C4DAIP_LENTIL_FOCUS_DISTANCEPO                     = 903312068,
   C4DAIP_LENTIL_EXTRA_SENSOR_SHIFTPO                 = 1758254316,
   C4DAIP_LENTIL_BOKEH_APERTURE_BLADESPO              = 1613338764,
   C4DAIP_LENTIL_BOKEH_ENABLE_IMAGEPO                 = 992063797,
   C4DAIP_LENTIL_BOKEH_IMAGE_PATHPO                   = 372694993,
   C4DAIP_LENTIL_BIDIR_MIN_LUMINANCEPO                = 1971143326,
   C4DAIP_LENTIL_BIDIR_SAMPLE_MULTPO                  = 1046204794,
   C4DAIP_LENTIL_BIDIR_ADD_LUMINANCEPO                = 1590552281,
   C4DAIP_LENTIL_BIDIR_ADD_LUMINANCE_TRANSITIONPO     = 199573007,
   C4DAIP_LENTIL_VIGNETTING_RETRIESPO                 = 507665258,
   C4DAIP_LENTIL_NAME                                 = 1947007164,

   C4DAIP_LENTIL_HANDEDNESS__RIGHT                    = 0,
   C4DAIP_LENTIL_HANDEDNESS__LEFT                     = 1,

   C4DAIP_LENTIL_SHUTTER_TYPE__BOX                    = 0,
   C4DAIP_LENTIL_SHUTTER_TYPE__TRIANGLE               = 1,
   C4DAIP_LENTIL_SHUTTER_TYPE__CURVE                  = 2,

   C4DAIP_LENTIL_ROLLING_SHUTTER__OFF                 = 0,
   C4DAIP_LENTIL_ROLLING_SHUTTER__TOP                 = 1,
   C4DAIP_LENTIL_ROLLING_SHUTTER__BOTTOM              = 2,
   C4DAIP_LENTIL_ROLLING_SHUTTER__LEFT                = 3,
   C4DAIP_LENTIL_ROLLING_SHUTTER__RIGHT               = 4,

   C4DAIP_LENTIL_UNITSPO__MM                          = 0,
   C4DAIP_LENTIL_UNITSPO__CM                          = 1,
   C4DAIP_LENTIL_UNITSPO__DM                          = 2,
   C4DAIP_LENTIL_UNITSPO__M                           = 3,

   C4DAIP_LENTIL_LENS_MODELPO__ANGENIEUX__DOUBLE_GAUSS__1953__49MM = 0,
   C4DAIP_LENTIL_LENS_MODELPO__ANGENIEUX__DOUBLE_GAUSS__1953__85MM = 1,
   C4DAIP_LENTIL_LENS_MODELPO__ANGENIEUX__DOUBLE_GAUSS__1953__105MM = 2,
   C4DAIP_LENTIL_LENS_MODELPO__ANGENIEUX__DOUBLE_GAUSS__1953__55MM = 3,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__45MM = 4,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__50MM = 5,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__65MM = 6,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__75MM = 7,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__58MM = 8,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1969__85MM = 9,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1970__28MM = 10,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1970__50MM = 11,
   C4DAIP_LENTIL_LENS_MODELPO__ASAHI__TAKUMAR__1970__35MM = 12,
   C4DAIP_LENTIL_LENS_MODELPO__CANON__RETROFOCUS_WIDEANGLE__1982__22MM = 13,
   C4DAIP_LENTIL_LENS_MODELPO__CANON__UNKNOWN__1956__35MM = 14,
   C4DAIP_LENTIL_LENS_MODELPO__CANON__UNKNOWN__1956__52MM = 15,
   C4DAIP_LENTIL_LENS_MODELPO__COOKE__SPEED_PANCHRO__1920__40MM = 16,
   C4DAIP_LENTIL_LENS_MODELPO__COOKE__SPEED_PANCHRO__1920__75MM = 17,
   C4DAIP_LENTIL_LENS_MODELPO__COOKE__SPEED_PANCHRO__1920__100MM = 18,
   C4DAIP_LENTIL_LENS_MODELPO__COOKE__SPEED_PANCHRO__1920__50MM = 19,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__150MM = 20,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__105MM = 21,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__85MM = 22,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__65MM = 23,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__75MM = 24,
   C4DAIP_LENTIL_LENS_MODELPO__KODAK__PETZVAL__1948__58MM = 25,
   C4DAIP_LENTIL_LENS_MODELPO__MEYER_OPTIK_GOERLITZ__PRIMOPLAN__1936__58MM = 26,
   C4DAIP_LENTIL_LENS_MODELPO__MEYER_OPTIK_GOERLITZ__PRIMOPLAN__1936__75MM = 27,
   C4DAIP_LENTIL_LENS_MODELPO__MINOLTA__FISHEYE__1978__16MM = 28,
   C4DAIP_LENTIL_LENS_MODELPO__MINOLTA__FISHEYE__1978__22MM = 29,
   C4DAIP_LENTIL_LENS_MODELPO__MINOLTA__FISHEYE__1978__28MM = 30,
   C4DAIP_LENTIL_LENS_MODELPO__NIKON__RETROFOCUS_WIDEANGLE__1971__28MM = 31,
   C4DAIP_LENTIL_LENS_MODELPO__NIKON__RETROFOCUS_WIDEANGLE__1971__35MM = 32,
   C4DAIP_LENTIL_LENS_MODELPO__NIKON__UNKNOWN__2014__65MM = 33,
   C4DAIP_LENTIL_LENS_MODELPO__NIKON__UNKNOWN__2014__40MM = 34,
   C4DAIP_LENTIL_LENS_MODELPO__NIKON__UNKNOWN__2014__50MM = 35,
   C4DAIP_LENTIL_LENS_MODELPO__UNKNOWN__PETZVAL__1900__85MM = 36,
   C4DAIP_LENTIL_LENS_MODELPO__UNKNOWN__PETZVAL__1900__100MM = 37,
   C4DAIP_LENTIL_LENS_MODELPO__UNKNOWN__PETZVAL__1900__75MM = 38,
   C4DAIP_LENTIL_LENS_MODELPO__UNKNOWN__PETZVAL__1900__65MM = 39,
   C4DAIP_LENTIL_LENS_MODELPO__ZEISS__BIOTAR__1927__65MM = 40,
   C4DAIP_LENTIL_LENS_MODELPO__ZEISS__BIOTAR__1927__58MM = 41,
   C4DAIP_LENTIL_LENS_MODELPO__ZEISS__BIOTAR__1927__85MM = 42,
   C4DAIP_LENTIL_LENS_MODELPO__ZEISS__BIOTAR__1927__45MM = 43,
};

#endif

