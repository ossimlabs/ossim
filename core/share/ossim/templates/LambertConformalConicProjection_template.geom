//---------------------------------------------------------------------
// $Id: LambertConformalConicProjection_template.geom 9465 2006-08-28 18:53:59Z dburken $
//
// Description:
// This is a keyword list with minimum set of key words to make a
// Lambert Conformal Conic Projection
// 
// If your image is foo.tif and you renamed this file to foo.geom
// ossim should make a LCC projection when you open the image.
//
//---------------------------------------------------------------------
central_meridian:  -81.0
datum:  WGE
ellipse_code:  WE
ellipse_name:  WGS 84
false_easting_northing:  ( 200000.0, 0.000000000000000 )
false_easting_northing_units:  meters
major_axis:  6378137.000000000000000
minor_axis:  6356752.314199999906123
origin_latitude:  28.0
pixel_scale_units:  meters
pixel_scale_xy:  ( 1.0, 1.0 )
std_parallel_1:  24.000000000000000
std_parallel_2:  32.000000000000000

// point(tie center of pixel) or area (tie upper left corner of pixel).
pixel_type:  point

tie_point_units:  meters
tie_point_xy:  ( 6156.0, 114892.0 )
type:  ossimLambertConformalConicProjection
